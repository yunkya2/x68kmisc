Human68kのI/Oバッファの情報を表示するツール (iobuf)
===================================================

## 概要

Human68kがファイルシステムのデータをバッファリングするために使用しているI/Oバッファの情報を表示します。

デフォルトではI/OバッファをLRU順に表示しますが、引数を指定すると番号順に表示します。

## 実行例

```
$1c34.l: I/Oバッファ先頭アドレス = 0x1497a
$1c72.b: バッファ数 = 99
$1c70.w :バッファサイズ = 1024
$b6b6.l: I/OバッファLRU先頭アドレス = 0x169fa

I/Oバッファリスト(LRU順)
 [08] 0x169fa: next=09 prev=-1 fat Z:0x000006 dpb=0x08dca2 unit=0 C-ram
 [09] 0x16e0a: next=10 prev=08 fat Z:0x000005 dpb=0x08dca2 unit=0 C-ram
 [10] 0x1721a: next=11 prev=09 fat Z:0x000004 dpb=0x08dca2 unit=0 C-ram
 [11] 0x1762a: next=12 prev=10 fat Z:0x000003 dpb=0x08dca2 unit=0 C-ram
 [12] 0x17a3a: next=13 prev=11 fat Z:0x000002 dpb=0x08dca2 unit=0 C-ram
 [13] 0x17e4a: next=14 prev=12 fat Z:0x000001 dpb=0x08dca2 unit=0 C-ram
 [14] 0x1825a: next=15 prev=13 dir Z:0x000007 dpb=0x08dca2 unit=0 C-ram
 [15] 0x1866a: next=16 prev=14 fat F:0x00004f dpb=0x014942 unit=0 HARDDSK
 [16] 0x18a7a: next=17 prev=15 fat F:0x00004e dpb=0x014942 unit=0 HARDDSK
 [17] 0x18e8a: next=18 prev=16 fat F:0x00004d dpb=0x014942 unit=0 HARDDSK
 [18] 0x1929a: next=19 prev=17 fat F:0x00004c dpb=0x014942 unit=0 HARDDSK
 [19] 0x196aa: next=20 prev=18 fat F:0x00004b dpb=0x014942 unit=0 HARDDSK
 [20] 0x19aba: next=21 prev=19 fat F:0x00004a dpb=0x014942 unit=0 HARDDSK
 [21] 0x19eca: next=22 prev=20 fat F:0x000049 dpb=0x014942 unit=0 HARDDSK
              :
```

## 参考情報

Human68kのワークエリアの情報は以下を参考にしました。
* [ぷにぐらま～ずまにゅある](https://github.com/kg68k/puni) by 立花@桑島技研 氏
  * [oswork.txt](https://github.com/kg68k/puni/blob/main/oswork.txt)
