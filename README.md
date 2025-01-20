## Technologies
- Conda
- python
- ultralytics
- yolov8
- google-cloud-pubsub
- VOne
- Arduino
- ESP32

## Setup
Setup from start to finish, refer to Quick Start if you only want the functionality as it is.

*To train AI and make use of the detection library*
### Install conda and dependencies

1. Create conda environment: 

    `conda create --name *insert_name* python=3.10`

2. Activate created environment:

    `conda activate *insert_name*`

3. Install ultralytics for yolov8 training and detection:

    `pip install ultralytics`
    
    `pip install opencv-python`

### Collect Data and Train Model

Collected over 2000 pictures of paper and plastic from the internet

For labeling the data we used this: https://app.cvat.ai/

Tried out to [install LabelStudio locally conda](https://github.com/HumanSignal/label-studio?tab=readme-ov-file#install-locally-with-anaconda)
*didn't quite work out...*

---

Create data directory inside that an images and a labels directory. Put all images and labels inside them.

1. Make the **yolov8_config.yaml** match the label classes and data.

2. Train yolov8 model (adjust epochs if needed):
    
    **Console**
    
    yolo detect train data=yolov8_config.yaml model="yolov8n.pt" epochs=1
    
    **Python**
    
    `python src/train.py`

3. Predict via use of Webcam(adjust "model_path" as needed):

    `python src/predict_webcam.py`

    **For this to work without Google PubSub Set up, delete/comment out all pubsub related code. Also comment out line 46 this is used to not send too much data...**

### Google PubSub Setup

Send data to the google cloud. Can be used for all kinds of uses, we use it to send to a pubsub topic which a subscriber subscribes. Finally the esp32 gets that data from the subscriber via http. Where the subscribed message also gets acknowledged.

We tried using mqtt and websockets however pubsub doesnt like mqtt unless you use a bridge.

---

1. Install googlepubsub and auth to send data:

    `pip install google-cloud-pubsub`

    `google-auth`

2. Get the service account json file from both the publisher and the subscriber which you can find in the google console.

3. Change the paths in the predict_webcam.py:
- topic_path 
- os.environ["GOOGLE_APPLICATION_CREDENTIALS"]

4. Change the path in the generate_authtoken.py:
- SERVICE_ACCOUNT_FILE


### Arduino and Hardware Setup
*See Design.jpg for image of hardware.*

Hardware used:
- ESP32
- Ultrasonic Sensor HCSR04
- Micro Servo SG90 (ESP32Servo)

Under Google Cloud Pub/Sub variables, fill out the **accessToken** with the token that is generated when running **generate_authtoken.py**. The token will stay valid for 1 hour.

Under **Network** fill out the ssid and password.

#### V-ONE Setup
Refer to documentation of VOne for details.

Create device for Servo and Sensor. Send their data to VOne, create graphs to analyze data.

Add VONE Library to sketch and change the IDs of the respective servo and sensor variable.

### If all other prerequisites are done

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
