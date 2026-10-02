import 'package:flutter/material.dart';
import 'package:firebase_database/firebase_database.dart';
import 'wasd_controller.dart';
import 'auth_service.dart';

class DashboardScreen extends StatelessWidget {
  const DashboardScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF0F172A), // Deep dark mode background
      appBar: AppBar(
        backgroundColor: const Color(0xFF1E293B),
        title: const Text('SYNTH-OS : COMMAND CENTER', style: TextStyle(letterSpacing: 2, fontWeight: FontWeight.w900)),
        actions: [
          IconButton(
            icon: const Icon(Icons.power_settings_new, color: Colors.redAccent),
            tooltip: 'Terminate Session',
            onPressed: () async {
              await AuthService().signOut();
              // In production, routing is bound to the Auth Stream. 
              // We simulate the pop here for structural completeness.
              Navigator.of(context).pop();
            },
          )
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(32.0),
        child: Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Left Column: Telemetry & Locomotion Control
            Expanded(
              flex: 4,
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: const [
                  TelemetryPanel(),
                  SizedBox(height: 40),
                  Expanded(child: WASDController()),
                ],
              ),
            ),
            const SizedBox(width: 40),
            // Right Column: OLED Emote Matrix
            Expanded(
              flex: 6,
              child: const EmoteGrid(),
            ),
          ],
        ),
      ),
    );
  }
}

class TelemetryPanel extends StatelessWidget {
  const TelemetryPanel({super.key});

  @override
  Widget build(BuildContext context) {
    final dbRef = FirebaseDatabase.instance.ref('telemetry');
    
    return StreamBuilder<DatabaseEvent>(
      stream: dbRef.onValue,
      builder: (context, snapshot) {
        double rssi = -100;
        double battery = 0.0;
        String status = 'OFFLINE';

        if (snapshot.hasData && snapshot.data!.snapshot.value != null) {
          try {
            final data = Map<String, dynamic>.from(snapshot.data!.snapshot.value as Map);
            rssi = (data['rssi'] ?? -100).toDouble();
            battery = (data['battery_v'] ?? 0.0).toDouble();
            status = data['status'] ?? 'OFFLINE';
          } catch (e) {
            // Failsafe in case of malformed initial data
          }
        }

        final isOnline = status == 'ONLINE';

        return Container(
          padding: const EdgeInsets.all(24),
          decoration: BoxDecoration(
            color: const Color(0xFF1E293B),
            borderRadius: BorderRadius.circular(16),
            border: Border.all(color: isOnline ? Colors.greenAccent : Colors.redAccent, width: 2),
            boxShadow: [
              BoxShadow(
                color: (isOnline ? Colors.greenAccent : Colors.redAccent).withOpacity(0.1),
                blurRadius: 20,
              )
            ],
          ),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(
                'LIVE TELEMETRY STREAM', 
                style: TextStyle(color: Colors.blueGrey[300], fontWeight: FontWeight.bold, letterSpacing: 1.5)
              ),
              const Divider(color: Colors.blueGrey, height: 30),
              _TelemetryRow(icon: Icons.wifi, label: 'Signal (RSSI)', value: '${rssi.toStringAsFixed(0)} dBm'),
              _TelemetryRow(icon: Icons.battery_charging_full, label: 'Core Voltage', value: '${battery.toStringAsFixed(2)} V'),
              _TelemetryRow(
                icon: Icons.circle, 
                label: 'System Status', 
                value: status,
                color: isOnline ? Colors.greenAccent : Colors.redAccent,
              ),
            ],
          ),
        );
      },
    );
  }
}

class _TelemetryRow extends StatelessWidget {
  final IconData icon;
  final String label;
  final String value;
  final Color? color;

  const _TelemetryRow({required this.icon, required this.label, required this.value, this.color});

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 10.0),
      child: Row(
        children: [
          Icon(icon, color: color ?? Colors.white70, size: 24),
          const SizedBox(width: 16),
          Text(label, style: const TextStyle(fontSize: 16, color: Colors.white70)),
          const Spacer(),
          Text(
            value, 
            style: TextStyle(
              fontSize: 18, 
              fontWeight: FontWeight.bold, 
              color: color ?? Colors.white,
              fontFamily: 'monospace',
            )
          ),
        ],
      ),
    );
  }
}

class EmoteGrid extends StatelessWidget {
  const EmoteGrid({super.key});

  void _sendEmote(int emoteId) {
    FirebaseDatabase.instance.ref('commands/emotes').set({
      'current_emote': emoteId,
      'timestamp': ServerValue.timestamp,
    });
  }

  @override
  Widget build(BuildContext context) {
    final emotes = [
      {'id': 1, 'label': 'DEFAULT / IDLE', 'icon': Icons.visibility, 'color': Colors.cyanAccent},
      {'id': 2, 'label': 'HAPPY', 'icon': Icons.sentiment_very_satisfied, 'color': Colors.greenAccent},
      {'id': 3, 'label': 'ANGRY', 'icon': Icons.sentiment_very_dissatisfied, 'color': Colors.redAccent},
      {'id': 4, 'label': 'SCANNING', 'icon': Icons.radar, 'color': Colors.purpleAccent},
      {'id': 5, 'label': 'SLEEP MODE', 'icon': Icons.bedtime, 'color': Colors.blueGrey},
      {'id': 6, 'label': 'CONFUSED', 'icon': Icons.question_mark, 'color': Colors.orangeAccent},
    ];

    return Container(
      padding: const EdgeInsets.all(24),
      decoration: BoxDecoration(
        color: const Color(0xFF1E293B),
        borderRadius: BorderRadius.circular(16),
        border: Border.all(color: Colors.blueGrey.withOpacity(0.3)),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'OLED EXPRESSION MATRIX', 
            style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold, color: Colors.blueGrey[300], letterSpacing: 1.5)
          ),
          const SizedBox(height: 24),
          Expanded(
            child: GridView.builder(
              gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
                crossAxisCount: 2,
                crossAxisSpacing: 20,
                mainAxisSpacing: 20,
                childAspectRatio: 1.5,
              ),
              itemCount: emotes.length,
              itemBuilder: (context, index) {
                final emote = emotes[index];
                return InkWell(
                  onTap: () => _sendEmote(emote['id'] as int),
                  borderRadius: BorderRadius.circular(12),
                  child: Container(
                    decoration: BoxDecoration(
                      color: const Color(0xFF0F172A),
                      borderRadius: BorderRadius.circular(12),
                      border: Border.all(color: (emote['color'] as Color).withOpacity(0.3)),
                    ),
                    child: Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        Icon(emote['icon'] as IconData, size: 48, color: emote['color'] as Color),
                        const SizedBox(height: 16),
                        Text(
                          emote['label'] as String, 
                          style: TextStyle(
                            fontSize: 16, 
                            fontWeight: FontWeight.bold, 
                            color: emote['color'] as Color,
                            letterSpacing: 1.2,
                          )
                        ),
                      ],
                    ),
                  ),
                );
              },
            ),
          ),
        ],
      ),
    );
  }
}
