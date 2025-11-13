Human68kのドライブの内部情報を表示するツール (drvinfo)
======================================================

## 概要

Human68kがドライブやファイルディスクリプタを管理するために使用している内部情報を表示します。

## 実行例

```
$1c15.b: カレントドライブ番号 = 4 (E)
$1c73.b: LASTDRIVE = 25 (Z)
$1c74.b: 最大ドライブ番号(固定) = 25
$1c7e- : ドライブ交換テーブル : D E A B C F I H Z J K L M N O P Q R S T U V W X Y G 
$1c3c.l: DPBテーブル = 0x0148ae
$1c38.l: カレントディレクトリテーブル = 0x01407a
 A:->D: 40 DPB:0x0148d2(->0x01490a) 0x01097e(DISK2HD) #0 1024 :\
 B:->E: 40 DPB:0x01490a(->0x014942) 0x01097e(DISK2HD) #1 1024 :\
 C:->A: 40 DPB:0x0148ae(->0x0148ba) 0x014866(EMUHOST) #0    0 :\
 D:->B: 40 DPB:0x0148ba(->0x0148c6) 0x014866(EMUHOST) #1    0 :\
 E:->C: 40 DPB:0x0148c6(->0x0148d2) 0x014866(EMUHOST) #2    0 :\m68k\x68kmisc\drvinfo
 F:->F: 40 DPB:0x014942(->0x08dca2) 0x010da2(HARDDSK) #0 1024 :\
 G:->I: 40 DPB:0x09b0fc(->0xffffff) 0x099f7c(LOOPDRV) #0 1024 :\
 H:->H: 40 DPB:0x096014(->0x09b0fc) 0x08dcda(CDROM  ) #0    0 :\
 Z:->G: 40 DPB:0x08dca2(->0x096014) 0x08d950(C-ram  ) #0 1024 :\
$1c6e.w: 最大ファイルハンドル数 = 32
 fd=00 FCB:0x13d30 count=1 attr=0xc1(C) 0x5292c CON_____.___
 fd=01 FCB:0x13f10 count=2 attr=0x62(B) 0x148c6 a_______.___
 fd=02 FCB:0x13df0 count=1 attr=0xc2(C) 0x5292c CON_____.___
 fd=03 FCB:0x13e50 count=1 attr=0xc0(C) 0x51da4 AUX_____.___
 fd=04 FCB:0x13eb0 count=1 attr=0xc0(C) 0x1076e PRN_____.___
 fd=05 FCB:0x13d90 count=1 attr=0xc2(C) 0x5292c CON_____.___
 fd=06 FCB:0x13f10 count=2 attr=0x62(B) 0x148c6 a_______.___
```

## 参考情報

Human68kのワークエリアの情報は以下を参考にしました。
* [ぷにぐらま～ずまにゅある](https://github.com/kg68k/puni) by 立花@桑島技研 氏
  * [oswork.txt](https://github.com/kg68k/puni/blob/main/oswork.txt)
* [susie.x](http://retropc.net/x68000/software/disk/scsi/susie/) by GORRY 氏
  * アーカイブ内のソースコード
