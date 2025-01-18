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
String accessToken = "ya29.c.c0ASRK0GYuVoLdZkiUITN2KuZE0r3EncARthkCnVg2LFzLBcHEmzvV6irNnr8jqA4zNvKq2362JDLRu-GbtA5F27rfg_juOBFnRdEol66cxUHKg1t7DsqmHU2wiCr9de6AVs23szpK7a9fJh_RN1-0h3CTE4IEbDWSgPcmcIO02VSGusOC4t28bO69wShnai_O14Ax8pJRyZCvhHH-UNNn-JYlV1sbRRSt7sQRooZ9MrLACgxkRBmbjFnIpLr-y9qvBbAiTqQF2Ih2X-6pPxQ_x06yTzJ94a_cbG1vygClhlBEIAQJQCgr-AEOSQrICjQK5zuxu_F-ze6pHjpqZBTAFc878L6TONCbfDlV9Am7nVGDeFccY5FUp2_dT385Aear4r2reZ7ni3U4Qg36YoIJfk01s_nh4bmli9OVBhUjrbIBWbUtuMn6livYZc-yQbS04q4yxeaRty6jzS1ZIo06y1pd6bih663ldcV8SRmrfny_FIwl-WMiza9vb53mmz8M4Sx-41My-hepMehZ-o0770gO2y_nlz2M0yavai7B9zkpnWtcJgVsZdgg0eedt65zVbn24fjXhsV0xgjRxl_Oj7g57I-6_y6rJFbstrUs1s6-n9mc_tdY1RzXaRjX1Fb3wcjRYyOpwOvn6RhsRno7JMFfOtkI0QfaUjqtg4O78eBwzZ8Js-jQJqk0YYFwVRSnbhrm0VSZx2lstx2rR_zr-SccMV6v23WIfamanpF6xkuxa5gXRSu_mn5Vug3ixisvwQ3ce9WBM4BzmUp6bVy-fY5RiirrpqrvhMZuiZYixMuJreYmkzkmBUmMhIYjR_tVdfV8yIdenx0waVsgl8-i_9pirIQRzgozlvFiv49lhU3nc6buJ9uZqc2Isc98gniojUtfvvJoIpJJhRZ6xJmjZbwzcMbr0X77nM-hSz87m9nqd1zBYiF-rzj4thcR7wpXUbdvg3y14yqr1Wxsk7_UhkpXf6Rh3M7g6uVkWi0vvXgvgVkm63UfUhj";

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
    Serial.println();
  }
  voneClient.loop();
  measureWasteLevel();

  HTTPClient http;
  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + accessToken);

  String payload = R"({"maxMessages": 100})";
  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    String response = http.getString();
    //Serial.println("Response: " + response);
    
    JSONVar parsed = JSON.parse(response);
    acknowledgeMessages(response);

    JSONVar attributes = parsed["receivedMessages"][0]["message"]["attributes"];
    const char* type = (const char*)attributes["type"];
    moveServo(type);
  }
  http.end();

  delay(2000);
}

void measureWasteLevel() {
  unsigned long cur = millis();
  if (cur - lastMsgTime > INTERVAL) {
    lastMsgTime = cur;

    int sensorValue = -(WasteLevel.measureDistanceCm() - bin_depth);
    voneClient.publishTelemetryData(Ultrasonic, "Distance", sensorValue);

    if (sensorValue >= (bin_depth * 0.8)) {
      digitalWrite(Red_LED, HIGH);
    }
    else {
      digitalWrite(Red_LED, LOW);
    }
  }
}

void moveServo(const char* type) {
  Serial.println(type);
    
  if (strcmp((const char*)type, "paper") == 0)
  {
    servo.write(anglePaper);
    delay(2000);
    servo.write(angleNothing);
  }
  else if (strcmp((const char*)type, "plastic") == 0)
  {
    servo.write(anglePlastic);
    delay(2000);
    servo.write(angleNothing);
  }
  else 
  {
    servo.write(angleNothing);
  }
  voneClient.publishTelemetryData(cam, "Type", type);
}

void acknowledgeMessages(JSONVar response) {
  HTTPClient http;
  http.begin(ackEndpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + accessToken);

  JSONVar receivedMessages = response["receivedMessages"];

  int totalMessages = receivedMessages.length();
  String payload = "{\"ackIds\": [";
  int batchCount = 0;

  for (int i = 0; i < totalMessages; i++) {
    String ackId = String((const char*)receivedMessages[i]["ackId"]);
    
    if (ackId.isEmpty()) {
      Serial.println("Skipping invalid or empty ackId.");
      continue;
    }

    if (batchCount > 0 && batchCount < totalMessages) payload += ", ";
    payload += "\"" + ackId + "\"";
    batchCount++;

    // Send acknowledgment as batch
    if (batchCount == 100 || i == totalMessages-1) {
      payload += "]}";
      int httpCode = http.POST(payload);

      if (httpCode > 0) {
        String response = http.getString();
        Serial.println("Acknowledgment response:");
        Serial.println(response);
      } else {
        Serial.print("Error on acknowledgment request: ");
        Serial.println(http.errorToString(httpCode));
      }
    }
  }

  http.end();
}
