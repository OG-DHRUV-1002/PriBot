import 'package:firebase_database/firebase_database.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

// Global Riverpod Provider for Dependency Injection
final firebaseRepoProvider = Provider<FirebaseRepository>((ref) {
  return FirebaseRepository();
});

class FirebaseRepository {
  final DatabaseReference _dbRef = FirebaseDatabase.instance.ref();

  // --- STREAMS (ASYNCHRONOUS READS) ---
  
  Stream<Map<String, dynamic>> streamTelemetry() {
    return _dbRef.child('telemetry').onValue.map((event) {
      if (event.snapshot.value != null) {
        return Map<String, dynamic>.from(event.snapshot.value as Map);
      }
      return {'rssi': -99, 'battery_voltage_adc': 0.0};
    });
  }

  // --- COMMANDS (SYNCHRONOUS WRITES) ---
  
  Future<void> pushJoystickVector(double x, double y) async {
    // Note: Floating point truncation to prevent large payload saturation.
    // X and Y are validated strictly by the server-side RTDB rules (-1.0 to 1.0).
    await _dbRef.child('control/joystick').update({
      'x': double.parse(x.toStringAsFixed(2)),
      'y': double.parse(y.toStringAsFixed(2)),
    });
  }
  
  Future<void> pushStance(double height, double roll, double pitch) async {
    await _dbRef.child('control/stance').update({
      'height': double.parse(height.toStringAsFixed(2)),
      'roll': double.parse(roll.toStringAsFixed(2)),
      'pitch': double.parse(pitch.toStringAsFixed(2)),
    });
  }

  Future<void> triggerOledAnimation(String animationId) async {
    // Valid Enums: idle, blinking, angry, loading, offline
    await _dbRef.child('commands').update({
      'oled_animation': animationId,
    });
  }
  
  Future<void> triggerVoiceCommand(String command) async {
    await _dbRef.child('commands').update({
      'voice_trigger': command,
    });
  }
}
