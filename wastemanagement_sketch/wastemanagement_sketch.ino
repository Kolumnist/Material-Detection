#include <VOneMqttClient.h>
#include <HCSR04.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <Arduino_Json.h>

#define bin_depth 30

const char* Ultrasonic = "a90a3a32-e992-4fee-9025-be93bafe1ff8";
UltraSonicDistanceSensor WasteLevel(41, 42);
const int Red_LED = 7;

const char* cam = "18302e6f-a426-4054-ace9-fd5934c0a2a1";
const char* servoVOne = "ab216ab9-2219-4984-8317-6b3d89835a25";
Servo servo;
const int servoPin = 4;
const int angleNothing = 90;
const int anglePaper = 0;
const int anglePlastic = 180;

// Google Cloud Pub/Sub
const char* endpoint = "https://pubsub.googleapis.com/v1/projects/iot-usm-446702/subscriptions/detection-sub:pull";
const char* ackEndpoint = "https://pubsub.googleapis.com/v1/projects/iot-usm-446702/subscriptions/detection-sub:acknowledge";
String accessToken = "ya29.c.c0ASRK0GYOvb_t0ofsK1g2iIUVMa01g4PtrtK3XbDu2wmHuq73YRwCroavjPX3zZ3di4Nd-CBOUbw-pSJOM95aSy6CDzSUp-8fc5yvKJhKvRkDofvk16RR57nvTlO7JSM8MtwpdlTS6O_4I_WM6kJ43tac3vgYMoTf9TlenmUip3JjgtvaB2IPq5CjUpXZrebkqupIwcNc2yuhAT66kmF5Z3YyWh_W6ts_7PUE-x2GowX45IsUUiMzRs4PU-7KIvkJuj6YZyqb2kodCs501aAsr-LTzyS8YWhbMw4y5bKJHZp276ooTxsBhvU1zjo5QTiPO5POlgGWby0KWW4M0beccFOVzEqxedbjlPViBvf0BJCqxLJwFBG3A5ErSwE387Ar7at-pWQJSBwh9rdc7QFZkcmzVxmrmiM0W70enb0t0FfM6mFVeaqw7R2Z6xMwUrIMcot14wc0haBqY6iXV63X3ZlR6X8aU5mjeuBMjXs9IxYU3thx5SYFkspY8ji3vJwBSoBb0Mfu1WBFIMcrqScO7dO0UkS3JdXtUm1W7W8fflevqjYyOemx5rjr-i0xbvZV3eXik9pbau8M52iBeial4t4IZW4WW1F-xSn-_8WRr-F0v4yjOBIq32kFJle6cQk-cne3X0IZWbpYSdgydFBgpe-3agV51fdMriaoh-Rd_p-ll8iB_Y3Rgw8schvtUxZVYBfcSnx-rhx9ooMMnVFlcZJdzZlRygzhg2Z4du-MBg1FYQxtussJI11cjhuhgbJeyvu4foWmhm4xJMiizzovyt6wjrznJdnnzI45MtYYkI0cfJQFWd3yU6gUjzYedigjbO9f9vd-nipgzVUMtJYcyFJyWkcfmdzke1RoFx2Zwn3g7Fmv2zUdrmXx3MphYZY0XrSh8JialFqXan6cOUfyefR6ewS81hRl9g6-Fh9Uk7XzpIgO8d3rcMcoFoFlaoB-dooVY12ujFyxv1mYSRe9liY1sc42nI-Qvq7FyZtctVpkgr4r558z1o0";

// VOne
VOneMqttClient voneClient;
unsigned long lastMsgTime = 0;

// Network
const char* W_SSID = "Wlan17777";
const char* W_PASSWORD = "ichhabedurst";

void setup() {
  Serial.begin(115200);
  servo.attach(servoPin);

  pinMode(Red_LED, OUTPUT);

  servo.write(90);
  delay(1000);
  servo.write(0);
  delay(1000);
  servo.write(90);
  delay(1000);
  servo.write(180);
  delay(1000);
  servo.write(90);

  setup_wifi();
  voneClient.setup();
}

void setup_wifi() {
  delay(50);
  WiFi.mode(WIFI_STA);
  WiFi.begin(W_SSID, W_PASSWORD);
  Serial.print(String("Connecting to ") + W_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
  }
  Serial.println("WiFi connected");
}

void loop() {
  if (!voneClient.connected()) {
    voneClient.reconnect();
    voneClient.publishDeviceStatusEvent(Ultrasonic, true);
    Serial.println("");
  }
  voneClient.loop();
  measureWasteLevel();

  HTTPClient http;
  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + accessToken);

  String payload = R"({"maxMessages": 1})";
  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Response: " + response);
    Serial.println("");

    JSONVar parsed = JSON.parse(response);

    JSONVar ackId = parsed["receivedMessages"][0]["ackId"];
    acknowledgeMessage(ackId);

    JSONVar attributes = parsed["receivedMessages"][0]["message"]["attributes"];
    const char* type = (const char*)attributes["type"];
    moveServo(type);
  }
  http.end();

  delay(1000);
}

void measureWasteLevel() {
  unsigned long cur = millis();
  if (cur - lastMsgTime > INTERVAL) {
    lastMsgTime = cur;

    int sensorValue = -(WasteLevel.measureDistanceCm() - bin_depth);
    voneClient.publishTelemetryData(Ultrasonic, "Distance", sensorValue);

    if (sensorValue >= (bin_depth * 0.7)) {
      digitalWrite(Red_LED, HIGH);
    } else {
      digitalWrite(Red_LED, LOW);
    }
  }
}

void moveServo(const char* type) {
  Serial.println(type);

  if (strcmp((const char*)type, "paper") == 0) {
    servo.write(anglePaper);
    delay(3000);
    servo.write(angleNothing);
  } else if (strcmp((const char*)type, "plastic") == 0) {
    servo.write(anglePlastic);
    delay(3000);
    servo.write(angleNothing);
  } else {
    servo.write(angleNothing);
  }
  voneClient.publishTelemetryData(cam, "Type", type);
}

void acknowledgeMessage(String ackId) {
  HTTPClient http;
  http.begin(ackEndpoint);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + accessToken);

  String payload = "{\"ackIds\": [\"" + ackId + "\"]}";

  int httpCode = http.POST(payload);
  if (httpCode > 0) {
    String ackId = http.getString();
    Serial.println("Acknowledgment response:");
    Serial.println(ackId);
  } else {
    Serial.print("Error on acknowledgment request: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
}
