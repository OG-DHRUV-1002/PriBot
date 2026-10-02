import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import os
from dotenv import load_dotenv
import time
import cv2
from gesture_tracker import GestureTracker

class FirebasePublisher:
    def __init__(self):
        load_dotenv()
        cred_path = os.getenv("FIREBASE_CREDENTIALS_PATH", "./serviceAccountKey.json")
        database_url = os.getenv("FIREBASE_DATABASE_URL")
        
        print("[FirebasePublisher] Initializing RTDB Admin connection...")
        cred = credentials.Certificate(cred_path)
        firebase_admin.initialize_app(cred, {
            'databaseURL': database_url
        })
        self.ref = db.reference('/')
        self.last_publish = 0
        self.publish_interval = 1.0 / 30.0 # Strict 30 Hz throttling

    def publish_vector(self, x, y, is_stop):
        current_time = time.time()
        if current_time - self.last_publish >= self.publish_interval:
            
            if is_stop:
                x, y = 0.0, 0.0
                
            payload = {
                "control": {
                    "joystick": {
                        "x": float(round(x, 2)),
                        "y": float(round(y, 2))
                    }
                }
            }
            # Use RTDB update() to patch only the specific node without overwriting sibling keys
            self.ref.update(payload)
            self.last_publish = current_time

if __name__ == "__main__":
    publisher = FirebasePublisher()
    tracker = GestureTracker()
    cap = cv2.VideoCapture(0)
    
    print("[Vision Pipeline] Starting 30Hz feed...")
    
    try:
        while cap.isOpened():
            success, frame = cap.read()
            if not success:
                continue
                
            # Flip frame horizontally for selfie view coherence
            frame = cv2.flip(frame, 1)
            
            # Process gesture inference
            annotated_frame, v_x, v_y, is_stop = tracker.process_frame(frame)
            
            # Push vectors to Firebase at 30Hz
            publisher.publish_vector(v_x, v_y, is_stop)
            
            # Local HUD display
            status_text = "STOP (FIST DETECTED)" if is_stop else f"DRIVING [X: {v_x:.2f} | Y: {v_y:.2f}]"
            color = (0, 0, 255) if is_stop else (0, 255, 0)
            cv2.putText(annotated_frame, status_text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.8, color, 2)
            
            # Center target reticle
            h, w, c = annotated_frame.shape
            cv2.circle(annotated_frame, (int(w/2), int(h/2)), int(tracker.deadzone_radius*w/2), (255, 255, 255), 1)
            
            cv2.imshow('Project Synth-OS: AI Vision Node', annotated_frame)
            
            if cv2.waitKey(1) & 0xFF == 27: # Esc key termination
                break
                
    finally:
        cap.release()
        cv2.destroyAllWindows()
