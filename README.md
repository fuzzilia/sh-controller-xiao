## 概要

このプログラム、サークルfuzziliaがコミックマーケットにサークル参観する際に作成したデジタルお絵かき用キー入力デバイス、通称左手デバイスのファームウェアです。
対象のSoCはESP32で、プログラムのビルド、書き込みはArduinoで行います。

これを設定するためのPC側のアプリケーション https://github.com/fuzzilia/sh-config とセットで機能します。

## VSCodeで開発する

- 予めVSCodeとArduinoIDEが入っていること。
    - ArduinoIDEはストア版だとVSCodeが自動認識しない模様。インストーラーで入れるの推奨。
- VSCodeにArudinoプラグインをインストール
    - [Microsoftが出してるやつ](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino)
- VSCodeの設定(settings.json)に下記を追記
    - ```json
      "arduino.additionalUrls": ["https://www.adafruit.com/package_adafruit_index.json"]
      ```
    - arduino.additionalUrls : adafruit製ボードを追加できるようにするため。
- コマンドパレットから Arduino Board Managerを呼び出し、Adafruit nRF52を選択、Install
- コマンドパレットから Arduino Library Managerを呼び出し、以下を選択、それぞれInstall
  - Adafruit DotStar
  - Adafruit SPIFlash
- 画面右下から Select Board でAdafuit ItsyBitsy nRF52840 Express (Adafruit nRF52) を選択
- コマンドパレットから Arduino Upload でプログラム書き込み

## LICENSE

MIT