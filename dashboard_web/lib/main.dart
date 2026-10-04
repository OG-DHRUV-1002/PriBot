import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'screens/dashboard_screen.dart';

void main() {
  runApp(const SynthOSApp());
}

class SynthOSApp extends StatelessWidget {
  const SynthOSApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'PriBot Control',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        brightness: Brightness.dark,
        scaffoldBackgroundColor: const Color(0xFF0F0F1A), // Deep dark space blue
        colorScheme: const ColorScheme.dark(
          primary: Color(0xFF00E5FF), // Neon cyan
          secondary: Color(0xFFFF00FF), // Neon pink
          surface: Color(0xFF1A1A2E), // Darker panel
        ),
        fontFamily: GoogleFonts.orbitron().fontFamily,
      ),
      home: const DashboardScreen(),
    );
  }
}
