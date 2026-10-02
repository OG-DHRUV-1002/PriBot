import speech_recognition as sr
import re
import queue
import threading

class AudioNode:
    def __init__(self):
        self.recognizer = sr.Recognizer()
        self.mic = sr.Microphone()
        self.command_queue = queue.Queue()
        self.is_running = False
        self.thread = None

    def start_listening(self):
        print("[AUDIO] Calibrating microphone for ambient noise...")
        with self.mic as source:
            self.recognizer.adjust_for_ambient_noise(source, duration=1.5)
        print("[AUDIO] Calibration complete. Background thread listening...")
        
        self.is_running = True
        self.thread = threading.Thread(target=self._listen_loop, daemon=True)
        self.thread.start()

    def stop_listening(self):
        self.is_running = False
        if self.thread:
            self.thread.join()

    def _listen_loop(self):
        while self.is_running:
            with self.mic as source:
                try:
                    # Non-blocking timeout allows graceful thread exit
                    audio = self.recognizer.listen(source, timeout=1.0, phrase_time_limit=4.0)
                    text = self.recognizer.recognize_google(audio).lower()
                    
                    if "pribot" in text:
                        print(f"[AUDIO] Wake word detected. Transcript: '{text}'")
                        # Intent extraction using regex mapped to Emote Flags
                        if re.search(r'\b(happy|smile|joy)\b', text):
                            self.command_queue.put(1) # Happy
                        elif re.search(r'\b(angry|mad|attack)\b', text):
                            self.command_queue.put(2) # Angry
                        elif re.search(r'\b(scan|radar|search)\b', text):
                            self.command_queue.put(3) # Scan
                        elif re.search(r'\b(sleep|rest|shutdown)\b', text):
                            self.command_queue.put(4) # Sleep
                except sr.WaitTimeoutError:
                    pass # Timeout expected during silence
                except sr.UnknownValueError:
                    pass # Audio not understood, ignoring
                except sr.RequestError as e:
                    print(f"[AUDIO] API Error: {e}")

    def get_latest_command(self):
        try:
            return self.command_queue.get_nowait()
        except queue.Empty:
            return None
