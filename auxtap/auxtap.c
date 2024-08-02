/*
 * Copyright (c) 2024 Yuichi Nakamura (@yunkya2)
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <x68k/iocs.h>
#include <x68k/dos.h>

extern uint32_t oldvect;
extern void auxintr_asm();

extern uint16_t escptr;
extern uint8_t escseq[8];

__asm__(
"start: .long _start\n"
"oldvect: .long 0\n"
".ascii \"AXT\\0\"\n"

"auxintr_asm:\n"
"movem.l %d0-%d7/%a0-%a6,%sp@-\n"
"move.w %sr,%sp@-\n"
"ori.w #0x0700,%sr\n"
"bsr auxintr\n"
"move.w %sp@+,%sr\n"
"movem.l %sp@+,%d0-%d7/%a0-%a6\n"
"rte\n"

"escptr: .space 2\n"
"escseq: .space 8\n"
".even\n"
);

__attribute__ ((section(".text")))
const uint16_t keytbl[128] = {
    0x0000, 0x711e, 0x712e, 0x712c, 0x7120, 0x7113, 0x7121, 0x7122,     // 0x00
    0x000f, 0x0010, 0x7124, 0x7125, 0x7126, 0x001d, 0x712f, 0x7119,
    0x711a, 0x7111, 0x7114, 0x711f, 0x7115, 0x7117, 0x712d, 0x7112,     // 0x10
    0x712b, 0x7116, 0x712a, 0x0001, 0x710e, 0x7129, 0x710d, 0x7134,

    0x0035, 0x7002, 0x7003, 0x7004, 0x7005, 0x7006, 0x7007, 0x7008,     // 0x20
    0x7009, 0x700a, 0x7028, 0x7027, 0x0031, 0x000c, 0x0032, 0x0033,
    0x000b, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008,     // 0x30
    0x0009, 0x000a, 0x0028, 0x0027, 0x7031, 0x700c, 0x7032, 0x7033,

    0x001b, 0x701e, 0x702e, 0x702c, 0x7020, 0x7013, 0x7021, 0x7022,     // 0x40
    0x7023, 0x7018, 0x7024, 0x7025, 0x7026, 0x7030, 0x702f, 0x7019,
    0x701a, 0x7011, 0x7014, 0x701f, 0x7015, 0x7017, 0x702d, 0x7012,     // 0x50
    0x702b, 0x7016, 0x702a, 0x001c, 0x000e, 0x0029, 0x000d, 0x7034,

    0x701b, 0x001e, 0x002e, 0x002c, 0x0020, 0x0013, 0x0021, 0x0022,     // 0x60
    0x0023, 0x0018, 0x0024, 0x0025, 0x0026, 0x0030, 0x002f, 0x0019,
    0x001a, 0x0011, 0x0014, 0x001f, 0x0015, 0x0017, 0x002d, 0x0012,     // 0x70
    0x002b, 0x0016, 0x002a, 0x701c, 0x700e, 0x7029, 0x700d, 0x0037,
};

__attribute__ ((section(".text")))
const struct { uint16_t keycode; const char escseq[6]; } esctbl[] = {
    { 0x003b, "\x1b[D" },   // ←
    { 0x003c, "\x1b[A" },   // ↑
    { 0x003d, "\x1b[C" },   // →
    { 0x003e, "\x1b[B" },   // ↓

    { 0x0036, "\x1b[H" },   // HOME
    { 0x0037, "\x1b[3~" },  // DEL
    { 0x0038, "\x1b[6~" },  // R_UP   (PAGE DOWNN)
    { 0x0039, "\x1b[5~" },  // R_DOWN (PAGE UP)
    { 0x003f, "\x1b[F" },   // CLR    (END)

    { 0x0063, "\x1b[11~" }, // F1
    { 0x0064, "\x1b[12~" }, // F2
    { 0x0065, "\x1b[13~" }, // F3
    { 0x0066, "\x1b[14~" }, // F4
    { 0x0067, "\x1b[15~" }, // F5

    { 0x0068, "\x1b[17~" }, // F6
    { 0x0069, "\x1b[18~" }, // F7
    { 0x006a, "\x1b[19~" }, // F8
    { 0x006b, "\x1b[20~" }, // F9
    { 0x006c, "\x1b[21~" }, // F10

    { 0x005e, "\x1b[2~" },  // INS
};

static void skeyset(uint16_t scancode)
{
    __asm__ volatile (
        "move.l %0,%%d1\n"
        "moveq.l #0x05,%%d0\n"
        "trap #15\n"
        : : "d"(scancode) : "%%d0", "%%d1"
    );
}

static void sendkey(uint16_t keycode)
{
    if (!keycode)
        return;

    int mod = keycode >> 8;
    int key = keycode & 0xff;
    if (mod) {
        skeyset(mod);
        skeyset(key);
        skeyset(key | 0x80);
        skeyset(mod | 0x80);
    } else {
        skeyset(key);
        skeyset(key | 0x80);
    }
}

void flushesc(void)
{
    for (int i = 0; i < escptr; i++) {
        sendkey(keytbl[escseq[i] & 0x7f]);
    }
    escptr = 0;
}

void auxintr(void)
{
    uint8_t data = *(volatile uint16_t *)0xe98006 & 0xff; // SCC data port

#ifdef DEBUG
    _iocs_b_putc('{');
    _iocs_b_putc("0123456789abcdef"[(data >> 4) & 0xf]);
    _iocs_b_putc("0123456789abcdef"[(data >> 0) & 0xf]);
    _iocs_b_putc('}');
#endif

    if (escptr == 0) {
        if (data == '\x1b') {
            escseq[escptr++] = data;        // ESCの場合は後続の文字を待つ
        } else {
            sendkey(keytbl[data & 0x7f]);   // それ以外はキーコードに変換して出力
        }
    } else if (escptr == 1) {
        escseq[escptr++] = data;
        if (data != '[') {
            flushesc();                     // ESC [ でなければここまでの文字を吐き出す
        }
    } else {
        escseq[escptr++] = data;
        if ((data >= '0' && data <= '9') || (data == ';')) {
            if (escptr >= sizeof(escseq) - 1) {
                flushesc();                 // 数字 or ; が来る間はさらに待つが、文字数が制限を超えたら吐き出す
            }
        } else {
            escseq[escptr] = 0;             // エスケープシーケンスが揃った
            for (int i = 0; i < sizeof(esctbl) / sizeof(esctbl[0]); i++) {
                for (int j = 0; ; j++) {
                    if (esctbl[i].escseq[j] != escseq[j]) {
                        break;
                    }
                    if (escseq[j] == '\0') {    // 対応するキーコードに変換する
                        sendkey(esctbl[i].keycode);
                        escptr = 0;
                        break;
                    }
                }
            }
            flushesc();
        }
    }

//    __asm__ volatile ("tst.b 0xe9a001\n");
//    __asm__ volatile ("tst.b 0xe9a001\n");
    *(volatile uint16_t *)0xe98004 = 0x0038; // SCC command port
}

//////////////// ここまでが常駐部

void help(void);

int main(int argc, char **argv)
{
    _dos_print("AUX tap driver for X680x0 version " GIT_REPO_VERSION "\r\n");

    int baudrate = 0;
    int bdset = -1;
    int release = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '/' || argv[i][0] =='-') {
            switch (argv[i][1]) {
            case 's':
                baudrate = atoi(&argv[i][2]);
                break;
            case 'r':
                release = 1;
                break;
            default:
                help();
                break;
            }
        }
    }

    if (baudrate > 0) {
        bdset = 9;
        static const int bauddef[] = { 75, 150, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400 };
        for (int i = 0; i < sizeof(bauddef) / sizeof(bauddef[0]); i++) {
            if (baudrate == bauddef[i]) {
            bdset = i;
            break;
            }
        }
    }
    if (bdset > 0) {
        // stop 1 / nonparity / 8bit / nonxoff
        _iocs_set232c(0x4c00 | bdset);
    }

    _iocs_b_super(0);
    uint32_t oldvect = *(volatile uint32_t *)(0x5c * 4);

    if (release) {
        if (strcmp((char *)(oldvect - 4), "AXT") != 0) {
            _dos_print("auxtapが常駐していません\r\n");
            exit(1);
        }

        *(volatile uint32_t *)(0x5c * 4) = *(volatile uint32_t *)(oldvect - 8);
        _dos_mfree((void *)(*(volatile uint32_t *)(oldvect - 12) - 0xf0));
        _dos_print("auxtapの常駐を解除しました\r\n");
        exit(0);
    }

    if (strcmp((char *)(oldvect - 4), "AXT") == 0) {
        _dos_print("auxtapが既に常駐しています\r\n");
        exit(1);
    }

    *(volatile uint32_t *)(0x5c * 4) = (int)auxintr_asm;

    extern void _start();
    int size = (int)main - (int)_start;
#ifdef DEBUG
    int size = 0xffffffff;
#endif
    _dos_allclose();
    _dos_print("auxtapが常駐しました\r\n");
    _dos_keeppr(size, 0);
}

void help(void)
{
    _dos_print("AUX1からの入力をキー入力として扱います\r\n");
    _dos_print("Usage: auxtap.x [-r][-s<baud>]\r\n");
    exit(1);
}
