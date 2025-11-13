#include <stdio.h>
#include <stdint.h>
#include <x68k/dos.h>
#include <x68k/iocs.h>

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

struct dos_curdir {
    uint8_t     drive;      // 物理ドライブ番号 (A:=0, B:=1, ...)
    uint8_t     coron;      // コロン文字 (':')
    uint8_t     path[62];   // カレントディレクトリのパス (デリミタは'\t')
    uint32_t    reserved1;
    uint8_t     reserved2;
    uint8_t     type;       // ドライブ種別
    struct dos_dpb *dpb;
    uint16_t    fatno;
    uint16_t    pathlen;
} __attribute__((packed,aligned(2)));

static void name83_print(char *name1, char *ext)
{
    for (int i = 0; i < 8; i++) {
        putchar(name1[i] == ' ' ? '_' : name1[i]);
    }
    putchar('.');
    for (int i = 0; i < 3; i++) {
        putchar((ext[i] == 0 || ext[i] == ' ') ? '_' : ext[i]);
    }
}

int main(int argc, char **argv)
{
    _dos_super(0);

    char *drvxtbl;

    printf("$1c15.b: カレントドライブ番号 = %u (%c)\n", *(uint8_t *)0x1c15, *(uint8_t *)0x1c15 + 'A');
    printf("$1c73.b: LASTDRIVE = %u (%c)\n", *(uint8_t *)0x1c73, *(uint8_t *)0x1c73 + 'A');
    printf("$1c74.b: 最大ドライブ番号(固定) = %u\n", *(uint8_t *)0x1c74);

    printf("$1c7e- : ドライブ交換テーブル : ");
    drvxtbl = (char *)0x1c7e;
    for (int i = 0; i < 26; i++) {
        printf("%c ", 'A' + drvxtbl[i]);
    }
    printf("\n");

    printf("$1c3c.l: DPBテーブル = 0x%06lx\n", *(uint32_t *)0x1c3c);
    printf("$1c38.l: カレントディレクトリテーブル = 0x%06lx\n", *(uint32_t *)0x1c38);
    struct dos_curdir *curdir_table = (struct dos_curdir *)*(uint32_t *)0x1c38;
    for (int i = 0; i < 26; i++) {
        int drv = drvxtbl[i];
        struct dos_curdir *curdir = &curdir_table[drv];
        struct dos_dpb *dpb = curdir->dpb;
        char *dir = curdir->path;

        if (curdir->type != 0) {
            printf(" %c:->%c:", 'A' + i, 'A' + drv);
            printf(" %02x", curdir->type);
            printf(" DPB:0x%06lx", (int)curdir->dpb);
            printf("(->0x%06lx) 0x%06lx(", (int)dpb->next & 0xffffff, (int)dpb->devheader);
            for (int i = 0; i < 7; i++) {
                putchar(dpb->devheader->name[i + 1]);
            }
            printf(") #%d %4d :", dpb->unit, dpb->secbyte);
            while (*dir != '\0') {
                putchar(*dir == '\t' ? '\\' : *dir);
                dir++;
            }
            putchar('\n');
        }
    }

    printf("$1c6e.w: 最大ファイルハンドル数 = %u\n", *(uint16_t *)0x1c6e);
    for (int i = 0; i <= *(uint16_t *)0x1c6e; i++) {
        union dos_fcb *fcb = _dos_get_fcb_adr(i);
        if ((int)fcb < 0) {
            continue;
        }
        uint8_t devattr = fcb->chr.devattr;
        printf(" fd=%02d FCB:%p count=%u attr=0x%02x", i, fcb, fcb->chr.dupcnt, devattr);
        if (devattr & 0x80) {
            printf("(C) %p ", fcb->chr.deventry);
            name83_print(fcb->chr.name1, fcb->chr.ext);
        } else {
            printf("(B) %p ", fcb->blk.deventry);
            name83_print(fcb->blk.name1, fcb->blk.ext);
        }
        printf("\n");
    }
    return 0;
}
