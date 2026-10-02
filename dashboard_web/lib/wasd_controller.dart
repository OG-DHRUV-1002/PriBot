import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:firebase_database/firebase_database.dart';

class WASDController extends StatefulWidget {
  const WASDController({super.key});

  @override
  State<WASDController> createState() => _WASDControllerState();
}

class _WASDControllerState extends State<WASDController> {
  final FocusNode _focusNode = FocusNode();
  final DatabaseReference _dbRef = FirebaseDatabase.instance.ref('control/wasd');
  
  bool _isWPressed = false;
  bool _isAPressed = false;
  bool _isSPressed = false;
  bool _isDPressed = false;

  Timer? _pollingTimer;

  @override
  void initState() {
    super.initState();
    // 50Hz polling rate (20ms) to match the ESP32 hardware PWM loop.
    // Constantly pushes vectors even if keyboard repeat events stall.
    _pollingTimer = Timer.periodic(const Duration(milliseconds: 20), (_) {
      _publishControlVectors();
    });
    
    // Auto-focus to immediately capture keyboard input on render
    WidgetsBinding.instance.addPostFrameCallback((_) {
      _focusNode.requestFocus();
    });
  }

  @override
  void dispose() {
    _focusNode.dispose();
    _pollingTimer?.cancel();
    // Zero out velocity before destruction as a failsafe
    _dbRef.set({'x': 0.0, 'y': 0.0, 'timestamp': ServerValue.timestamp});
    super.dispose();
  }

  void _publishControlVectors() {
    double x = 0.0;
    double y = 0.0;

    if (_isWPressed) y += 1.0;
    if (_isSPressed) y -= 1.0;
    if (_isDPressed) x += 1.0;
    if (_isAPressed) x -= 1.0;

    // Normalize diagonal movement to prevent motor overdrive (max length 1.0)
    if (x != 0 && y != 0) {
      x *= 0.7071; // 1 / sqrt(2)
      y *= 0.7071;
    }

    _dbRef.set({
      'x': double.parse(x.toStringAsFixed(3)),
      'y': double.parse(y.toStringAsFixed(3)),
      'timestamp': ServerValue.timestamp,
    });
  }

  KeyEventResult _handleKeyEvent(FocusNode node, KeyEvent event) {
    bool isDown = event is KeyDownEvent || event is KeyRepeatEvent;
    bool handled = false;

    if (event.logicalKey == LogicalKeyboardKey.keyW) {
      if (_isWPressed != isDown) { _isWPressed = isDown; handled = true; }
    } else if (event.logicalKey == LogicalKeyboardKey.keyA) {
      if (_isAPressed != isDown) { _isAPressed = isDown; handled = true; }
    } else if (event.logicalKey == LogicalKeyboardKey.keyS) {
      if (_isSPressed != isDown) { _isSPressed = isDown; handled = true; }
    } else if (event.logicalKey == LogicalKeyboardKey.keyD) {
      if (_isDPressed != isDown) { _isDPressed = isDown; handled = true; }
    }

    if (handled) {
      setState(() {});
      return KeyEventResult.handled;
    }
    return KeyEventResult.ignored;
  }

  @override
  Widget build(BuildContext context) {
    return Focus(
      focusNode: _focusNode,
      onKeyEvent: _handleKeyEvent,
      child: GestureDetector(
        onTap: () => _focusNode.requestFocus(),
        child: Container(
          padding: const EdgeInsets.all(24),
          decoration: BoxDecoration(
            color: Colors.black54,
            border: Border.all(
              color: _focusNode.hasFocus ? Colors.cyanAccent : Colors.redAccent,
              width: 2,
            ),
            borderRadius: BorderRadius.circular(12),
            boxShadow: _focusNode.hasFocus 
                ? [BoxShadow(color: Colors.cyan.withOpacity(0.2), blurRadius: 20)]
                : [],
          ),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              Text(
                _focusNode.hasFocus ? 'WASD TELEMETRY LINK: ACTIVE' : 'WARNING: FOCUS LOST - CLICK TO RE-ENGAGE',
                style: TextStyle(
                  fontWeight: FontWeight.bold,
                  color: _focusNode.hasFocus ? Colors.cyanAccent : Colors.redAccent,
                  letterSpacing: 1.5,
                ),
              ),
              const SizedBox(height: 24),
              Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  _KeyIndicator(label: 'W', isActive: _isWPressed),
                ],
              ),
              Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  _KeyIndicator(label: 'A', isActive: _isAPressed),
                  _KeyIndicator(label: 'S', isActive: _isSPressed),
                  _KeyIndicator(label: 'D', isActive: _isDPressed),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}

class _KeyIndicator extends StatelessWidget {
  final String label;
  final bool isActive;

  const _KeyIndicator({required this.label, required this.isActive});

  @override
  Widget build(BuildContext context) {
    return Container(
      margin: const EdgeInsets.all(6),
      width: 60,
      height: 60,
      decoration: BoxDecoration(
        color: isActive ? Colors.cyan : Colors.grey[900],
        borderRadius: BorderRadius.circular(10),
        boxShadow: isActive
            ? [BoxShadow(color: Colors.cyanAccent, blurRadius: 15, spreadRadius: 1)]
            : [],
      ),
      alignment: Alignment.center,
      child: Text(
        label,
        style: TextStyle(
          fontSize: 28,
          fontWeight: FontWeight.bold,
          color: isActive ? Colors.black : Colors.white54,
        ),
      ),
    );
  }
}
