
#include <PID_v1.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================== 引脚定义 ====================

// DS18B20 温度传感器
#define ONE_WIRE_BUS D5

// 继电器控制引脚
#define RELAY_PIN D6

// OLED 显示屏
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

// ==================== 温度传感器初始化 ====================

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ==================== PID 控制参数 ====================

double Setpoint = 25.0;  // 目标温度
double Input = 0;        // 当前温度
double Output = 0;       // PID 输出

double Kp = 2.0;
double Ki = 5.0;
double Kd = 1.0;

// 创建 PID 控制器
PID myPID(&Input, &Output, &Setpoint,
          Kp, Ki, Kd, DIRECT);

// ==================== OLED 初始化 ====================

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ==================== 初始化程序 ====================

void setup() {

  // 初始化串口
  Serial.begin(9600);

  // 初始化温度传感器
  sensors.begin();

  // 设置继电器引脚
  pinMode(RELAY_PIN, OUTPUT);

  // 默认关闭加热器
  digitalWrite(RELAY_PIN, LOW);

  // 初始化 PID
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 255);

  // 设置 PID 计算间隔
  myPID.SetSampleTime(1000);

  // 初始化 I2C
  Wire.begin(D2, D1);

  // 初始化 OLED
  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS
      )) {

    Serial.println("OLED 初始化失败！");
    while (1);
  }

  // OLED 启动画面
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("Temperature Control");
  display.println("-------------------");
  display.println("System Starting...");

  display.display();

  delay(2000);

  display.clearDisplay();
}

// ==================== 主循环程序 ====================

void loop() {

  // 请求温度数据
  sensors.requestTemperatures();

  // 获取当前温度
  float temperature = sensors.getTempCByIndex(0);

  // 检查传感器是否正常
  if (temperature == DEVICE_DISCONNECTED_C ||
      temperature < -55 ||
      temperature > 125) {

    Serial.println("温度传感器读取失败！");

    // 传感器故障时关闭加热器
    digitalWrite(RELAY_PIN, LOW);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Sensor Error!");
    display.println("Check DS18B20");
    display.display();

    delay(1000);
    return;
  }

  // 更新 PID 输入值
  Input = temperature;

  // PID 计算
  myPID.Compute();

  // ==================== 继电器控制 ====================

  // 根据 PID 输出控制加热器
  if (Output > 128 && Input < Setpoint) {

    digitalWrite(RELAY_PIN, HIGH);

  } else {

    digitalWrite(RELAY_PIN, LOW);

  }

  // ==================== 串口显示 ====================

  Serial.print("当前温度：");
  Serial.print(temperature);
  Serial.print(" C");

  Serial.print("  目标温度：");
  Serial.print(Setpoint);
  Serial.print(" C");

  Serial.print("  PID输出：");
  Serial.println(Output);

  // ==================== OLED 显示 ====================

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Temperature Control");

  display.drawLine(0, 12, 128, 12, SSD1306_WHITE);

  display.setCursor(0, 18);
  display.print("Current: ");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 30);
  display.print("Setpoint: ");
  display.print(Setpoint, 1);
  display.println(" C");

  display.setCursor(0, 42);
  display.print("PID Output: ");
  display.println(Output);

  display.setCursor(0, 54);

  if (digitalRead(RELAY_PIN) == HIGH) {
    display.println("Heater: ON");
  } else {
    display.println("Heater: OFF");
  }

  display.display();

  // 每隔一秒更新
  delay(1000);
}
