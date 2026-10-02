import 'package:firebase_auth/firebase_auth.dart';
import 'package:google_sign_in/google_sign_in.dart';

class AuthService {
  final FirebaseAuth _auth = FirebaseAuth.instance;
  final GoogleSignIn _googleSignIn = GoogleSignIn();
  
  static const String _adminEmail = 'admin.synthos@gmail.com';

  Future<User?> signInWithGoogle() async {
    try {
      final GoogleSignInAccount? googleUser = await _googleSignIn.signIn();
      if (googleUser == null) return null; // User canceled sign-in

      // 1st Check: Validate the Google Account Email before requesting Firebase tokens
      if (googleUser.email != _adminEmail) {
        await _googleSignIn.signOut();
        throw Exception('UNAUTHORIZED ACCESS: Email does not match administrator account.');
      }

      final GoogleSignInAuthentication googleAuth = await googleUser.authentication;
      final OAuthCredential credential = GoogleAuthProvider.credential(
        accessToken: googleAuth.accessToken,
        idToken: googleAuth.idToken,
      );

      final UserCredential userCredential = await _auth.signInWithCredential(credential);
      final User? user = userCredential.user;

      // 2nd Check: Validate the resulting Firebase User
      if (user != null && user.email != _adminEmail) {
        await _auth.signOut();
        await _googleSignIn.signOut();
        throw Exception('UNAUTHORIZED ACCESS: Firebase User email does not match administrator account.');
      }

      return user;
    } catch (e) {
      rethrow;
    }
  }

  Future<void> signOut() async {
    await _googleSignIn.signOut();
    await _auth.signOut();
  }
}
