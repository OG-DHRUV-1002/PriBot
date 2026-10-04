import 'package:firebase_core/firebase_core.dart' show FirebaseOptions;
import 'package:flutter/foundation.dart'
    show defaultTargetPlatform, kIsWeb, TargetPlatform;

class DefaultFirebaseOptions {
  static FirebaseOptions get currentPlatform {
    return const FirebaseOptions(
      apiKey: 'AIzaSyAPYhzTR1pRly67oBDFjf9RcEBSSpDNnjg',
      appId: '1:179112802421:web:e80389947110b4428c603c',
      messagingSenderId: '179112802421',
      projectId: 'pribot-c759e',
      authDomain: 'pribot-c759e.firebaseapp.com',
      databaseURL: 'https://pribot-c759e-default-rtdb.asia-southeast1.firebasedatabase.app',
      storageBucket: 'pribot-c759e.firebasestorage.app',
    );
  }
}
