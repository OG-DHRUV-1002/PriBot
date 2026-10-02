import time
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
from vision_node import VisionNode
# from audio_node import AudioNode
import cv2

SERVICE_ACCOUNT_KEY_PATH = 'firebase_service_account.json'
DATABASE_URL = 'https://pribot-c759e-default-rtdb.asia-southeast1.firebasedatabase.app'

def main():
    print("[EDGE AI] Initializing Firebase Admin SDK...")
    try:
        cred = credentials.Certificate(SERVICE_ACCOUNT_KEY_PATH)
        firebase_admin.initialize_app(cred, {
            'databaseURL': DATABASE_URL
        })
    except Exception as e:
        print(f"[ERROR] Failed to load Firebase Service Account: {e}")
        print("Please ensure 'firebase_service_account.json' is configured securely.")
        return
    
    wasd_ref = db.reference('/control/wasd')
    emotes_ref = db.reference('/commands/emotes')

    print("[EDGE AI] Booting Vision Node... (Audio Disabled)")
    vision = VisionNode()
    # audio = AudioNode()
    
    # audio.start_listening()
    print("[EDGE AI] Nodes Active. Listening for 'Pribot' wake word.")
    print("[EDGE AI] Show Open Palm to STOP, Point Index Finger to MOVE.")
    print("[EDGE AI] Press 'q' to quit.")

    last_x, last_y = 0.0, 0.0
    has_published_zero = True

    try:
        while True:
            # 1. Vision Node: Hardware accelerated MediaPipe tracking
            frame, (x, y) = vision.process_frame()
            
            if frame is not None:
                # Telemetry overlay
                cv2.putText(frame, f"Vector: ({x:.2f}, {y:.2f})", (10, 30), 
                            cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                cv2.imshow("Edge AI - Vision Control Node", frame)
            
            # 2. Optimized Firebase Publisher (Only transmit state changes)
            is_zero = (x == 0.0 and y == 0.0)
            if x != last_x or y != last_y or (is_zero and not has_published_zero):
                last_x = x
                last_y = y
                has_published_zero = is_zero
                
                wasd_ref.update({
                    'x': x,
                    'y': y,
                    'timestamp': int(time.time() * 1000)
                })

            # 3. Audio Node: Asynchronous Speech Intent Queue
            # emote_cmd = audio.get_latest_command()
            # if emote_cmd is not None:
            #     print(f"[EDGE AI] Voice Intent Confirmed. Pushing Emote Flag {emote_cmd} to Firebase.")
            #     emotes_ref.update({
            #         'active_emote_flag': emote_cmd,
            #         'trigger_timestamp': int(time.time() * 1000)
            #     })

            # 4. Render & Exit loop
            if cv2.waitKey(20) & 0xFF == ord('q'):
                break

    except KeyboardInterrupt:
        print("[EDGE AI] Keyboard interrupt received.")
    finally:
        print("[EDGE AI] Shutting down resources gracefully...")
        # audio.stop_listening()
        vision.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
