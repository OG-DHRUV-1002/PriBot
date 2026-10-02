import speech_recognition as sr
import time

class AudioListener:
    def __init__(self, wake_word="synth", energy_threshold=300):
        self.recognizer = sr.Recognizer()
        self.recognizer.energy_threshold = energy_threshold
        # Enable dynamic thresholding for environmental noise gating
        self.recognizer.dynamic_energy_threshold = True
        self.wake_word = wake_word.lower()
        self.mic = sr.Microphone()
        
        # Noise gating calibration
        print("[AudioListener] Calibrating noise gate. Please remain quiet...")
        with self.mic as source:
            self.recognizer.adjust_for_ambient_noise(source, duration=2.0)
        print(f"[AudioListener] Calibration complete. Energy threshold set to: {self.recognizer.energy_threshold}")

    def listen_for_command(self):
        """Listens for the wake word, then captures the subsequent command."""
        with self.mic as source:
            print(f"[AudioListener] Listening for wake word '{self.wake_word}'...")
            try:
                # Capture audio snippet
                audio = self.recognizer.listen(source, timeout=1, phrase_time_limit=3)
                transcript = self.recognizer.recognize_google(audio).lower()
                
                if self.wake_word in transcript:
                    print(f"[AudioListener] Wake word detected in: '{transcript}'")
                    # If command was spoken with wake word (e.g. "Synth dance")
                    command = transcript.replace(self.wake_word, "").strip()
                    
                    if not command:
                        print("[AudioListener] Awaiting command intent...")
                        # Listen for the actual command if only wake word was said
                        audio_cmd = self.recognizer.listen(source, timeout=3, phrase_time_limit=5)
                        command = self.recognizer.recognize_google(audio_cmd).lower()
                        
                    print(f"[AudioListener] Captured Command: '{command}'")
                    return command
            except sr.WaitTimeoutError:
                pass # Timeout during listening, loop continues
            except sr.UnknownValueError:
                pass # Speech unintelligible, filter out noise
            except sr.RequestError as e:
                print(f"[AudioListener] Speech service API error: {e}")
                
        return None

if __name__ == "__main__":
    listener = AudioListener(wake_word="synth")
    while True:
        cmd = listener.listen_for_command()
        if cmd:
            print(f"--> Handing off to NLP parser: {cmd}")
        time.sleep(0.1)
