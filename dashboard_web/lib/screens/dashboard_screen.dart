import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_database/firebase_database.dart';

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({super.key});

  @override
  State<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {
  final FocusNode _focusNode = FocusNode();
  final DatabaseReference _dbRef = FirebaseDatabase.instance.ref();
  
  // Hardware Input States
  bool _w = false;
  bool _a = false;
  bool _s = false;
  bool _d = false;

  Timer? _pollingTimer;
  double _lastX = 0.0;
  double _lastY = 0.0;
  bool _hasPublishedZero = true;

  @override
  void initState() {
    super.initState();
    _focusNode.requestFocus();
    // 20Hz publishing loop for hardware vectors
    _pollingTimer = Timer.periodic(const Duration(milliseconds: 50), (_) {
      _processAndPublishControlVector();
    });
  }

  @override
  void dispose() {
    _focusNode.dispose();
    _pollingTimer?.cancel();
    super.dispose();
  }

  void _updateKeys(KeyEvent event) {
    bool isDown = event is KeyDownEvent || event is KeyRepeatEvent;
    
    if (event.logicalKey == LogicalKeyboardKey.keyW) _w = isDown;
    if (event.logicalKey == LogicalKeyboardKey.keyA) _a = isDown;
    if (event.logicalKey == LogicalKeyboardKey.keyS) _s = isDown;
    if (event.logicalKey == LogicalKeyboardKey.keyD) _d = isDown;
  }

  void _processAndPublishControlVector() {
    double x = 0;
    double y = 0;

    if (_w) y += 1;
    if (_s) y -= 1;
    if (_a) x -= 1;
    if (_d) x += 1;

    // Strict normalization for diagonal movement (max magnitude 1.0)
    if (x != 0 && y != 0) {
      final length = 1.41421356; // sqrt(2)
      x /= length;
      y /= length;
    }

    bool isZero = (x == 0 && y == 0);
    if (x != _lastX || y != _lastY || (isZero && !_hasPublishedZero)) {
      setState(() {
        _lastX = x;
        _lastY = y;
        _hasPublishedZero = isZero;
      });
      
      _dbRef.child('control/wasd').set({
        'w': _w,
        'a': _a,
        's': _s,
        'd': _d,
        'x': x,
        'y': y,
        'timestamp': ServerValue.timestamp,
      });
    }
  }

  void _triggerEmote(int emoteFlag) {
    // Push the precise integer flag to RTDB
    _dbRef.child('commands/emotes').set({
      'active_emote_flag': emoteFlag,
      'trigger_timestamp': ServerValue.timestamp,
    });
    
    // Regain focus so WASD hardware listener continues to work
    _focusNode.requestFocus();
  }

  Future<void> _signOut() async {
    await FirebaseAuth.instance.signOut();
  }

  @override
  Widget build(BuildContext context) {
    final screenWidth = MediaQuery.of(context).size.width;
    final isDesktop = screenWidth > 800; // Basic responsive threshold

    return Scaffold(
      appBar: AppBar(
        title: const Text('Enterprise Dashboard', style: TextStyle(fontWeight: FontWeight.bold)),
        actions: [
          IconButton(
            icon: const Icon(Icons.logout),
            onPressed: _signOut,
            tooltip: 'Sign Out (Lockdown)',
          )
        ],
      ),
      body: Focus(
        focusNode: _focusNode,
        autofocus: true,
        onKeyEvent: (FocusNode node, KeyEvent event) {
          _updateKeys(event);
          return KeyEventResult.handled; // Capture keys so browser doesn't scroll
        },
        child: Padding(
          padding: const EdgeInsets.all(24.0),
          child: isDesktop 
              ? Row(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Expanded(flex: 2, child: _buildControlPanel()),
                    const SizedBox(width: 24),
                    Expanded(flex: 1, child: _buildSidePanel()),
                  ],
                )
              : SingleChildScrollView(
                  child: Column(
                    children: [
                      _buildControlPanel(),
                      const SizedBox(height: 24),
                      _buildSidePanel(),
                    ],
                  ),
                ),
        ),
      ),
    );
  }

  Widget _buildControlPanel() {
    return Container(
      padding: const EdgeInsets.all(32),
      decoration: BoxDecoration(
        color: const Color(0xFF1E1E1E),
        borderRadius: BorderRadius.circular(16),
        boxShadow: const [BoxShadow(color: Colors.black45, blurRadius: 10, offset: Offset(0, 5))],
      ),
      child: Column(
        children: [
          const Icon(Icons.sports_esports, size: 48, color: Colors.blueAccent),
          const SizedBox(height: 16),
          const Text(
            'WASD Hardware Control',
            style: TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
          ),
          const Text(
            'Focus is locked. Use W, A, S, D to drive.',
            style: TextStyle(color: Colors.grey),
          ),
          const SizedBox(height: 48),
          
          // Vector Crosshairs UI
          Container(
            width: 200,
            height: 200,
            decoration: BoxDecoration(
              color: Colors.white10,
              border: Border.all(color: Colors.blueAccent, width: 2),
              shape: BoxShape.circle,
            ),
            child: Stack(
              children: [
                Center(child: Container(width: 2, height: 200, color: Colors.blueAccent.withOpacity(0.3))),
                Center(child: Container(width: 200, height: 2, color: Colors.blueAccent.withOpacity(0.3))),
                Positioned(
                  left: 100 + (_lastX * 90) - 15,
                  top: 100 - (_lastY * 90) - 15, // Y is inverted visually in Flutter
                  child: Container(
                    width: 30,
                    height: 30,
                    decoration: const BoxDecoration(
                      color: Colors.cyanAccent,
                      shape: BoxShape.circle,
                      boxShadow: [
                        BoxShadow(color: Colors.cyanAccent, blurRadius: 10, spreadRadius: 2)
                      ],
                    ),
                  ),
                ),
              ],
            ),
          ),
          const SizedBox(height: 48),
          
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 16),
            decoration: BoxDecoration(color: Colors.black26, borderRadius: BorderRadius.circular(12)),
            child: Text(
              'Vector Output: (X: ${_lastX.toStringAsFixed(2)}, Y: ${_lastY.toStringAsFixed(2)})',
              style: const TextStyle(fontFamily: 'monospace', fontSize: 18),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildSidePanel() {
    return Column(
      children: [
        _buildTelemetryPanel(),
        const SizedBox(height: 24),
        _buildEmoteGrid(),
      ],
    );
  }

  Widget _buildTelemetryPanel() {
    return Container(
      width: double.infinity,
      padding: const EdgeInsets.all(24),
      decoration: BoxDecoration(
        color: const Color(0xFF1E1E1E),
        borderRadius: BorderRadius.circular(16),
        boxShadow: const [BoxShadow(color: Colors.black45, blurRadius: 10, offset: Offset(0, 5))],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text('Live Telemetry', style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold)),
          const SizedBox(height: 16),
          StreamBuilder(
            stream: _dbRef.child('telemetry').onValue,
            builder: (context, AsyncSnapshot<DatabaseEvent> snapshot) {
              if (snapshot.hasError) {
                return const Text('Error loading telemetry stream.', style: TextStyle(color: Colors.redAccent));
              }
              if (snapshot.connectionState == ConnectionState.waiting) {
                return const Center(child: CircularProgressIndicator());
              }

              final data = snapshot.data?.snapshot.value as Map<dynamic, dynamic>?;
              
              if (data == null) {
                return const Text('Awaiting incoming data payload...', style: TextStyle(color: Colors.grey));
              }

              final int battery = data['battery_level'] ?? 0;
              final int signal = data['signal_strength'] ?? 0;
              final String status = data['status'] ?? 'UNKNOWN';

              return Column(
                children: [
                  _TelemetryRow(icon: Icons.battery_charging_full, label: 'Battery', value: '$battery%'),
                  const SizedBox(height: 16),
                  _TelemetryRow(icon: Icons.wifi, label: 'RSSI', value: '$signal dBm'),
                  const SizedBox(height: 16),
                  _TelemetryRow(
                    icon: Icons.circle, 
                    label: 'System Status', 
                    value: status.toUpperCase(), 
                    iconColor: status.toLowerCase() == 'online' ? Colors.greenAccent : Colors.redAccent,
                  ),
                ],
              );
            },
          ),
        ],
      ),
    );
  }

  Widget _buildEmoteGrid() {
    return Container(
      width: double.infinity,
      padding: const EdgeInsets.all(24),
      decoration: BoxDecoration(
        color: const Color(0xFF1E1E1E),
        borderRadius: BorderRadius.circular(16),
        boxShadow: const [BoxShadow(color: Colors.black45, blurRadius: 10, offset: Offset(0, 5))],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text('Action & Emote Matrix', style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold)),
          const SizedBox(height: 16),
          GridView.count(
            shrinkWrap: true,
            crossAxisCount: 2,
            mainAxisSpacing: 12,
            crossAxisSpacing: 12,
            childAspectRatio: 2.0, // Wider layout for the buttons
            physics: const NeverScrollableScrollPhysics(),
            children: [
              _EmoteButton(label: 'Happy', icon: Icons.sentiment_satisfied, flag: 1, onPressed: () => _triggerEmote(1)),
              _EmoteButton(label: 'Angry', icon: Icons.sentiment_very_dissatisfied, flag: 2, onPressed: () => _triggerEmote(2)),
              _EmoteButton(label: 'Scan', icon: Icons.radar, flag: 3, onPressed: () => _triggerEmote(3)),
              _EmoteButton(label: 'Sleep', icon: Icons.bedtime, flag: 4, onPressed: () => _triggerEmote(4)),
            ],
          )
        ],
      ),
    );
  }
}

class _TelemetryRow extends StatelessWidget {
  final IconData icon;
  final String label;
  final String value;
  final Color? iconColor;

  const _TelemetryRow({required this.icon, required this.label, required this.value, this.iconColor});

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        Icon(icon, color: iconColor ?? Colors.blueAccent, size: 24),
        const SizedBox(width: 12),
        Text(label, style: const TextStyle(fontSize: 16, color: Colors.grey)),
        const Spacer(),
        Text(value, style: const TextStyle(fontSize: 16, fontWeight: FontWeight.bold)),
      ],
    );
  }
}

class _EmoteButton extends StatelessWidget {
  final String label;
  final IconData icon;
  final int flag;
  final VoidCallback onPressed;

  const _EmoteButton({required this.label, required this.icon, required this.flag, required this.onPressed});

  @override
  Widget build(BuildContext context) {
    return ElevatedButton.icon(
      onPressed: onPressed,
      icon: Icon(icon, size: 20),
      label: Text(label),
      style: ElevatedButton.styleFrom(
        backgroundColor: const Color(0xFF2C2C2C),
        foregroundColor: Colors.white,
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(8),
          side: const BorderSide(color: Colors.blueAccent, width: 1),
        ),
      ),
    );
  }
}
