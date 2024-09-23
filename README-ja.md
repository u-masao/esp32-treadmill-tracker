# Treadmill Tracker

このプロジェクトは、トレッドミルの磁気センサーを利用して走行距離を計算し、ログを送信する ESP32 ベースのトラッカーです。

## 特徴

- トレッドミルのフライホイールに設置された磁石と磁気センサーを利用して走行距離を計測
- 計測データを HTTP POST で指定の URL に送信（HTTPS にも対応）
- ESP32 開発キットで動作確認済み
- Arduino IDE で簡単にビルド可能

## 動作確認済みトレッドミル

- [ALINCO EXW8023](https://www.alinco.co.jp/product/fitness/detail/id=6880)

## 必要物

-  ESP32 開発キット
  - 例 -> [example](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/index.html)
- 磁石と磁気センサーを内蔵したトレッドミル
  - 工夫すれば後から磁石と磁気センサーを取り付けることも可能

## 利用方法

1. Arduino IDE で ESP32 開発キット等のボード設定を行います。
1. treadmill-tracker/treadmill-tracker.ino を Arduino IDE で開きます。
1. treadmill-tracker/credentials.example を参考に treadmill-tracker/credentials.h を作成し、Wi-Fi の設定、ロギング API の URL と認証情報を記述します。
1. ビルドを行い、ESP32 デバイスに書き込みます。
1. シリアルモニタで動作状況を確認できます。

## 設定

フライホイール1回転あたりの走行距離は、treadmill-tracker.ino の MILLIMETER_PAR_STEP で設定します。利用するトレッドミルで実測するなどして算出した値を設定してください。

```
// treadmill-tracker.ino : 25 行目
#define MILLIMETER_PAR_STEP 134.0
```

## データ送信

計測データは、HTTP POST で指定の URL に JSON 形式で送信されます。
およそ60秒ごとに送信しますが精度は時間間隔の精度は高くありません。
タイムスタンプも送信していますが、ESP32 のタイムスタンプは当てにならないのでロギングAPI側で時刻を見ることを推奨します。

```
{"esp32":{"ts":1234567,"distance":123.456}}
```

- ts: エポックタイム(秒)
- distance: ESP32 の電源投入時から現在までの走行距離（メートル）

## 免責事項

このプロジェクトは、個人の hobby project として開発されたものであり、いかなる保証も提供しません。利用者の責任においてご使用ください。

## 権利

著作権等の権利は u-masao が保持します。

## GitHub リポジトリ

https://github.com/u-masao/esp32-treadmill-tracker
