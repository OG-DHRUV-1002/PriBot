import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'firebase_repository.dart';

class KeyboardController extends ConsumerStatefulWidget {
  const KeyboardController({Key? key}) : super(key: key);

  @override
  ConsumerState<KeyboardController> createState() => _KeyboardControllerState();
}

class _KeyboardControllerState extends ConsumerState<KeyboardController> {
  final FocusNode _focusNode = FocusNode();
  
  // Track sustained physical key-down states
  bool _isWPressed = false;
  bool _isAPressed = false;
  bool _isSPressed = false;
  bool _isDPressed = false;

  @override
  void initState() {
    super.initState();
    // Automatically grab focus so keyboard events are captured immediately on PC/Web targets
    _focusNode.requestFocus();
  }

  @override
  void dispose() {
    _focusNode.dispose();
    super.dispose();
  }

  void _handleKeyEvent(KeyEvent event) {
    // Only intercept logical WASD keys
    if (event is KeyDownEvent) {
      if (event.logicalKey == LogicalKeyboardKey.keyW) _isWPressed = true;
      if (event.logicalKey == LogicalKeyboardKey.keyA) _isAPressed = true;
      if (event.logicalKey == LogicalKeyboardKey.keyS) _isSPressed = true;
      if (event.logicalKey == LogicalKeyboardKey.keyD) _isDPressed = true;
    } else if (event is KeyUpEvent) {
      if (event.logicalKey == LogicalKeyboardKey.keyW) _isWPressed = false;
      if (event.logicalKey == LogicalKeyboardKey.keyA) _isAPressed = false;
      if (event.logicalKey == LogicalKeyboardKey.keyS) _isSPressed = false;
      if (event.logicalKey == LogicalKeyboardKey.keyD) _isDPressed = false;
    }

    _calculateAndPushVector();
    
    // Trigger UI rebuild for the visual key highlights
    setState(() {});
  }

  void _calculateAndPushVector() {
    double x = 0.0;
    double y = 0.0;

    // Calculate Y axis vector (Forward/Backward)
    if (_isWPressed && !_isSPressed) y = 1.0;
    if (_isSPressed && !_isWPressed) y = -1.0;

    // Calculate X axis vector (Left/Right)
    if (_isAPressed && !_isDPressed) x = -1.0;
    if (_isDPressed && !_isAPressed) x = 1.0;

    // Normalize diagonal vectors so magnitude doesn't exceed 1.0
    // e.g., W+D = x: 0.707, y: 0.707
    if (x != 0.0 && y != 0.0) {
      const double magnitude = 1.41421356; // sqrt(2)
      x = x / magnitude;
      y = y / magnitude;
    }

    // Push calculated vector to Firebase RTDB
    ref.read(firebaseRepoProvider).pushJoystickVector(x, y);
  }

  @override
  Widget build(BuildContext context) {
    return KeyboardListener(
      focusNode: _focusNode,
      onKeyEvent: _handleKeyEvent,
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          const Icon(Icons.keyboard, color: Colors.blueAccent, size: 48),
          const SizedBox(height: 20),
          const Text('PC/WEB KEYBOARD OVERRIDE', 
            style: TextStyle(color: Colors.blueAccent, fontWeight: FontWeight.bold, letterSpacing: 1.5)),
          const SizedBox(height: 30),
          _buildKeyBox('W', _isWPressed),
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              _buildKeyBox('A', _isAPressed),
              _buildKeyBox('S', _isSPressed),
              _buildKeyBox('D', _isDPressed),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildKeyBox(String label, bool isPressed) {
    return Container(
      margin: const EdgeInsets.all(4),
      width: 55,
      height: 55,
      decoration: BoxDecoration(
        color: isPressed ? Colors.blueAccent : const Color(0xFF2A2A2A),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: Colors.blueAccent.withOpacity(0.5)),
        boxShadow: isPressed ? [BoxShadow(color: Colors.blueAccent.withOpacity(0.5), blurRadius: 10)] : [],
      ),
      child: Center(
        child: Text(label, 
          style: TextStyle(
            color: isPressed ? Colors.black : Colors.blueAccent, 
            fontSize: 22, 
            fontWeight: FontWeight.bold
          )
        ),
      ),
    );
  }
}
