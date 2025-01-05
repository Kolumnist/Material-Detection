from ultralytics import YOLO

model = YOLO("yolov8n.pt")

results = model.train(data="yolov8_config.yaml", epochs=100)