# SmartGarden
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

SmartGardenは、Raspberry PiとESP32を使用して屋外の環境情報を計測し、ElasticsearchとGrafanaを使って環境情報を可視化するシステムです。

## 機能
- 屋外の温度、湿度、気圧、土壌の湿度を計測
- ElasticsearchとGrafanaを使って環境情報を可視化
- システム構成 -> [system_structure](https://github.com/qoopen0815/SmartGarden/blob/main/docs/system_structure.png)

## 必要なもの
- Raspberry Pi
- ESP32
- 温湿度センサ
- 土壌湿度センサ

## インストール方法

### データサーバ側（Raspberry Pi）
1. Raspberry Piをセットアップします。
2. Raspberry Pi上に[docker-compose.yml](https://github.com/qoopen0815/SmartGarden/blob/main/server/docker-compose.yml)を使ってElasticsearchをセットアップします。
3. Grafanaをインストールします。

### エッジデバイス側（ESP32）
1. PlatformIOで[data_logger](https://github.com/qoopen0815/SmartGarden/tree/main/edge/data_logger)を開きます。
2. Wi-Fi情報やElasticsearchのホスト情報を修正して、ESP32に書き込みます。
3. ESP32に必要なセンサや機器を接続します。

### ダッシュボード作成
Grafanaの公式マニュアルに沿ってダッシュボードを作成します。  
このシステムでアップロードされるデータの構造は以下の通りです。
（※他の値は追加予定）

```
{
    @timestamp: date,
    soil_moisture: long
}
```

また、データのライフサイクル設定（ilm policy）は以下の通りです。  
この設定は[こちら](https://github.com/qoopen0815/SmartGarden/blob/2a4746662acee0ee77d22576b2f95f3a7cc70290/edge/data_logger/src/main.cpp#L141-L160)で変更できます。

|フェーズ|期限|最大データ量|
|----|----|----|
|Hot|1d|10gb|
|Warm|2d|10gb|
|Cold|5d|10gb|
|Delete|7d|-|

## ライセンス
このプロジェクトはMITライセンスの元で公開されています。詳細は[LICENSE](https://github.com/qoopen0815/SmartGarden/blob/main/LICENSE)をご確認ください。
