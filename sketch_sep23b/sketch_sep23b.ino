#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// ==============================
// Wi-Fi 설정
// ==============================
const char* ssid = "신동환";
const char* password = "0987654321";

// ==============================
// 핀 설정
// ==============================
#define DHTPIN D7
#define DHTTYPE DHT11

const int ledPin = D5;

// ==============================
// 기준 온도 / 습도
// ==============================
const float TEMP_LIMIT = 28.0;
const float HUM_LIMIT = 70.0;

// ==============================
// 객체 생성
// ==============================
DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(80);

// ==============================
// 센서 값
// ==============================
float temperature = 0;
float humidity = 0;

bool sensorOK = false;

bool tempAlert = false;
bool humidityAlert = false;
bool alertState = false;

bool ledState = false;

// ==============================
// 시간 관리
// ==============================
unsigned long lastSensorRead = 0;

const unsigned long SENSOR_INTERVAL = 2000;

// 정상 상태
const unsigned long NORMAL_CYCLE = 60000; // 1분
const unsigned long NORMAL_LED_ON = 15000; // 15초

// 이상 상태
const unsigned long ALERT_BLINK = 5000; // 5초


// ==============================
// 센서 읽기
// ==============================
void readSensor() {

  unsigned long now = millis();

  if (now - lastSensorRead < SENSOR_INTERVAL) {
    return;
  }

  lastSensorRead = now;

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // 센서 읽기 실패
  if (isnan(h) || isnan(t)) {

    sensorOK = false;

    Serial.println("DHT11 센서 읽기 실패");

    return;
  }

  sensorOK = true;

  temperature = t;
  humidity = h;

  // 기준값 초과 여부
  tempAlert = temperature >= TEMP_LIMIT;
  humidityAlert = humidity >= HUM_LIMIT;

  alertState = tempAlert || humidityAlert;

  Serial.print("온도: ");
  Serial.print(temperature);
  Serial.print(" ℃ / 습도: ");
  Serial.print(humidity);
  Serial.println(" %");
}


// ==============================
// LED 제어
// ==============================
void controlLED() {

  unsigned long now = millis();

  // ------------------------------
  // 이상 상태
  // ------------------------------
  if (alertState) {

    // 5초마다 ON/OFF 전환
    bool newState = ((now / ALERT_BLINK) % 2 == 0);

    ledState = newState;

    digitalWrite(
      ledPin,
      ledState ? HIGH : LOW
    );
  }

  // ------------------------------
  // 정상 상태
  // ------------------------------
  else {

    unsigned long cycleTime = now % NORMAL_CYCLE;

    // 처음 15초
    if (cycleTime < NORMAL_LED_ON) {

      ledState = true;

      digitalWrite(
        ledPin,
        HIGH
      );
    }

    // 이후 45초
    else {

      ledState = false;

      digitalWrite(
        ledPin,
        LOW
      );
    }
  }
}


// ==============================
// React에서 요청할 API
// ==============================
void handleStatus() {

  // React CORS 허용
  server.sendHeader(
    "Access-Control-Allow-Origin",
    "*"
  );

  String json = "{";

  // 센서 정상 여부
  json += "\"sensorOK\":";
  json += sensorOK ? "true" : "false";

  json += ",";

  // 온도
  json += "\"temperature\":";

  if (sensorOK) {
    json += String(temperature, 1);
  }
  else {
    json += "null";
  }

  json += ",";

  // 습도
  json += "\"humidity\":";

  if (sensorOK) {
    json += String(humidity, 1);
  }
  else {
    json += "null";
  }

  json += ",";

  // 온도 경고
  json += "\"tempAlert\":";
  json += tempAlert ? "true" : "false";

  json += ",";

  // 습도 경고
  json += "\"humidityAlert\":";
  json += humidityAlert ? "true" : "false";

  json += ",";

  // 전체 경고
  json += "\"alert\":";
  json += alertState ? "true" : "false";

  json += ",";

  // LED 상태
  json += "\"led\":";
  json += ledState ? "true" : "false";

  json += ",";

  // 기준값
  json += "\"tempLimit\":";
  json += String(TEMP_LIMIT, 1);

  json += ",";

  json += "\"humidityLimit\":";
  json += String(HUM_LIMIT, 1);

  json += "}";

  server.send(
    200,
    "application/json",
    json
  );
}


// ==============================
// ESP 접속 확인용
// ==============================
void handleRoot() {

  server.send(
    200,
    "text/plain; charset=utf-8",
    "ESP8266 Sensor Server Running"
  );
}


// ==============================
// setup
// ==============================
void setup() {

  Serial.begin(115200);

  delay(100);

  // LED
  pinMode(
    ledPin,
    OUTPUT
  );

  digitalWrite(
    ledPin,
    LOW
  );

  // DHT11 시작
  dht.begin();

  // Wi-Fi 연결
  Serial.println();
  Serial.print("Wi-Fi 연결 중: ");
  Serial.println(ssid);

  WiFi.begin(
    ssid,
    password
  );

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi 연결 완료!");

  Serial.print("ESP8266 IP: ");
  Serial.println(
    WiFi.localIP()
  );

  // API
  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );

  server.on(
    "/status",
    HTTP_GET,
    handleStatus
  );

  // 서버 시작
  server.begin();

  Serial.println(
    "웹 서버 시작"
  );

  Serial.print(
    "상태 API: http://"
  );

  Serial.print(
    WiFi.localIP()
  );

  Serial.println(
    "/status"
  );
}


// ==============================
// loop
// ==============================
void loop() {

  server.handleClient();

  readSensor();

  controlLED();
}
