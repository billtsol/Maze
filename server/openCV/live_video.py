# Live_video.py
# Ο κώδικας αυτός επιτρέπει τη λήψη και την εμφάνιση εικόνας από ένα IP stream της ESP32-CAM
# χρησιμοποιώντας OpenCV και NumPy. Αρχικά, η κάμερα συνδέεται μέσω ενός URL που παραπέμπει
# στη ροή εικόνας της κάμερας. Σε κάθε επανάληψη, το πρόγραμμα κάνει αίτηση για λήψη εικόνας
# μέσω του URL και μετατρέπει τα δεδομένα εικόνας σε μορφή που μπορεί να αναγνωρίσει το OpenCV.
# Η εικόνα εμφανίζεται σε ένα παράθυρο με τίτλο "live Cam" 

import cv2
import numpy as np
import urllib.request

# URL της ροής κάμερας ESP32 (υψηλή ανάλυση)
url = 'http://192.168.2.4/cam-hi.jpg'

# Καταγραφή της ροής βίντεο από το URL
cap = cv2.VideoCapture(url)

# Ορισμός διαστάσεων για το μέγεθος εισόδου στο δίκτυο
whT = 320

# Όρια εμπιστοσύνης και καταστολής μη μέγιστων
confThreshold = 0.5  # Όριο εμπιστοσύνης
nmsThreshold = 0.3   # Όριο καταστολής μη μέγιστων

# Φόρτωση ονομάτων κλάσεων COCO
classesfile = "coco.names"
classNames = []

# Ανάγνωση των ονομάτων κλάσεων από το αρχείο
with open(classesfile, 'rt') as f:
    classNames = f.read().rstrip('\n').split('\n')

# Φόρτωση του μοντέλου YOLOv3
modelConfig = "yolov3.cfg"
modelWeights = "yolov3.weights"
net = cv2.dnn.readNetFromDarknet(modelConfig, modelWeights)

# Ορισμός προτιμώμενου backend και στόχου (CPU σε αυτή την περίπτωση)
net.setPreferableBackend(cv2.dnn.DNN_BACKEND_OPENCV)
net.setPreferableTarget(cv2.dnn.DNN_TARGET_CPU)

# Συνάρτηση εύρεσης αντικειμένων στο καρέ
def findObject(outputs, im):
    hT, wT, cT = im.shape  # Λήψη διαστάσεων εικόνας
    bbox = []              # Πλαίσια ορίων
    classIds = []          # Ταυτότητες κλάσεων
    confs = []             # Τιμές εμπιστοσύνης

    # Διάσχιση κάθε εξόδου από το δίκτυο
    for output in outputs:
        for det in output:
            scores = det[5:]  # Σκορ εμπιστοσύνης για κάθε κλάση
            classId = np.argmax(scores)  # Λήψη κλάσης με το υψηλότερο σκορ
            confidence = scores[classId] # Λήψη εμπιστοσύνης για αυτήν την κλάση
            if confidence > confThreshold:  # Φιλτράρισμα χαμηλής εμπιστοσύνης
                w, h = int(det[2] * wT), int(det[3] * hT)  # Υπολογισμός πλάτους και ύψους πλαισίου
                x, y = int((det[0] * wT) - w / 2), int((det[1] * hT) - h / 2)  # Υπολογισμός πάνω αριστερής γωνίας
                bbox.append([x, y, w, h])  # Προσθήκη συντεταγμένων πλαισίου στη λίστα
                classIds.append(classId)  # Προσθήκη ταυτότητας κλάσης στη λίστα
                confs.append(float(confidence))  # Προσθήκη εμπιστοσύνης στη λίστα

    # Εκτέλεση καταστολής μη μέγιστων για εξάλειψη αλληλοκαλυπτόμενων πλαισίων
    indices = cv2.dnn.NMSBoxes(bbox, confs, confThreshold, nmsThreshold)

    # Σχεδίαση πλαισίων και ετικετών για τα ανιχνευμένα αντικείμενα
    for i in indices:
        i = i
        box = bbox[i]  # Λήψη του πλαισίου
        x, y, w, h = box[0], box[1], box[2], box[3]  # Λήψη συντεταγμένων και διαστάσεων πλαισίου

        # Σχεδίαση ορθογωνίου γύρω από το αντικείμενο
        cv2.rectangle(im, (x, y), (x + w, y + h), (255, 0, 255), 2)
        # Εμφάνιση ονόματος κλάσης και ποσοστού εμπιστοσύνης στην εικόνα
        cv2.putText(im, f'{classNames[classIds[i]].upper()} {int(confs[i] * 100)}%',
                    (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 0, 255), 2)

# Κύριος βρόχος για ανάγνωση εικόνων και εκτέλεση ανίχνευσης αντικειμένων
while True:
    # Λήψη εικόνας από το URL και αποκωδικοποίηση
    img_resp = urllib.request.urlopen(url)
    imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
    im = cv2.imdecode(imgnp, -1)

    # Προεπεξεργασία της εικόνας για το μοντέλο YOLO
    blob = cv2.dnn.blobFromImage(im, 1/255, (whT, whT), [0, 0, 0], 1, crop=False)
    net.setInput(blob)

    # Λήψη των ονομάτων των εξόδων του δικτύου
    layernames = net.getLayerNames()
    outputNames = [layernames[i - 1] for i in net.getUnconnectedOutLayers()]

    # Εκτέλεση της προώθησης για λήψη εξόδων
    outputs = net.forward(outputNames)

    # Εύρεση αντικειμένων στις εξόδους
    findObject(outputs, im)

    # Εμφάνιση εικόνας με τα ανιχνευμένα αντικείμενα
    cv2.imshow('Image', im)
    cv2.waitKey(1)  # Αναμονή 1 ms για εμφάνιση επόμενου καρέ
