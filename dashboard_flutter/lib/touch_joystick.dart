import 'dart:math';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'firebase_repository.dart';

class TouchJoystick extends ConsumerStatefulWidget {
  const TouchJoystick({Key? key}) : super(key: key);

  @override
  ConsumerState<TouchJoystick> createState() => _TouchJoystickState();
}

class _TouchJoystickState extends ConsumerState<TouchJoystick> {
  final double _baseSize = 200.0;
  final double _stickSize = 60.0;
  
  Offset _stickOffset = Offset.zero;

  void _updateStick(Offset localPosition) {
    // Calculate geometric center of the joystick base
    final Offset center = Offset(_baseSize / 2, _baseSize / 2);
    
    // Delta vector from center to the exact thumb touch point
    Offset delta = localPosition - center;
    
    // Calculate Euclidean distance and polar angle (Task 13.2)
    final double distance = delta.distance;
    final double maxDistance = _baseSize / 2 - _stickSize / 2;
    
    // Mathematically constrain the stick thumb to remain inside the circular boundary
    if (distance > maxDistance) {
      final double angle = delta.direction;
      delta = Offset(cos(angle) * maxDistance, sin(angle) * maxDistance);
    }
    
    setState(() {
      _stickOffset = delta;
    });

    // Map delta to normalized driving vectors (-1.0 to 1.0)
    // Note: Y is mathematically inverted here so pushing UP on the screen = Positive Forward Velocity
    final double normalizedX = delta.dx / maxDistance;
    final double normalizedY = -(delta.dy / maxDistance); 

    ref.read(firebaseRepoProvider).pushJoystickVector(normalizedX, normalizedY);
  }

  void _resetStick() {
    // Snap back to absolute zero center when thumb is lifted
    setState(() {
      _stickOffset = Offset.zero;
    });
    ref.read(firebaseRepoProvider).pushJoystickVector(0.0, 0.0);
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        const Icon(Icons.touch_app, color: Colors.blueAccent, size: 48),
        const SizedBox(height: 20),
        const Text('MOBILE TOUCH ANALOG OVERRIDE', 
          style: TextStyle(color: Colors.blueAccent, fontWeight: FontWeight.bold, letterSpacing: 1.5)),
        const SizedBox(height: 40),
        GestureDetector(
          onPanStart: (details) => _updateStick(details.localPosition),
          onPanUpdate: (details) => _updateStick(details.localPosition),
          onPanEnd: (details) => _resetStick(),
          child: Container(
            width: _baseSize,
            height: _baseSize,
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: const Color(0xFF1A1A1A),
              border: Border.all(color: Colors.blueAccent.withOpacity(0.3), width: 3),
              boxShadow: [
                BoxShadow(
                  color: Colors.black.withOpacity(0.5),
                  blurRadius: 20,
                  spreadRadius: 5,
                )
              ]
            ),
            child: Stack(
              children: [
                // Crosshairs
                Center(
                  child: Container(
                    width: _baseSize - 40,
                    height: _baseSize - 40,
                    decoration: BoxDecoration(
                      shape: BoxShape.circle,
                      border: Border.all(color: Colors.white12, width: 1),
                    ),
                  ),
                ),
                // Movable Thumb Stick
                Center(
                  child: Transform.translate(
                    offset: _stickOffset,
                    child: Container(
                      width: _stickSize,
                      height: _stickSize,
                      decoration: BoxDecoration(
                        shape: BoxShape.circle,
                        color: Colors.blueAccent,
                        boxShadow: [
                          BoxShadow(
                            color: Colors.blueAccent.withOpacity(0.6),
                            blurRadius: 15,
                            spreadRadius: 2,
                          )
                        ],
                      ),
                    ),
                  ),
                ),
              ],
            ),
          ),
        ),
      ],
    );
  }
}
