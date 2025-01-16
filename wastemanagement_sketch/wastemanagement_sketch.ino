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
const char* ackEndpoint = "https://pubsub.googleapis.com/v1/projects/iot-usm-446702/subscriptions/detection-sub:acknowledge";
String accessToken = "ya29.c.c0ASRK0GZ3ZvxBuQi6q3T9eWT38L3X1u-dnA7zs1FX7A4m-uUSuNbmc_OxNTIZaEkpa0pwLqie4YT4mk2PKCCzsXu0FKY79M2xxcxIZMwespNh_mu1LaRd2zSDhaPKYrLSnKSufngv9EcQVZWWThDKblB2m6iIluI5Wsitnet87kb5AX1yGy7nQhGvnNNf0hvJnU-6iljKZfJ9Lvsz9_EzuEAqXavg_5ntvxSZRf5rS8LUDIMAZKn7E_2Wp6Xi2_CpUYV1Vk_r-5k3gvMy7ze9lM7wYObZktR3RqYK0btBA9AD-VYtQfBGL_3Mhp51wekCYG2GDWLHs5m25dHIYvfDav-niL_4h9JC12wx4j-MsdsRGS2zoeJikiZiE385KUUIu89iBYSSX4VkZR2om6SxVp29_dw9JI_ifRyQe5gntcOl0_I3jMYRajdShkJbzdqcu1glfYF04mnw8_8cI1OxaVR1rYW_iVo7UUSOw5VOdfiaQiROqlcv0ROQIrOtystB4OsiSdwWIZFcZqRyXa-Zbp6seVoRfkmnQ0701x-gw5ZBu98rd4XXuBrJZxpJq4zqMqQ0myedaypfmWaf5iedaO8b695otcdQQJraYdhgMfRQS2YQB4v2Jbf8I-qrov_Onyzfd6mwM4Qf_sbXIj613ecYtfo_juYunxVM29IZ2ns020qjr3d-ygoYQdc51S5sorx0jubmt50hnF7uRVsbvvIJ7lpbyeBn66225k9xueIot4dY8OW2p8s6J_hJgw6yoxwrbnkFsajtvyx13mdhbMY9s2YeMmhp4vMiQbyVYZ9mcl0dv4k9e7hFbzrxvRfBv_oX38Q2lqwuB6f5O2qt8vwo6X1F4dg4fO1q4x4ebFc5r4Xto26x24Rb4IyuORn3d5OkJky2UfIZYijwwqkF-b7tkIoqvXX8kdhRphwc7tc1UUFllyb9r0Je2O8z0MQ26JYuvyyOvkwsM8d6tqIyS902kabyUpF_8fz3ls1i7myWx4vv6bcIFy8";

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
    
    JSONVar ackId = parsed["receivedMessages"][0]["ackId"];
    acknowledgeMessage(ackId);

    JSONVar attributes = parsed["receivedMessages"][0]["message"]["attributes"];
    const char* type = (const char*)attributes["type"];
    moveServo(type);
  }
  http.end();

  delay(4000);
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
  Serial.println(type);
    
  if (strcmp((const char*)type, "paper") == 0)
  {
    servo.write(anglePaper);
    delay(1000);
    servo.write(angleNothing);
  }
  else if (strcmp((const char*)type, "plastic") == 0)
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

void acknowledgeMessage(String ackId) {
  HTTPClient http;
  http.begin(ackEndpoint);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + accessToken);
  
  String payload = "{\"ackIds\": [\"" + ackId + "\"]}";

  int httpCode = http.POST(payload);
  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Acknowledgment response:");
    Serial.println(response);
  } else {
    Serial.print("Error on acknowledgment request: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
}
