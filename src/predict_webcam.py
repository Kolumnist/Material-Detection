import time
from ultralytics import YOLO
from google.cloud import pubsub_v1
import os
import cv2
import math

cap = cv2.VideoCapture(0)
cap.set(3, 640)
cap.set(4, 480)

# google
os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = "C:\dev\personal\Material Detection\iot-usm-446702-4befce424c11.json"
publisher = pubsub_v1.PublisherClient()
topic_path = "projects/iot-usm-446702/topics/detection"

# model
model_path = os.path.join('..', 'runs', 'last.pt')
model = YOLO(model_path)
confidence_threshold = 0.8

classNames = ["paper", "plastic"]

while True:
    success, img = cap.read()
    results = model(img, stream=True)
    
    for result in results:
        boxes = result.boxes

        for box in boxes:
        
            confidence = math.ceil((box.conf[0]*100))/100

            if confidence < confidence_threshold : break
            cls = int(box.cls[0])

            publisher.publish(topic_path, classNames[cls].encode("utf-8"), type=classNames[cls])

            # bounding box
            x1, y1, x2, y2 = box.xyxy[0]
            x1, y1, x2, y2 = int(x1), int(y1), int(x2), int(y2)

            cv2.rectangle(img, (x1, y1), (x2, y2), (255, 0, 255), 3)
            cv2.putText(img, classNames[cls].upper(), [160, 160], cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)
    time.sleep(1)        
    cv2.imshow('Webcam', img)
    if cv2.waitKey(1) == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()