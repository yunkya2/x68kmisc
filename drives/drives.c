#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <x68k/dos_inline.h>

struct devheader {
	struct devheader *next;
	uint16_t attr;
	void *strategy;
	void *interrupt;
	char name[8];
};

struct devlist {
	int used;
	int id;
	int unit;
	uint16_t attr;
	char name[8];
} devlist[26];

struct devid {
	void *driver;
	int id;
	char name[8];
} devid[27];

void getdevid(void)
{
	struct devid *l = devid;
	char name[8] = "";
	int id = 0;
	char *dev = *(char **)0x001c20;
	while (memcmp(dev, "NUL     ", 8) != 0) {
		dev += 2;
	}
	struct devheader *h = (struct devheader *)(dev - 14);
	while (1) {
		if (!(h->attr & 0x8000)) {
			l->driver = h;
			l->name[7] = '\0';
			memcpy(l->name, &h->name[1], 7);
			if (memcmp(name, l->name, 8) == 0) {
				id++;
			} else {
				memcpy(name, l->name, 8);
				id = 0;
			}
			l->id = id;
			l++;
		}
		if (h->next == (struct devheader *)-1) {
			break;
		}
		h = h->next;
	}
	l->driver = NULL;
}

void scan(void)
{
	struct dos_dpbptr d;
	struct devlist *l;
	for (int i = 0; i < 26; i++) {
		l = &devlist[i];
		int r = _dos_getdpb(i + 1, &d);
		if (r == -15) {
			l->used = false;
			continue;
		}
		l->used = true;
		struct devheader *h = (struct devheader *)d.driver;
		memcpy(l->name, &h->name[1], 7);
		l->name[7] = '\0';
		l->unit = d.unit;
		l->attr = h->attr;

		l->id = 0;
		for (struct devid *i = devid; i->driver != NULL; i++) {
			if (i->driver == (void *)d.driver) {
				l->id = i->id;
				break;
			}
		}
	}
}


int main(int argc, char **argv)
{
	_dos_super(0);
	getdevid();

	scan();
	if (argc == 1) {
		for (int i = 0; i < 26; i++) {
			struct devlist *l = &devlist[i];
			if (l->used) {
				printf("%c: %s %d %d 0x%04x\n", 'A' + i, l->name, l->id, l->unit, l->attr);
			}
		}
	}

	int drive = 0;
	int arg = 1;
	int verbose = true;

	if (argc > 1 && strcmp(argv[1], "-q") == 0) {
		arg++;
		verbose = false;
	}

	for (int i = arg; i < argc; i++) {
		const char *count = strchr(argv[i], ':');
		size_t len = count ? (size_t)(count - argv[i]) : strlen(argv[i]);
		int remaining = 26;
		if (len == 0 || len > 7) {
			fprintf(stderr, "invalid driver name: %s\n", argv[i]);
			return 1;
		}
		if (count) {
			count++;
			if (*count == '\0') {
				fprintf(stderr, "invalid unit count: %s\n", argv[i]);
				return 1;
			}
			remaining = 0;
			for (; *count; count++) {
				if (*count < '0' || *count > '9') {
					fprintf(stderr, "invalid unit count: %s\n", argv[i]);
					return 1;
				}
				if (remaining < 26) {
					remaining = remaining * 10 + (*count - '0');
					if (remaining > 26)
						remaining = 26;
				}
			}
		}
		char name[8];
		memset(name, ' ', 7);
		memcpy(name, argv[i], len);
		name[7] = '\0';
		while (remaining > 0 && drive < 26) {
			int best = -1;
			for (int j = drive; j < 26; j++) {
				struct devlist *l = &devlist[j];
				if (l->used && strcasecmp(name, l->name) == 0 &&
				    (best < 0 || l->id < devlist[best].id ||
				     (l->id == devlist[best].id && l->unit < devlist[best].unit)))
					best = j;
			}
			if (best < 0)
				break;
			struct devlist *l = &devlist[best];
			if (verbose)
				printf("%c: <-> %c: %s %d %d 0x%04x\n", 'A' + drive, 'A' + best, l->name, l->id, l->unit, l->attr);
			if (drive != best) {
				struct devlist tmp = devlist[drive];
				devlist[drive] = devlist[best];
				devlist[best] = tmp;
				_dos_drvxchg(drive + 1, best + 1);
				if (_dos_curdrv() == drive) {
					_dos_chgdrv(best);
					if (verbose)
						printf("chdrv %c: -> %c:\n", 'A' + drive, 'A' + best);
				} else if (_dos_curdrv() == best) {
					_dos_chgdrv(drive);
					if (verbose)
						printf("chdrv %c: -> %c:\n", 'A' + best, 'A' + drive);
			}
			}
			drive++;
			remaining--;
		}
	}

	return 0;
}
