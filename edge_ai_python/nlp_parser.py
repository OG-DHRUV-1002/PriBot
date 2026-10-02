import re
import json
import difflib

class NLPParser:
    def __init__(self):
        # Master intent dictionary mapping natural language to expected Firebase JSON contracts
        self.intent_map = {
            "dance": {
                "target": "/commands",
                "payload": {"oled_animation": "idle", "voice_trigger": "dance"}
            },
            "look around": {
                "target": "/commands",
                "payload": {"oled_animation": "loading", "voice_trigger": "look_around"}
            },
            "get angry": {
                "target": "/commands",
                "payload": {"oled_animation": "angry", "voice_trigger": "none"}
            },
            "stand up": {
                "target": "/control/stance",
                "payload": {"height": 80.0, "roll": 0.0, "pitch": 0.0}
            },
            "sit down": {
                "target": "/control/stance",
                "payload": {"height": 20.0, "roll": 0.0, "pitch": 0.0}
            },
            "stop": {
                "target": "/control/joystick",
                "payload": {"x": 0.0, "y": 0.0}
            }
        }
        
        self.known_commands = list(self.intent_map.keys())

    def parse_command(self, transcribed_text):
        """
        Parses transcribed text using regex boundary matching and fuzzy logic fallback.
        Returns a tuple: (database_path, json_payload_dict) or (None, None)
        """
        if not transcribed_text:
            return None, None
            
        transcribed_text = transcribed_text.lower().strip()
        
        # 1. Regex Exact Substring Matching
        for cmd in self.known_commands:
            # Check if command exists as a whole word construct in the text
            if re.search(r'\b' + re.escape(cmd) + r'\b', transcribed_text):
                print(f"[NLPParser] Exact regex match found for intent: '{cmd}'")
                return self.intent_map[cmd]["target"], self.intent_map[cmd]["payload"]

        # 2. Fuzzy Matching (Fallback for speech transcription errors)
        # Using a 60% similarity threshold
        matches = difflib.get_close_matches(transcribed_text, self.known_commands, n=1, cutoff=0.6)
        
        if matches:
            best_match = matches[0]
            print(f"[NLPParser] Fuzzy match resolved: '{transcribed_text}' -> '{best_match}'")
            return self.intent_map[best_match]["target"], self.intent_map[best_match]["payload"]
            
        print(f"[NLPParser] Intent unrecognized or ambiguous: '{transcribed_text}'")
        return None, None

if __name__ == "__main__":
    # Internal component test sequence
    parser = NLPParser()
    
    test_phrases = [
        "time to dance", 
        "look aroud",  # Typo testing fuzzy matching
        "can you stand up", 
        "get angry",
        "make me a sandwich" # Expected to fail
    ]
    
    for phrase in test_phrases:
        print(f"\nTesting Input: '{phrase}'")
        path, payload = parser.parse_command(phrase)
        if path:
            print(f"Target RTDB Path: {path}")
            print(f"JSON Payload: {json.dumps(payload)}")
