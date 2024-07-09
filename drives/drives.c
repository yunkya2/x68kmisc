#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <x68k/dos.h>

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
	char *dev = (char *)0x006800;
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
		char name[8];
		memset(name, 0, 8);
		strncpy(name, argv[i], 7);
		for (int i = strlen(name); i < 7; i++)
			name[i] = ' ';
		int unit = 0;
		int id = 0;
		while (1) {
			int namefound = false;
			int idfound = false;
			int unitfound = false;
			int j;
			for (j = 0; j < 26; j++) {
				struct devlist *l = &devlist[j];
				if (memcmp(name, l->name, 7) == 0) {
					namefound = true;
					if (l->id == id) {
						idfound = true;
						if (l->unit == unit) {
							unitfound = true;
							if (verbose)
								printf("%c: <-> %c: %s %d %d 0x%04x\n", 'A' + drive, 'A' + j, l->name, l->id, l->unit, l->attr);
							if (drive != j) {
								struct devlist tmp;
								tmp = devlist[drive];
								devlist[drive] = devlist[j];
								devlist[j] = tmp;
								_dos_drvxchg(drive + 1, j + 1);
								if (_dos_curdrv() == drive) {
									_dos_chgdrv(j);
									if (verbose)
										printf("chdrv %c: -> %c:\n:", 'A' + drive, 'A' + j);
								} else if (_dos_curdrv() == j) {
									_dos_chgdrv(drive);
									if (verbose)
										printf("chdrv %c: -> %c:\n", 'A' + j, 'A' + drive);
								}
							}
							drive++;
							unit++;
							break;
						}
					}
				}
			}
			if (!namefound)
				break;
			if (!idfound)
				break;
			if (!unitfound) {
				id++;
				unit = 0;
			}
		}
	}

	return 0;
}
