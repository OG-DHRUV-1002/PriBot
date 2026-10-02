import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'firebase_repository.dart';

class DashboardScreen extends ConsumerWidget {
  const DashboardScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final repo = ref.watch(firebaseRepoProvider);
    
    return Scaffold(
      backgroundColor: const Color(0xFF121212),
      appBar: AppBar(
        title: const Text('PROJECT SYNTH-OS COMMAND CENTER', style: TextStyle(letterSpacing: 2.0)),
        backgroundColor: Colors.black,
        elevation: 0,
      ),
      body: Row(
        children: [
          // Left Panel: Controls (Reserved for Phase 12/13)
          Expanded(
            flex: 1,
            child: Container(
              margin: const EdgeInsets.all(8),
              decoration: BoxDecoration(
                color: const Color(0xFF1E1E1E),
                border: Border.all(color: Colors.blueAccent.withOpacity(0.5)),
                borderRadius: BorderRadius.circular(12),
              ),
              child: const Center(
                child: Text('CONTROLS\n(WASD / Touch Joystick)', 
                  textAlign: TextAlign.center,
                  style: TextStyle(color: Colors.white30, letterSpacing: 1.5)),
              ),
            ),
          ),
          
          // Center Panel: Live Telemetry Stream
          Expanded(
            flex: 2,
            child: Container(
              margin: const EdgeInsets.all(8),
              decoration: BoxDecoration(
                color: const Color(0xFF1A1A1A),
                border: Border.all(color: Colors.greenAccent.withOpacity(0.3)),
                borderRadius: BorderRadius.circular(12),
                boxShadow: [
                  BoxShadow(color: Colors.greenAccent.withOpacity(0.05), blurRadius: 20)
                ]
              ),
              child: StreamBuilder<Map<String, dynamic>>(
                stream: repo.streamTelemetry(),
                builder: (context, snapshot) {
                  if (snapshot.connectionState == ConnectionState.waiting) {
                    return const Center(child: CircularProgressIndicator(color: Colors.greenAccent));
                  }
                  
                  final data = snapshot.data ?? {};
                  final int rssi = data['rssi'] as int? ?? -99;
                  final double battery = (data['battery_voltage_adc'] as num?)?.toDouble() ?? 0.0;
                  
                  final bool isOnline = rssi > -85;
                  final String status = isOnline ? 'ONLINE' : 'LINK LOST';
                  final Color statusColor = isOnline ? Colors.greenAccent : Colors.redAccent;
                  
                  return Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      const Icon(Icons.memory, color: Colors.greenAccent, size: 48),
                      const SizedBox(height: 10),
                      const Text('ESP32-S3 TELEMETRY', 
                        style: TextStyle(color: Colors.white, fontSize: 24, fontWeight: FontWeight.w900, letterSpacing: 2.0)),
                      const SizedBox(height: 40),
                      _buildTelemetryRow('SYSTEM STATUS', status, statusColor),
                      _buildTelemetryRow('WIFI RSSI (dBm)', '$rssi', isOnline ? Colors.white : Colors.redAccent),
                      _buildTelemetryRow('LOGIC BATTERY (V)', '${battery.toStringAsFixed(2)}', battery > 3.3 ? Colors.white : Colors.redAccent),
                    ],
                  );
                },
              ),
            ),
          ),
          
          // Right Panel: OLED Animation Override Grid
          Expanded(
            flex: 1,
            child: Container(
              margin: const EdgeInsets.all(8),
              padding: const EdgeInsets.all(16),
              decoration: BoxDecoration(
                color: const Color(0xFF1E1E1E),
                border: Border.all(color: Colors.orangeAccent.withOpacity(0.5)),
                borderRadius: BorderRadius.circular(12),
              ),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  const Icon(Icons.remove_red_eye, color: Colors.orangeAccent, size: 32),
                  const SizedBox(height: 10),
                  const Text('OLED OVERRIDE', 
                    textAlign: TextAlign.center,
                    style: TextStyle(color: Colors.orangeAccent, fontSize: 16, fontWeight: FontWeight.bold, letterSpacing: 1.5)),
                  const SizedBox(height: 30),
                  Expanded(
                    child: GridView.count(
                      crossAxisCount: 2,
                      crossAxisSpacing: 12,
                      mainAxisSpacing: 12,
                      children: [
                        _buildOledButton('IDLE', 'idle', repo, Icons.sentiment_neutral),
                        _buildOledButton('BLINK', 'blinking', repo, Icons.visibility),
                        _buildOledButton('ANGRY', 'angry', repo, Icons.warning_amber_rounded),
                        _buildOledButton('LOAD', 'loading', repo, Icons.hourglass_bottom),
                      ],
                    ),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildTelemetryRow(String label, String value, Color valueColor) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 12.0, horizontal: 60.0),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label, style: const TextStyle(color: Colors.white54, fontSize: 16, fontWeight: FontWeight.w500, letterSpacing: 1.0)),
          Text(value, style: TextStyle(color: valueColor, fontSize: 18, fontWeight: FontWeight.bold, fontFamily: 'monospace')),
        ],
      ),
    );
  }

  Widget _buildOledButton(String label, String animId, FirebaseRepository repo, IconData icon) {
    return ElevatedButton(
      style: ElevatedButton.styleFrom(
        backgroundColor: const Color(0xFF2A2A2A),
        foregroundColor: Colors.orangeAccent,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
        elevation: 4,
      ),
      onPressed: () => repo.triggerOledAnimation(animId),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Icon(icon, size: 28),
          const SizedBox(height: 8),
          Text(label, style: const TextStyle(fontWeight: FontWeight.bold, fontSize: 12, letterSpacing: 1.0)),
        ],
      ),
    );
  }
}
