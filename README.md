# stack-chan-tester-360-bluetooth-control

本リポジトリは [tack-chan-tester-360](https://github.com/u-tanick/stack-chan-tester-360) をもとに、
Bluetoothで接続したAndroidスマホから、サーボモーターの回転を制御する仕組みを追加したものです。

サーボモーターに関する仕組みは、[stack-chan-tester](https://github.com/mongonta0716/stack-chan-tester) を、
Bluetooth接続の仕組みは、[BLEでM5Stackと通信](https://coskxlabsite.stars.ne.jp/html/android/BluetoothLE/bluetoothLE.html) を参考にさせていただきました。
感謝です！

## 動作確認

Core2 AWS, Core2 v1.1　で動作確認済み。


## 使い方

### Bluetooth接続 (※)

1. Androidスマホに、別途公開している、[tack-chan-tester-360](https://github.com/u-tanick/stack-chan-bluetooth-control-client-app) アプリをインストール。
  - リポジトリにapkファイルも格納しています。
  - 自作apkとなるため、「提供元不明のアプリ」のインストール許可が必要となります。
  - ESETを入れたスマホでセキュリティチェック済みですが、インストールの際に発生する責任は負いかねます。ご了承ください。
2. 本リポジトリのビルド結果をインストールしたM5Stackを起動。
3. M5StackとスマホをBluetoothで接続。
4. スマホアプリを起動し、[connect]ボタンを押し、アプリとの接続を確立。
5. アプリにある各種ボタンを操作して、サーボモーターを動作させる。

※ Bluetooth接続には、BLE(Bluetooth Low Energy)を利用しています。

### 単体での機能

[tack-chan-tester-360](https://github.com/u-tanick/stack-chan-tester-360) と同じく、M5StackのABCボタンに下記の機能が割り当てられています。

* Aボタン
  * 押すたびに360度回転サーボの正転／負転を切り替えます。
* Bボタン
  * サーボの回転を止めます。
* Cボタン
  * サーボの正転／負転をランダムに切り替えながら動作します。


M5Stack (Core2を想定）に、[Stack-chan_Takao_Base](https://www.switch-science.com/products/8905?variant=42653590913222)
や
[ｽﾀｯｸﾁｬﾝ タカオ版 部品キット](https://mongonta.booth.pm/items/3520339)
を接続し、360度回転サーボを付けた状態で起動させてください。

サーボモーターは2個同時に動かせますが、M5Stack Core2単体のバッテリーだけだと正転／負転を切り替えの不可から落ちることが多いです。
そのため外部から給電するか、サーボモーターは1個での使用がよさそうです。
