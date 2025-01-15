#include <VOneMqttClient.h>
#include <HCSR04.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <Arduino_Json.h>

#define bin_depth 30

const char* Ultrasonic = "a90a3a32-e992-4fee-9025-be93bafe1ff8";
UltraSonicDistanceSensor WasteLevel(41, 42);

Servo servo;
const int servoPin = 4;
const int angleNothing = 90;
const int anglePaper = 0;
const int anglePlastic = 180;

// Google Cloud Pub/Sub
const char* endpoint = "https://pubsub.googleapis.com/v1/projects/iot-usm-446702/subscriptions/detection-sub:pull";
String accessToken = "ya29.c.c0ASRK0GatklfK9l72ZvyrV4aftYZrkM3tSB8DNb7zjr0AnVCaUSUhsmyRO_QkgGjprxMex7zu8WQjgv5Q2bpg77tLKXfeqSVrfiumBy1LssO5ZK_YX111LQRCv3GE7t4tc5HJnHQmmF7k-UKIYz5FkO2xJGh5aEK_DaTByCnuwQJxasrTdc2zK5pwi1HZWLkNoDhyRi4ghTQlZH8vajcpykIPvyrrCS_OWCmbbIOcOy6-qBpVyvufuIsZ0Oxu9PGnVE2fbqjCMlS8d8ekogNw80MCPyFVhzqCv772BGKFb68Md16WtSFL-CQsvNfNV5q_UXQwcTlckrcMneRcphPWBinZT1DJrv_dquppYZFfq2pEDlRMXROfL0QG384AkQyJRryjX_zURdbmnwMlqqbis8lzzSwSbSnyre6-voiabtZY4MYhR9v5iMgtzcu85Z-lthfle3hpzk3Jum--22F9UO0R5XQO-gz76xwbVWOkUX3F2rf1SmB7ofjwr_sS6nyoxQuJ4aQzxeXsQv07lqfhIkazv4e7mf02wU5VbknqXwsojpI67ohvF0yeX5dl1mrpl-m3ebuwyxZvv6from4q5e0ZUfcxbpe03btB1nn5ogOs6UlS2WmMFkz40Zji4q1vZ7U2-wot26a8qje63Ivwlqn-Q72oZO8p4MvRmSR_86_qZ4MZmmFWhvxxeoen8Xjy1Mwd21kbcspl8bayr82bl-R2y4SgF-SincFB-XVzqgfJpaj7Zf-MWaXItazUbwfvvqd_xQUBxe2Bv5wn5-w_Zb_w7S--o2VrkIIWbSFY5_tsiFk8jxFvpj5Q_RY-Z1rSBz5xOOvIJ5Iy59tSlbzauMzXZhht1ztg1nf-4Sj0uMmf29lVaUbsxxhpcBwaBimqSU38FV51xpM5uJijU7I2vtuqhJmUShMvUhQYQyg72X2_tVqXamU9bXvMFsuF5v04ZnaReVvqRtiszR-00uj2vBO04Ijq1WIhcvg8vh-W6f6Zbd7zYQ6WzQa";

// VOne
VOneMqttClient voneClient;
unsigned long lastMsgTime = 0;

// Network
const char* W_SSID = "Wlan17777";
const char* W_PASSWORD = "ichhabedurst";

void setup() {
  Serial.begin(115200);
  servo.attach(servoPin);

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
    // Add MicroServo status here
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
    //Serial.println("Response: " + response);

    JSONVar parsed = JSON.parse(response);
    JSONVar attributes = parsed["receivedMessages"][0]["message"]["attributes"];
    const char* type = (const char*)attributes["type"];
    Serial.print(type);
    moveServo(type);
  }
  http.end();

  delay(10000);
}

void measureWasteLevel() {
  unsigned long cur = millis();
  if (cur - lastMsgTime > INTERVAL) {
    lastMsgTime = cur;

    int sensorValue = -(WasteLevel.measureDistanceCm() - bin_depth);
    voneClient.publishTelemetryData(Ultrasonic, "Distance", sensorValue);
  }
  Serial.println(-(WasteLevel.measureDistanceCm() - bin_depth));
}

void moveServo(const char* type) {
  if (type == "paper")
  {
    servo.write(anglePaper);
    delay(1000);
    servo.write(angleNothing);
  }
  else if (type == "plastic")
  {
    servo.write(anglePlastic);
    delay(1000);
    servo.write(angleNothing);
  }
  else 
  {
    servo.write(angleNothing);
  }
}

