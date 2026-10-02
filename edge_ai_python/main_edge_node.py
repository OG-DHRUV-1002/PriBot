import time
import cv2
import firebase_admin
from firebase_admin import credentials, db
from vision_node import VisionNode
from audio_node import AudioNode

class EdgeAINode:
    def __init__(self):
        print("[EDGE AI] Initializing Synth-OS Local Computation Node...")
        
        # Initialize Firebase Admin SDK for secure, server-level database access
        try:
            # Place your generated service account JSON in this directory
            cred = credentials.Certificate("firebase_service_account.json")
            firebase_admin.initialize_app(cred, {
                'databaseURL': 'https://synthos-default-rtdb.firebaseio.com/'
            })
            print("[EDGE AI] Securely Connected to Firebase RTDB.")
        except FileNotFoundError:
            print("[ERROR] 'firebase_service_account.json' not found. Network publishing will fail.")
        except Exception as e:
            print(f"[ERROR] Firebase Auth Failed: {e}")
            
        # Get references to the exact schema paths defined in Phase 2
        try:
            self.db_wasd = db.reference('control/wasd')
            self.db_emotes = db.reference('commands/emotes')
        except ValueError:
            self.db_wasd = None
            self.db_emotes = None
        
        # Initialize the hardware sub-nodes
        self.audio = AudioNode()
        self.vision = VisionNode()
        
        # State tracking to prevent spamming the RTDB bandwidth
        self.last_pushed_emote = 1
        
    def start(self):
        # 1. Start Audio Daemon Thread
        self.audio.start_background_listening()
        
        # 2. Transfer control to the main Vision Thread (OpenCV must run on MainThread)
        print("[EDGE AI] Boot sequence complete. Engaging Neural Pipelines.")
        self._run_main_loop()

    def _run_main_loop(self):
        try:
            while True:
                ret, frame = self.vision.cap.read()
                if not ret:
                    time.sleep(0.01)
                    continue
                
                frame = cv2.flip(frame, 1)
                rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                
                # Execute MediaPipe Inference
                results = self.vision.hands.process(rgb_frame)
                
                if results.multi_hand_landmarks:
                    for hand_landmarks in results.multi_hand_landmarks:
                        self.vision.mp_draw.draw_landmarks(frame, hand_landmarks, self.vision.mp_hands.HAND_CONNECTIONS)
                        self.vision.analyze_gesture(hand_landmarks)
                        
                        # Task 15.1: Publish Vision Vectors to Firebase RTDB
                        # We only publish if we are actively gesturing, effectively overriding Web WASD inputs
                        if self.vision.current_gesture in ["MOVE (POINTING)", "STOP (OPEN PALM)"] and self.db_wasd:
                            try:
                                self.db_wasd.set({
                                    'x': round(self.vision.control_x, 3),
                                    'y': round(self.vision.control_y, 3),
                                    'timestamp': int(time.time() * 1000)
                                })
                            except Exception as e:
                                print(f"[EDGE AI] RTDB Vector Push Error: {e}")
                else:
                    self.vision.current_gesture = "NO HAND DETECTED"
                    self.vision.control_x = 0.0
                    self.vision.control_y = 0.0

                # Task 15.1: Publish Audio Emote Intents to Firebase RTDB
                current_audio_intent = self.audio.current_emote_intent
                if current_audio_intent != self.last_pushed_emote and self.db_emotes:
                    try:
                        self.db_emotes.set({
                            'current_emote': current_audio_intent,
                            'timestamp': int(time.time() * 1000)
                        })
                        self.last_pushed_emote = current_audio_intent
                        print(f"[EDGE AI] ☁️ Pushed Emote ID {current_audio_intent} to Firebase")
                    except Exception as e:
                        print(f"[EDGE AI] RTDB Emote Push Error: {e}")
                
                # Render Unified Edge AI HUD
                cv2.putText(frame, f"VISION: {self.vision.current_gesture}", (20, 40), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 255), 2)
                cv2.putText(frame, f"V-VECTOR: X={self.vision.control_x:.2f} Y={self.vision.control_y:.2f}", (20, 80), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
                
                # Resolve Audio intent name for HUD
                audio_intent_name = [k for k, v in self.audio.intent_map.items() if v == current_audio_intent]
                a_name = audio_intent_name[0].upper() if audio_intent_name else "IDLE"
                cv2.putText(frame, f"AUDIO: {a_name} (ID: {current_audio_intent})", (20, 120), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 100, 100), 2)
                
                cv2.imshow('Synth-OS : Neural Master Node', frame)
                
                # Maintain 30FPS loop rate
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
                    
        finally:
            self.vision.shutdown()
            self.audio.stop()
            print("[EDGE AI] System offline.")

if __name__ == "__main__":
    edge_node = EdgeAINode()
    edge_node.start()
