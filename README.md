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
- コマンドパレットから Arduino Library Managerを呼び出し、Adafruit DotStarを選択、それぞれInstall
- 画面右下から Select Board でAdafruit ItsyBitsy nRF52840 Express (Adafruit nRF52) を選択
- コマンドパレットから Arduino Upload でプログラム書き込み

## LICENSE

MIT
