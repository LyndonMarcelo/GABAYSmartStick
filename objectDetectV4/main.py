import cv2
import numpy as np
import requests
import urllib.request
import time

#cap = cv2.VideoCapture(0)
net = cv2.dnn.readNetFromONNX("gabayVisionV5.onnx")
classes = ['crosswalk', 'person', 'post', 'pothole', 'stairs']

phpip = "###.###.###.###"
camip = "###.###.###.###"

URL = 'http://'+camip+'/cam-hi.jpg'

while True:
    #img = cap.read()[1]

    with urllib.request.urlopen(URL) as url:
        with open('temp.jpg', 'wb') as f:
            f.write(url.read())

    img = cv2.imread('temp.jpg')

    if img is None:
        break
    img = cv2.resize(img, (416, 416))
    blob = cv2.dnn.blobFromImage(img, scalefactor=1/255, size=(416, 416), mean=[0, 0, 0], swapRB=True, crop=False)
    net.setInput(blob)
    detections = net.forward()[0]
    #print(detections.shape)

    #cx, cy, w, h, confidence, class_scores
    #class-id, confidence, boxes

    classes_ids = []
    confidences = []
    boxes = []
    rows = detections.shape[0]

    img_width, img_height = img.shape[1], img.shape[0]
    x_scale = img_width/416
    y_scale = img_height/416

    for i in range(rows):
        row = detections[i]
        confidence = row[4]
        if confidence > 0.70:
            classes_score = row[5:]
            ind = np.argmax(classes_score)
            if classes_score[ind] > 0.5:
                classes_ids.append(ind)
                confidences.append(confidence)
                cx, cy, w, h = row[:4]
                x1 = int((cx-w/2)*x_scale)
                y1 = int((cy-h/2)*y_scale)
                width = int(w * x_scale)
                height = int(h * y_scale)
                box = np.array([x1, y1, width, height])
                boxes.append(box)
                #print(confidences)

    indices = cv2.dnn.NMSBoxes(boxes, confidences, 0.5, 0.5)

    for i in indices:
        x1, y1, w, h = boxes[i]
        label = classes[classes_ids[i]]
        conf = confidences[i]
        text = label + "{:.2f}".format(conf)
        cv2.rectangle(img,(x1, y1),(x1+w,y1+h),(255,0,0),2)
        cv2.putText(img, text, (x1,y1-2), cv2.FONT_HERSHEY_COMPLEX, 0.7, (255,0,255), 2)
        print(classes_ids[i])

        if classes_ids[i] == 0:
            print("crosswalk")
            requests.get("http://"+phpip+"/gabay/gabayCrosswalkTrue.php")

        if classes_ids[i] == 1:
            print("person")
            requests.get("http://"+phpip+"/gabay/gabayPersonTrue.php")

        if classes_ids[i] == 2:
            print("post")
            requests.get("http://"+phpip+"/gabay/gabayPostTrue.php")

        if classes_ids[i] == 3:
            print("pothole")
            requests.get("http://"+phpip+"/gabay/gabayPotholeTrue.php")

        if classes_ids[i] == 4:
            print("stairs")
            requests.get("http://"+phpip+"/gabay/gabayStairsTrue.php")



    cv2.imshow("Cam", img)
    k = cv2.waitKey(1)
    if k == ord('q'):
        break
