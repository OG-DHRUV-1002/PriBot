import cv2
import os
import urllib.request

class VisionNode:
    def __init__(self, camera_index=0):
        cascade_path = 'haarcascade_frontalface_default.xml'
        
        # Download the Haar Cascade if it doesn't exist locally
        if not os.path.exists(cascade_path):
            print("[EDGE AI] Downloading Haar Cascade XML...")
            url = "https://raw.githubusercontent.com/opencv/opencv/master/data/haarcascades/haarcascade_frontalface_default.xml"
            urllib.request.urlretrieve(url, cascade_path)
            
        # Initialize OpenCV Face Detector from local file
        self.face_cascade = cv2.CascadeClassifier(cascade_path)
        self.cap = cv2.VideoCapture(camera_index)
        
        # Optimize camera settings for latency
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        self.cap.set(cv2.CAP_PROP_FPS, 30)

    def process_frame(self):
        ret, frame = self.cap.read()
        if not ret:
            return None, (0.0, 0.0)

        # Flip horizontally for selfie-view
        frame = cv2.flip(frame, 1)
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
        # Detect faces (fast, low overhead)
        faces = self.face_cascade.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5, minSize=(100, 100))
        
        x_norm, y_norm = 0.0, 0.0
        
        if len(faces) > 0:
            # Get the largest face
            (x, y, w, h) = max(faces, key=lambda rect: rect[2] * rect[3])
            
            # Draw bounding box
            cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)
            cv2.putText(frame, "DRIVING: MOVE HEAD", (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 0, 0), 2)
            
            # Calculate face center
            center_x = x + w // 2
            center_y = y + h // 2
            cv2.circle(frame, (center_x, center_y), 5, (0, 0, 255), -1)
            
            # Map screen coordinates to -1.0 to +1.0 vectors
            # X: 0 (left) -> 640 (right) | Y: 0 (top) -> 480 (bottom)
            x_norm = (center_x / 320.0) - 1.0
            y_norm = -((center_y / 240.0) - 1.0) # Invert Y so up is positive
            
            # Deadzone at the center
            if abs(x_norm) < 0.15: x_norm = 0.0
            if abs(y_norm) < 0.15: y_norm = 0.0
            
            # Clamp vectors
            x_norm = max(-1.0, min(1.0, x_norm))
            y_norm = max(-1.0, min(1.0, y_norm))
        else:
            cv2.putText(frame, "NO FACE DETECTED - PARKING", (10, 70), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 255), 2)
            
        return frame, (x_norm, y_norm)

    def release(self):
        self.cap.release()
