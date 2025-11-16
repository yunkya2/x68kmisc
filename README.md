# X68000 関連の雑多なツール類

テスト目的などに作った雑多なツール類をまとめたディレクトリです。


ツール名                 | 概要
-------------------------|----------------------------------------------------
[drives](drives/)        |ドライバ名を元にドライブレターを並び替える
[history-bc](history-bc/)|HISTORY.Xのキーバインドをbashっぽく改造
[xdftool](xdftool/)      |XDFフォーマットのディスクイメージを作成・展開
[drvinfo](drvinfo/)      |Human68kのドライブの内部情報を表示
[iobuf](iobuf/)          |Human68kのI/Oバッファの情報を表示
[stackfree](stackfree/)  |Human68k スーパバイザスタック消費量の表示
[usefile](usefile/)      |指定したファイルをオープンしたままにする

ソースコードのみが置かれているツールは、コンパイルに[elf2x68k](https://github.com/yunkya2/elf2x68k)を使用します。

各ツールの詳細については、各サブディレクトリ内のREADME.mdを参照してください。
