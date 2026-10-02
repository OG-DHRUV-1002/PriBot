import cv2
import mediapipe as mp
import math

class GestureTracker:
    def __init__(self):
        self.mp_hands = mp.solutions.hands
        self.hands = self.mp_hands.Hands(
            static_image_mode=False,
            max_num_hands=1,
            min_detection_confidence=0.7,
            min_tracking_confidence=0.5
        )
        self.mp_draw = mp.solutions.drawing_utils
        
        # Deadzone configuration for neutral hover state
        self.center_x = 0.5
        self.center_y = 0.5
        self.deadzone_radius = 0.1

    def is_palm_open(self, hand_landmarks):
        """Calculates Euclidian distance between wrist (0) and middle finger tip (12)"""
        wrist = hand_landmarks.landmark[self.mp_hands.HandLandmark.WRIST]
        mid_tip = hand_landmarks.landmark[self.mp_hands.HandLandmark.MIDDLE_FINGER_TIP]
        
        dist = math.hypot(mid_tip.x - wrist.x, mid_tip.y - wrist.y)
        return dist > 0.35 # > 0.35 is open palm (GO), < 0.35 is closed fist (STOP)

    def process_frame(self, frame):
        """
        Processes a BGR image frame.
        Returns: (annotated_frame, vector_x, vector_y, is_stop)
        """
        # Convert BGR to RGB for MediaPipe inference
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        results = self.hands.process(rgb_frame)
        
        vec_x, vec_y = 0.0, 0.0
        is_stop = True # Failsafe: Default to stop if no hand detected

        if results.multi_hand_landmarks:
            for hand_landmarks in results.multi_hand_landmarks:
                self.mp_draw.draw_landmarks(frame, hand_landmarks, self.mp_hands.HAND_CONNECTIONS)
                
                # Use wrist point as the master centroid for tracking
                wrist = hand_landmarks.landmark[self.mp_hands.HandLandmark.WRIST]
                
                # Invert X because camera is physically mirrored for the user
                centroid_x = 1.0 - wrist.x
                centroid_y = wrist.y
                
                is_stop = not self.is_palm_open(hand_landmarks)
                
                if not is_stop:
                    # Map normalized coordinates to -1.0 to 1.0 physical driving vectors
                    # Note: Y is inverted (up is negative in image, positive in physical joystick)
                    raw_x = (centroid_x - self.center_x) * 2.0
                    raw_y = (self.center_y - centroid_y) * 2.0
                    
                    # Apply circular deadzone logic
                    if math.hypot(raw_x, raw_y) > self.deadzone_radius:
                        vec_x = max(-1.0, min(1.0, raw_x))
                        vec_y = max(-1.0, min(1.0, raw_y))
                    
                    # Draw steering vector telemetry on frame
                    h, w, c = frame.shape
                    cx, cy = int(wrist.x * w), int(wrist.y * h)
                    end_x = int(cx - (vec_x * 100))
                    end_y = int(cy - (vec_y * 100))
                    cv2.arrowedLine(frame, (cx, cy), (end_x, end_y), (0, 255, 0), 3)

        return frame, vec_x, vec_y, is_stop
