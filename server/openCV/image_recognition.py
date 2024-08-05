import cv2
import numpy as np
import urllib.request

# URL of the ESP32 camera stream (high resolution)
url = 'http://192.168.2.4/cam-hi.jpg'

# Capture video stream from the provided URL
cap = cv2.VideoCapture(url)

# Define width/height for network input size
whT = 320

# Confidence and non-max suppression thresholds
confThreshold = 0.5  # Confidence threshold
nmsThreshold = 0.3   # Non-Max Suppression threshold

# Load COCO class names
classesfile = "coco.names"
classNames = []

# Read the class names from the file
with open(classesfile, 'rt') as f:
    classNames = f.read().rstrip('\n').split('\n')

# Load YOLOv3 model configuration and weights
modelConfig = "yolov3.cfg"
modelWeights = "yolov3.weights"
net = cv2.dnn.readNetFromDarknet(modelConfig, modelWeights)

# Set preferable backend and target (CPU in this case)
net.setPreferableBackend(cv2.dnn.DNN_BACKEND_OPENCV)
net.setPreferableTarget(cv2.dnn.DNN_TARGET_CPU)

# Function to find objects in the frame
def findObject(outputs, im):
    hT, wT, cT = im.shape  # Get the dimensions of the image
    bbox = []              # Bounding boxes
    classIds = []          # Class IDs
    confs = []             # Confidence values

    # Iterate through each output from the network
    for output in outputs:
        for det in output:
            scores = det[5:]  # Confidence scores for each class
            classId = np.argmax(scores)  # Get the class with the highest score
            confidence = scores[classId] # Get the confidence of that class
            if confidence > confThreshold:  # Filter out low confidence detections
                w, h = int(det[2] * wT), int(det[3] * hT)  # Calculate box width and height
                x, y = int((det[0] * wT) - w / 2), int((det[1] * hT) - h / 2)  # Calculate top-left corner
                bbox.append([x, y, w, h])  # Add bounding box coordinates to list
                classIds.append(classId)  # Add class ID to list
                confs.append(float(confidence))  # Add confidence score to list

    # Perform non-max suppression to eliminate redundant overlapping boxes
    indices = cv2.dnn.NMSBoxes(bbox, confs, confThreshold, nmsThreshold)

    # Draw bounding boxes and labels for detected objects
    for i in indices:
        i = i
        box = bbox[i]  # Get the bounding box
        x, y, w, h = box[0], box[1], box[2], box[3]  # Get coordinates and dimensions of the box

        # Draw rectangle around the object
        cv2.rectangle(im, (x, y), (x + w, y + h), (255, 0, 255), 2)
        # Put class name and confidence percentage on the image
        cv2.putText(im, f'{classNames[classIds[i]].upper()} {int(confs[i] * 100)}%',
                    (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 0, 255), 2)

# Main loop to read images and perform object detection
while True:
    # Fetch the image from the URL and decode it
    img_resp = urllib.request.urlopen(url)
    imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
    im = cv2.imdecode(imgnp, -1)

    # Capture frame (not used in this context)
    success, img = cap.read()

    # Preprocess the image for the YOLO model
    blob = cv2.dnn.blobFromImage(im, 1/255, (whT, whT), [0, 0, 0], 1, crop=False)
    net.setInput(blob)

    # Get the names of the output layers
    layernames = net.getLayerNames()
    outputNames = [layernames[i - 1] for i in net.getUnconnectedOutLayers()]

    # Perform the forward pass to get outputs
    outputs = net.forward(outputNames)

    # Find objects in the outputs
    findObject(outputs, im)

    # Display the image with detected objects
    cv2.imshow('Image', im)
    cv2.waitKey(1)  # Wait for 1 ms to show the next frame
