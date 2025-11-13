#include <stdio.h>
#include <stdint.h>
#include <x68k/dos.h>

struct dos_iobuf {
	struct dos_iobuf *next;
	uint32_t drvsect;
	struct dos_iobuf *prev;
	uint32_t flagdpb;
};

struct dos_devheader {
    struct dos_devheader *next;
    uint16_t    attr;
    void        *strategy;
    void        *interrupt;
    char        name[8];
};

struct dos_dpb {
    int8_t      drive;
    int8_t      unit;
    struct dos_devheader *devheader;
    struct dos_dpb *next;
    uint16_t    secbyte;
} __attribute__((packed,aligned(2)));

void print_iobuf(struct dos_iobuf *p, struct dos_iobuf *iobuf, int iobufsize, int iobufnum)
{
	char *drvxtbl = (char *)0x1c7e;	// ドライブ交換テーブル

	int num = (int)((uint32_t)p - (uint32_t)iobuf) / (iobufsize + sizeof(struct dos_iobuf));
	int nextnum = ((int)p->next == 0xffffffff) ? -1 : (int)((uint32_t)p->next - (uint32_t)iobuf) / (iobufsize + sizeof(struct dos_iobuf));
	int prevnum = ((int)p->prev == 0xffffffff) ? -1 : (int)((uint32_t)p->prev - (uint32_t)iobuf) / (iobufsize + sizeof(struct dos_iobuf));
//	printf(" [%02d] %p: next=%p(%02d) prev=%p(%02d) ", num, p, p->next, nextnum, p->prev, prevnum);
	printf(" [%02d] %p: next=%02d prev=%02d ", num, p, nextnum, prevnum);

	int drive = p->drvsect >> 24;
	int sector = p->drvsect & 0x00ffffff;
	int flag = p->flagdpb >> 24;
	struct dos_dpb *dpb = (struct dos_dpb *)(p->flagdpb & 0x00ffffff);

	if (flag & 0x40) {
		if (flag & 0x80) printf("FAT"); else printf("fat");
	} else if (flag & 0x20) {
		if (flag & 0x80) printf("DIR"); else printf("dir");
	} else if (flag & 0x10) {
		if (flag & 0x80) printf("FIL"); else printf("fil");
	} else {
		printf("---");
	}

	if (flag) {
		char vdrv = '?';
		for (int i = 0; i < 26; i++) {
			if (drvxtbl[i] == drive) {
				vdrv = 'A' + i;
				break;
			}
		}	

		printf(" %c:0x%06x dpb=0x%06x unit=%d ", vdrv, sector, (int)dpb, dpb->unit);
		for (int i = 1; i < 8; i++) {
			putchar(dpb->devheader->name[i]);
		}
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	_dos_super(0);

	struct dos_iobuf *iobuf = *(struct dos_iobuf **)0x1c34;
	int iobufnum = *(uint8_t *)0x1c72;
	int iobufsize = *(uint16_t *)0x1c70;

	printf("$1c34.l: I/Oバッファ先頭アドレス = %p\n", iobuf);
	printf("$1c72.b: バッファ数 = %d\n", iobufnum);
	printf("$1c70.w :バッファサイズ = %d\n", iobufsize);

	struct dos_iobuf *p;

	if (argc > 1) {
		printf("\nI/Oバッファリスト(番号順)\n");
		p = iobuf;
		for (int i = 0; i < iobufnum; i++) {
			print_iobuf(p, iobuf, iobufsize, iobufnum);
			p = (struct dos_iobuf *)((uint32_t)&p[1] + iobufsize);
		}
	} else {
		// LRU順リストの先頭から表示するために、リンクを先頭までさかのぼる
		struct dos_iobuf *iobuflru = iobuf;
		while (iobuflru->prev != (struct dos_iobuf *)0xffffffff) {
			iobuflru = iobuflru->prev;
		}
		printf("$b6b6.l: I/OバッファLRU先頭アドレス = %p\n", iobuflru);

		printf("\nI/Oバッファリスト(LRU順)\n");
		p = iobuflru;
		while ((int)p != 0xffffffff) {
			print_iobuf(p, iobuf, iobufsize, iobufnum);
			p = p->next;
		}
	}

	return 0;
}
