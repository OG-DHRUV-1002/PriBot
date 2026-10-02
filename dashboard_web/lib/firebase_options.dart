import 'package:firebase_core/firebase_core.dart' show FirebaseOptions;
import 'package:flutter/foundation.dart'
    show defaultTargetPlatform, kIsWeb, TargetPlatform;

class DefaultFirebaseOptions {
  static FirebaseOptions get currentPlatform {
    return const FirebaseOptions(
      apiKey: 'AIzaSyDummyKey-ThisIsAPlaceholderKey123',
      appId: '1:1234567890:web:abcdef123456',
      messagingSenderId: '1234567890',
      projectId: 'dummy-project-id',
      authDomain: 'dummy-project-id.firebaseapp.com',
      databaseURL: 'https://dummy-project-id.firebaseio.com',
      storageBucket: 'dummy-project-id.appspot.com',
    );
  }
}
