#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <x68k/dos.h>

int main(int argc, char **argv)
{
    int sstop;
    int ssbtm;
    int ssp;

    ssp = _dos_super(0);
    sstop = 0x3800;
    ssbtm = *(int *)0x1c04;

    if (argc >= 2) {
        memset((void *)sstop, 0xaa, ssp - sstop);
        printf("スーパバイザスタック消費量計測を開始します\n");
    }

    printf("スーパバイザスタック:\t0x%08x - 0x%08x (%dbytes)\n", sstop, ssbtm, ssbtm - sstop);
    printf("現在のスタックポインタ:\t0x%08x (used %dbytes, free %dbytes)\n", ssp, ssbtm - ssp, ssp - sstop);

    for (int addr = sstop; addr < ssbtm; addr += 2) {
        if (*(uint16_t *)addr != 0xaaaa) {
            printf("最大スタック消費量:\t0x%08x (used %dbytes, free %dbytes)\n", addr, ssbtm - addr, addr - sstop);
            break;
        }
    }

    _dos_super(ssp);

    return 0;
}
