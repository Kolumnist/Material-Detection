import os
from ultralytics import YOLO
import cv2
import math

cap = cv2.VideoCapture(0)
cap.set(3, 640)
cap.set(4, 480)

# model
model_path = os.path.join('..', 'runs', 'detect', 'train3', 'weights', 'last.pt')
model = YOLO(model_path)

threshold = 0.5

classNames = ["paper", "plastic"]

while True:
    success, img = cap.read()
    results = model(img, stream=True)
    
    for result in results:
        boxes = result.boxes

        for box in boxes:
            
            confidence = math.ceil((box.conf[0]*100))/100
            if confidence < threshold: break
            #print("Confidence --->",confidence)

            # class name
            cls = int(box.cls[0])
            #print("Class name -->", classNames[cls].upper())

            # bounding box
            x1, y1, x2, y2 = box.xyxy[0]
            x1, y1, x2, y2 = int(x1), int(y1), int(x2), int(y2)

            cv2.rectangle(img, (x1, y1), (x2, y2), (255, 0, 255), 3)
            cv2.putText(img, classNames[cls].upper(), [x1, y1], cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)

    cv2.imshow('Webcam', img)
    if cv2.waitKey(1) == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()