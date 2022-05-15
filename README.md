## 概要

このプログラム、サークルfuzziliaがコミックマーケット参加に合わせて作成中のデジタルお絵かき用キー入力デバイス、通称左手デバイスのファームウェアです。
対象のSoCはnRF52840で、特にAdafruit ItsyBitsy nRF52840 Express開発ボードに合わせて実装しています。プログラムのビルド、書き込みはArduinoで行います。
ESP版は https://github.com/fuzzilia/sh-controller です。(現在非公開となっています。)

これを設定するためのPC側のアプリケーション https://github.com/fuzzilia/sh-config とセットで機能します。

## VSCodeで開発する

- 予めVSCodeとArduinoIDEが入っていること。
    - ArduinoIDEはストア版だとVSCodeが自動認識しない模様。インストーラーで入れるの推奨。
- VSCodeにArudinoプラグインをインストール
    - [Microsoftが出してるやつ](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino)
- VSCodeの設定(settings.json)に下記を追記
    - ```json
      "arduino.additionalUrls": ["https://adafruit.github.io/arduino-board-index/package_adafruit_index.json"]
      ```
    - arduino.additionalUrls : adafruit製ボードを追加できるようにするため。
- コマンドパレットから Arduino Board Managerを呼び出し、Adafruit nRF52を選択、Install
- コマンドパレットから Arduino Library Managerを呼び出し、以下のライブラリをそれぞれInstall
    - Adafruit DotStar
    - Adafruit BusIO
    - Adafruit MPR121
- nrf52 用コンパイラのplatform.txt ファイルのオプション指定を修正
    - compiler.cpp.flags のうち、 gnu++11 を gnu++17 に変更
    - ompiler.c.elf.cmd を arm-none-eabi-g++ に変更
- 画面右下から Select Board でAdafruit ItsyBitsy nRF52840 Express (Adafruit nRF52) を選択
- コマンドパレットから Arduino Upload でプログラム書き込み

platform.txtの変更が気持ち悪ければ、arduino-cliを利用する方針でビルド時にオプションを書き換えても構いません。
shells/build.sh にそのためのコマンドが記載されています。

## LICENSE

MIT
