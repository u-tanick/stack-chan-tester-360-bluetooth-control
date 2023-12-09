 #include <Arduino.h>

#include <SD.h>
#include <Ticker.h>
#include <M5StackUpdater.h>
#include <M5Unified.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#if defined(ARDUINO_M5STACK_Core2)
  // M5Stack Core2用のサーボの設定
  // Port.A X:G33, Y:G32
  // Port.C X:G13, Y:G14
  // スタックチャン基板 X:G19, Y:G27
  #define SERVO_PIN_X 33
  #define SERVO_PIN_Y 32
#elif defined( ARDUINO_M5STACK_FIRE )
  // M5Stack Fireの場合はPort.A(X:G22, Y:G21)のみです。
  // I2Cと同時利用は不可
  #define SERVO_PIN_X 22
  #define SERVO_PIN_Y 21
#elif defined( ARDUINO_M5Stack_Core_ESP32 )
  // M5Stack Basic/Gray/Go用の設定
  // Port.A X:G22, Y:G21
  // Port.C X:G16, Y:G17
  // スタックチャン基板 X:G5, Y:G2
  #define SERVO_PIN_X 22
  #define SERVO_PIN_Y 21
#elif defined( ARDUINO_M5STACK_CORES3 )
  // M5Stack CoreS3用の設定 ※暫定的にplatformio.iniにARDUINO_M5STACK_CORES3を定義しています。
  // Port.A X:G1 Y:G2
  // Port.B X:G8 Y:G9
  // Port.C X:18 Y:17
  #define SERVO_PIN_X 18 
  #define SERVO_PIN_Y 17
  #include <gob_unifiedButton.hpp> // 2023/5/12現在 M5UnifiedにBtnA等がないのでGobさんのライブラリを使用
  gob::UnifiedButton unifiedButton;
#endif

#include <Avatar.h> // https://github.com/meganetaaan/m5stack-avatar
#include <ESP32Servo.h>

using namespace m5avatar;
Avatar avatar;

#define TIMER_WIDTH 16

/**
 * GeekServo (https://nagomi0132.hateblo.jp/entry/2023/05/02/213233)
 * 時計回り   : 500 - 1500US : 0度
 * 停止       : 1500US : 90度
 * 反時計周り : 1500 - 2500US : 180度
*/
Servo servo_A;
Servo servo_B;

const int STOP_DEGREE = 90;

bool GO_FORWARD = true;

// 停止
void moveStop() {
  servo_A.write(STOP_DEGREE);
  servo_B.write(STOP_DEGREE);
  GO_FORWARD = true;
  avatar.setSpeechText("停止！");
}

// 前進
void moveForward(int deg) {
  moveStop();
  delay(500);
  servo_A.write(deg);
  servo_B.write(deg);
  GO_FORWARD = false;
  avatar.setSpeechText("前進！");
}

// 後退
void moveBack(int deg) {
  moveStop();
  delay(500);
  servo_A.write(deg);
  servo_B.write(deg);
  GO_FORWARD = true;
  avatar.setSpeechText("後退！");
}

// ランダムモード（実装途中）
[[deprecated("sorry, Does not work according to specifications.")]]
void moveRandom() {
  for (;;) {
    moveStop();
    delay(500);
    int rnd_speed = random(0, 6) * 30;
    M5.update();
    if (M5.BtnC.wasPressed()) {
      break;
    }
    servo_A.write(rnd_speed);
    servo_B.write(rnd_speed);
    int delay_time = random(10);
    delay(1000 + 50 * delay_time);
  }
  M5.Speaker.tone(2500, 500);
  GO_FORWARD = true;
}

// BLE接続用変数
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

// BLE接続コールバック関数
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      //M5.Lcd.println("connect");
      //Serial.println("connect");
      avatar.setSpeechText("connect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      //M5.Lcd.println("disconnect");
      //Serial.println("disconnect");
      avatar.setSpeechText("disconnect");
      deviceConnected = false;
      delay(3000);
      ESP.restart();
    }
};

// BLE接続コールバック関数
class MyCallbacks: public BLECharacteristicCallbacks {

  void onRead(BLECharacteristic *pCharacteristic) {
    Serial.println("M5Stack Connected.");
    avatar.setSpeechText("こんにちわ！");
    pCharacteristic->setValue("Hello, I'm M5Stack. Thank you connected. Enjoy!!");
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    Serial.println("Get send data.");
    std::string value = pCharacteristic->getValue();
    Serial.println(value.c_str());
    int num = std::stoi(value.c_str());
    if(num==0 || num==30 || num==60) {
      GO_FORWARD = true;
      moveForward(num);
    } else if(num==120 || num==150 || num==180) {
      GO_FORWARD = false;
      moveBack(num);
    } else if(num==STOP_DEGREE) {
      GO_FORWARD = true;
      moveStop();
    }
  }
};

// BLE接続用サービス起動
void initBLEServise() {
  BLEDevice::init("M5Stack");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();  
}

// setup
void setup() {

  auto cfg = M5.config();     // 設定用の情報を抽出
  cfg.output_power = true;    // Groveポートの出力をしない
  M5.begin(cfg);              // M5Stackをcfgの設定で初期化

  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);
  M5.Log.setEnableColor(m5::log_target_serial, false);

#if defined( ARDUINO_M5STACK_FIRE )
  // M5Stack Fireの場合、Port.Aを使う場合は内部I2CをOffにする必要がある。
  M5.In_I2C.release();
#endif
  servo_A.setPeriodHertz(50);
  servo_B.setPeriodHertz(50);
  if (servo_A.attach(SERVO_PIN_X, 500, 2500)) {
    Serial.print("Error attaching servo A");
  }
  if (servo_B.attach(SERVO_PIN_Y, 500, 2500)) {
    Serial.print("Error attaching servo B");
  }

  avatar.setBatteryIcon(true);
  avatar.setSpeechFont(&fonts::efontJA_16_b);
  avatar.init(8);

  Serial.println("BLE start.");
  avatar.setSpeechText("BLE start.");
  initBLEServise();

}

// loop
void loop() {

  M5.update();

  // 前進/後退
  if (M5.BtnA.wasPressed()) {
    Serial.print("Button A : Go Forward / Back");
    M5.Speaker.tone(1000, 100);
    if(GO_FORWARD) {
      moveForward(60);
    } else {
      moveBack(120);
    }
  }

  // 停止
  if (M5.BtnB.wasPressed()) {
    Serial.print("Button B : Stop");
    M5.Speaker.tone(1500, 100);
    moveStop();
  }

  // 未実装
  if (M5.BtnC.wasPressed()) {
  }

}