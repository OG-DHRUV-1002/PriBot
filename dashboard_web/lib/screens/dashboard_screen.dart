import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:http/http.dart' as http;
import 'package:app_settings/app_settings.dart';
import 'package:flutter_joystick/flutter_joystick.dart';
import 'package:flutter_animate/flutter_animate.dart';

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({super.key});

  @override
  State<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {
  final FocusNode _focusNode = FocusNode();
  
  // Hardware Input States
  bool _w = false;
  bool _a = false;
  bool _s = false;
  bool _d = false;

  Timer? _pollingTimer;
  double _padX = 0.0;
  double _padY = 0.0;
  double _lastX = 0.0;
  double _lastY = 0.0;
  bool _hasPublishedZero = true;

  // PriBot Local IP Address
  final String _robotIp = "192.168.4.1";

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
    
    // Core WASD and D-Pad movement
    if (event.logicalKey == LogicalKeyboardKey.keyW || event.logicalKey == LogicalKeyboardKey.arrowUp) _w = isDown;
    if (event.logicalKey == LogicalKeyboardKey.keyA || event.logicalKey == LogicalKeyboardKey.arrowLeft) _a = isDown;
    if (event.logicalKey == LogicalKeyboardKey.keyS || event.logicalKey == LogicalKeyboardKey.arrowDown) _s = isDown;
    if (event.logicalKey == LogicalKeyboardKey.keyD || event.logicalKey == LogicalKeyboardKey.arrowRight) _d = isDown;

    // Standard Gamepad Face Buttons (trigger on KeyDown only)
    if (event is KeyDownEvent) {
      if (event.logicalKey == LogicalKeyboardKey.gameButtonA || event.logicalKey.keyLabel.toLowerCase() == "a") _triggerEmote(10);
      else if (event.logicalKey == LogicalKeyboardKey.gameButtonB || event.logicalKey.keyLabel.toLowerCase() == "b") _triggerEmote(11);
      else if (event.logicalKey == LogicalKeyboardKey.gameButtonX || event.logicalKey.keyLabel.toLowerCase() == "x") _triggerEmote(14);
      else if (event.logicalKey == LogicalKeyboardKey.gameButtonY || event.logicalKey.keyLabel.toLowerCase() == "y") _triggerEmote(99);
    }
  }

  void _processAndPublishControlVector() {
    double x = _padX;
    double y = _padY;

    if (_w) y += 1;
    if (_s) y -= 1;
    if (_a) x -= 1;
    if (_d) x += 1;

    // Strict normalization for diagonal movement (max magnitude 1.0)
    if (x > 1.0) x = 1.0;
    if (x < -1.0) x = -1.0;
    if (y > 1.0) y = 1.0;
    if (y < -1.0) y = -1.0;


    bool isZero = (x == 0 && y == 0);
    if (x != _lastX || y != _lastY || (isZero && !_hasPublishedZero)) {
      setState(() {
        _lastX = x;
        _lastY = y;
        _hasPublishedZero = isZero;
      });
      
      // Send HTTP command to PriBot Local IP
      final url = Uri.parse('http://$_robotIp/move?x=${x.toStringAsFixed(2)}&y=${y.toStringAsFixed(2)}');
      http.get(url).catchError((_) => http.Response('Error', 500));
    }
  }

  void _triggerEmote(int emoteFlag) {
    // Send HTTP command to PriBot Local IP
    final url = Uri.parse('http://$_robotIp/emote?id=$emoteFlag');
    http.get(url).catchError((_) => http.Response('Error', 500));
    
    // Regain focus so WASD hardware listener continues to work
    _focusNode.requestFocus();
  }
  
  void _openWifiSettings() {
    AppSettings.openAppSettings(type: AppSettingsType.wifi);
  }

  @override
  Widget build(BuildContext context) {
    final screenWidth = MediaQuery.of(context).size.width;
    final isDesktop = screenWidth > 900; 

    return Scaffold(
      appBar: AppBar(
        backgroundColor: Colors.transparent,
        elevation: 0,
        title: Row(
          children: [
            const Icon(Icons.smart_toy, color: Colors.cyanAccent, size: 32),
            const SizedBox(width: 16),
            Flexible(
              child: FittedBox(
                fit: BoxFit.scaleDown,
                child: Text(
                  'PriBot Command Center',
                  style: TextStyle(
                    fontSize: 24,
                    fontWeight: FontWeight.bold,
                    color: Colors.white,
                    shadows: [
                      Shadow(color: Colors.cyanAccent.withOpacity(0.5), blurRadius: 10),
                    ],
                    letterSpacing: 2.0,
                  ),
                ),
              ),
            ),
          ],
        ),
      ),
      body: Focus(
        focusNode: _focusNode,
        autofocus: true,
        onKeyEvent: (FocusNode node, KeyEvent event) {
          _updateKeys(event);
          return KeyEventResult.handled; 
        },
        child: Container(
          decoration: BoxDecoration(
            gradient: RadialGradient(
              center: Alignment.center,
              radius: 1.5,
              colors: [
                const Color(0xFF151525),
                const Color(0xFF07070F),
              ],
            ),
          ),
          child: SafeArea(
            child: Padding(
              padding: const EdgeInsets.all(24.0),
              child: isDesktop 
                  ? SingleChildScrollView(
                      child: Row(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Expanded(flex: 2, child: _buildControlPanel().animate().fadeIn(duration: 800.ms).slideX(begin: -0.2)),
                          const SizedBox(width: 24),
                          Expanded(flex: 3, child: _buildSidePanel().animate().fadeIn(duration: 800.ms, delay: 200.ms).slideX(begin: 0.2)),
                        ],
                      ),
                    )
                  : SingleChildScrollView(
                      child: Column(
                        children: [
                          _buildControlPanel().animate().fadeIn(duration: 800.ms).slideY(begin: 0.2),
                          const SizedBox(height: 24),
                          _buildSidePanel().animate().fadeIn(duration: 800.ms, delay: 200.ms).slideY(begin: 0.2),
                        ],
                      ),
                    ),
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
        color: const Color(0xFF12121E),
        borderRadius: BorderRadius.circular(24),
        border: Border.all(color: Colors.cyanAccent.withOpacity(0.3), width: 1),
        boxShadow: [
          BoxShadow(color: Colors.cyanAccent.withOpacity(0.1), blurRadius: 20, spreadRadius: 5)
        ],
      ),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Container(
            padding: const EdgeInsets.all(16),
            decoration: BoxDecoration(
              color: Colors.cyanAccent.withOpacity(0.1),
              shape: BoxShape.circle,
            ),
            child: const Icon(Icons.gamepad, size: 64, color: Colors.cyanAccent),
          ),
          const SizedBox(height: 24),
          const Text(
            'SYNTH-OS DIRECT LINK',
            style: TextStyle(
              fontSize: 24, 
              fontWeight: FontWeight.bold,
              letterSpacing: 2,
              color: Colors.white,
            ),
          ),
          const SizedBox(height: 8),
          const Text(
            'WASD / Arrows to Navigate',
            style: TextStyle(color: Colors.white54, letterSpacing: 1),
          ),
          const SizedBox(height: 48),
          
          // Glowing Vector Crosshairs UI
          // Virtual Joystick
          Joystick(
            mode: JoystickMode.all,
            listener: (details) {
              setState(() {
                _padX = details.x;
                _padY = -details.y; // Invert Y so up is positive
              });
            },
            stick: Container(
              width: 60,
              height: 60,
              decoration: BoxDecoration(
                color: Colors.cyanAccent,
                shape: BoxShape.circle,
                boxShadow: [
                  BoxShadow(color: Colors.cyanAccent.withOpacity(0.5), blurRadius: 15, spreadRadius: 5)
                ],
              ),
            ),
            base: Container(
              width: 240,
              height: 240,
              decoration: BoxDecoration(
                color: const Color(0xFF0A0A12),
                shape: BoxShape.circle,
                border: Border.all(color: Colors.cyanAccent.withOpacity(0.5), width: 2),
                boxShadow: [
                  BoxShadow(
                    color: Colors.cyanAccent.withOpacity(_hasPublishedZero ? 0.05 : 0.2), 
                    blurRadius: 30,
                    spreadRadius: 5
                  )
                ]
              ),
            ),
          ),
          const SizedBox(height: 32),
          
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 16),
            decoration: BoxDecoration(
              color: Colors.black45, 
              borderRadius: BorderRadius.circular(12),
              border: Border.all(color: Colors.white10),
            ),
            child: Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                const Icon(Icons.speed, color: Colors.cyanAccent, size: 20),
                const SizedBox(width: 12),
                Flexible(
                  child: FittedBox(
                    fit: BoxFit.scaleDown,
                    child: Text(
                      'VEC: [X: ${_lastX.toStringAsFixed(2)} | Y: ${_lastY.toStringAsFixed(2)}]',
                      style: const TextStyle(fontFamily: 'monospace', fontSize: 16, color: Colors.cyanAccent),
                    ),
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildSidePanel() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _buildConnectionPanel(),
        const SizedBox(height: 24),
        _buildEmoteGrid(),
      ],
    );
  }

  Widget _buildConnectionPanel() {
    return Container(
      padding: const EdgeInsets.all(24),
      decoration: BoxDecoration(
        color: const Color(0xFF12121E),
        borderRadius: BorderRadius.circular(24),
        border: Border.all(color: Colors.pinkAccent.withOpacity(0.3), width: 1),
        boxShadow: [
          BoxShadow(color: Colors.pinkAccent.withOpacity(0.05), blurRadius: 20, spreadRadius: 5)
        ],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              const Icon(Icons.router, color: Colors.pinkAccent, size: 28),
              const SizedBox(width: 12),
              const Flexible(
                child: FittedBox(
                  fit: BoxFit.scaleDown,
                  child: Text(
                    'NETWORK STATUS', 
                    style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold, letterSpacing: 1.5)
                  ),
                ),
              ),
              const SizedBox(width: 8),
              ElevatedButton.icon(
                onPressed: _openWifiSettings,
                icon: const Icon(Icons.wifi_find, size: 18),
                label: const Text('SCAN / CONNECT'),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.pinkAccent.withOpacity(0.2),
                  foregroundColor: Colors.pinkAccent,
                  elevation: 0,
                  side: const BorderSide(color: Colors.pinkAccent),
                  shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
                ),
              )
            ],
          ),
          const Padding(
            padding: EdgeInsets.symmetric(vertical: 16),
            child: Divider(color: Colors.white10, height: 1),
          ),
          Row(
            children: [
              _buildStatusChip('SSID', 'PriBot', Colors.blueAccent),
              const SizedBox(width: 16),
              _buildStatusChip('TARGET', _robotIp, Colors.greenAccent),
              const SizedBox(width: 16),
              _buildStatusChip('PORT', '80', Colors.orangeAccent),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildStatusChip(String label, String value, Color color) {
    return Expanded(
      child: Container(
        padding: const EdgeInsets.all(12),
        decoration: BoxDecoration(
          color: color.withOpacity(0.1),
          borderRadius: BorderRadius.circular(12),
          border: Border.all(color: color.withOpacity(0.3)),
        ),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(label, style: TextStyle(color: color, fontSize: 10, fontWeight: FontWeight.bold, letterSpacing: 1)),
            const SizedBox(height: 4),
            Text(value, style: const TextStyle(color: Colors.white, fontSize: 14, fontWeight: FontWeight.bold)),
          ],
        ),
      ),
    );
  }

  Widget _buildEmoteGrid() {
    return Container(
      padding: const EdgeInsets.all(24),
      decoration: BoxDecoration(
        color: const Color(0xFF12121E),
        borderRadius: BorderRadius.circular(24),
        border: Border.all(color: Colors.purpleAccent.withOpacity(0.3), width: 1),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              const Icon(Icons.auto_awesome, color: Colors.purpleAccent, size: 28),
              const SizedBox(width: 12),
              const Text(
                'PRESET ROUTINES', 
                style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold, letterSpacing: 1.5)
              ),
            ],
          ),
          const SizedBox(height: 24),
          GridView.count(
            shrinkWrap: true,
            physics: const NeverScrollableScrollPhysics(),
            crossAxisCount: 3,
            mainAxisSpacing: 16,
            crossAxisSpacing: 16,
            childAspectRatio: 2.5,
              children: [
                _EmoteButton(label: 'Happy', icon: Icons.sentiment_very_satisfied, flag: 1, color: Colors.greenAccent, onPressed: () => _triggerEmote(1)),
                _EmoteButton(label: 'Angry', icon: Icons.sentiment_very_dissatisfied, flag: 2, color: Colors.redAccent, onPressed: () => _triggerEmote(2)),
                _EmoteButton(label: 'Sad', icon: Icons.sentiment_dissatisfied, flag: 3, color: Colors.blueAccent, onPressed: () => _triggerEmote(3)),
                _EmoteButton(label: 'Surprised', icon: Icons.sentiment_neutral, flag: 4, color: Colors.orangeAccent, onPressed: () => _triggerEmote(4)),
                _EmoteButton(label: 'Sleepy', icon: Icons.bedtime, flag: 5, color: Colors.purpleAccent, onPressed: () => _triggerEmote(5)),
                _EmoteButton(label: 'Heart', icon: Icons.favorite, flag: 6, color: Colors.pinkAccent, onPressed: () => _triggerEmote(6)),
                _EmoteButton(label: 'Stand', icon: Icons.accessibility, flag: 0, color: Colors.blueAccent, onPressed: () => _triggerEmote(0)),
                _EmoteButton(label: 'Wave', icon: Icons.waving_hand, flag: 10, color: Colors.cyanAccent, onPressed: () => _triggerEmote(10)),
                _EmoteButton(label: 'Dance', icon: Icons.music_note, flag: 11, color: Colors.pinkAccent, onPressed: () => _triggerEmote(11)),
                _EmoteButton(label: 'Swim', icon: Icons.pool, flag: 12, color: Colors.blue, onPressed: () => _triggerEmote(12)),
                _EmoteButton(label: 'Point', icon: Icons.touch_app, flag: 13, color: Colors.tealAccent, onPressed: () => _triggerEmote(13)),
                _EmoteButton(label: 'Pushup', icon: Icons.fitness_center, flag: 14, color: Colors.orangeAccent, onPressed: () => _triggerEmote(14)),
                _EmoteButton(label: 'Bow', icon: Icons.keyboard_double_arrow_down, flag: 15, color: Colors.greenAccent, onPressed: () => _triggerEmote(15)),
                _EmoteButton(label: 'Cute', icon: Icons.favorite, flag: 16, color: Colors.redAccent, onPressed: () => _triggerEmote(16)),
                _EmoteButton(label: 'Freaky', icon: Icons.visibility, flag: 17, color: Colors.purpleAccent, onPressed: () => _triggerEmote(17)),
                _EmoteButton(label: 'Worm', icon: Icons.bug_report, flag: 18, color: Colors.lightGreenAccent, onPressed: () => _triggerEmote(18)),
                _EmoteButton(label: 'Shake', icon: Icons.vibration, flag: 19, color: Colors.amberAccent, onPressed: () => _triggerEmote(19)),
                _EmoteButton(label: 'Shrug', icon: Icons.question_mark, flag: 20, color: Colors.deepOrangeAccent, onPressed: () => _triggerEmote(20)),
                _EmoteButton(label: 'Dead', icon: Icons.close, flag: 21, color: Colors.grey, onPressed: () => _triggerEmote(21)),
                _EmoteButton(label: 'Crab', icon: Icons.pest_control, flag: 22, color: Colors.red, onPressed: () => _triggerEmote(22)),
                _EmoteButton(label: 'Rest', icon: Icons.power_settings_new, flag: 99, color: Colors.redAccent, onPressed: () => _triggerEmote(99)),
              ],
            ),
        ],
      ),
    );
  }
}

class _EmoteButton extends StatelessWidget {
  final String label;
  final IconData icon;
  final int flag;
  final Color color;
  final VoidCallback onPressed;

  const _EmoteButton({
    required this.label, 
    required this.icon, 
    required this.flag, 
    required this.color, 
    required this.onPressed
  });

  @override
  Widget build(BuildContext context) {
    return ElevatedButton.icon(
      onPressed: onPressed,
      icon: Icon(icon, size: 20),
      label: Text(label, style: const TextStyle(fontWeight: FontWeight.bold, letterSpacing: 1)),
      style: ElevatedButton.styleFrom(
        backgroundColor: color.withOpacity(0.1),
        foregroundColor: color,
        elevation: 0,
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(12),
          side: BorderSide(color: color.withOpacity(0.5), width: 1.5),
        ),
      ),
    );
  }
}
