# Intelligent Waste System
 
Near Real-time Waste Bin Level Detection system, with advanced features such as AI to divide the type of trash. In our current model it detects either paper or plastic.
We use YOLO, ESP32, VOne and Google Cloud Pub/Sub.

## Key Features
- Object detection via YOLO. Paper or Plastic.
- Cloud-based data handling with Pub/Sub.
- Cloud-based data analyzation with VOne.
- Integration with ESP32 for hardware response via micro servo.
- Ultrasonic Sensor for bin level measuring, red LED lights up when bin too full

## System Architecture
![Alt text](./architecture/DataFlowArchitecture.png "Flow Diagramm")
*(architecture/DataFlowArchitecture.png)*

This diagram represents the complete flow of data in the system, connecting four primary components: the detection module, ESP32, Google Cloud Pub/Sub, and VONE platform for data analysis.

**Detection Module (Webcam and YOLO)**

A webcam captures frames in near real-time via python and ultralytics. The YOLO model processes each frame to detect objects (e.g., paper or plastic). Detection results are published to the Pub/Sub topic in Google Cloud.

**Google Cloud Pub/Sub**

A Publisher sends detection results to a Pub/Sub topic. The ESP32 retrieves the latest data from the Pub/Sub topic via an HTTP POST request. Upon successful retrieval, the ESP32 acknowledges the message via another HTTP POST request.

**ESP32 Component**

The ESP32 receives data from Google Cloud, executes actions based on the detection results, and uses an MQTT client to forward this and the ultrasonic sensor data to VONE.

**VONE**

VONEs MQTT Broker receives data from the ESP32. The data is processed and analyzed within Platform VOne's data processing component. 

This flow ensures seamless integration between object detection, cloud messaging, action execution, and data analysis.

## Setup, Configuration and Installation

**VIEW README in Github project**

(For everyone that wants or needs a quick setup:)
### Quick Setup Guide

**Hardware**

- HC-SRO4 on pins 41 and 42
- LED on pin 7
- Servo on pin 4
- Connect ESP32 to laptop with working webcam

**Cloud**

If you have a Google Cloud Topic and Service Accounts set up follow the README under "Google PubSub Setup", for VONE follow the instructions in the README under "V-ONE Setup".

For an Internet connection change variables in "wastemanagement_sketch.ino" line 32 and 33.

If you only want to test the sensor and you don't want cloud, comment out every cloud related code inside "wastemanagement_sketch.ino" and upload it to the esp32. (The next step is not needed in this case)

**Python and Webcam**

First run "generate_authtoken.py" and copy-paste the result in your console into the "wastemanagement_sketch.ino" variable in line 25. It will only last for an hour, after this hour repeat this step.

Then run "predict_webcam.py" a UI of your camera should pop-up and the detection is running.

At this point upload "wastemanagement_sketch.ino" to the esp32 and dont forget to activate your wifi.

This is all you need to run both the sensor and the detection.

## Troubleshooting
Due to the nature of the authentication token creation the token will expire after 1 hour.

Labeling and collecting the data was more work than imagined and also wasnt good enough. The model is not working absolutely accurate but this would need way more time.

Unfortunately the system is not working at real-time as the detected object is first send to the cloud and then it gets pulled by the esp32. This is sub-optimal, we tried a lot of different ways however this seemed to be the one that works and the others didn't.

## Links

[GitHub](https://github.com/Kolumnist/Material-Detection)

[Video Presentation]()