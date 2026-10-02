# Project Synth-OS: Deployment & Compilation

## Task 15.1: Android APK Production Overrides

### 1. `android/app/build.gradle` Overrides
To ensure the Firebase RTDB streams compile correctly and the application payload is optimized, these explicit overrides must be injected into the `android/app/build.gradle` configuration:

```gradle
android {
    compileSdkVersion 34 // Required for latest Firebase SDK compatibility

    defaultConfig {
        applicationId "com.synthos.dashboard"
        minSdkVersion 23 // Strict minimum for Firebase + Flutter Riverpod
        targetSdkVersion 34
        versionCode flutterVersionCode.toInteger()
        versionName flutterVersionName
        multiDexEnabled true // Critical: Prevents 64k method reference limit crash
    }

    buildTypes {
        release {
            signingConfig signingConfigs.debug // Replace with production keystore profile
            minifyEnabled true // Strip unused code
            shrinkResources true // Strip unused assets
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
        }
    }
}

dependencies {
    // Force MultiDex resolution for Firebase
    implementation 'androidx.multidex:multidex:2.0.1'
}
```

### 2. Android APK Compilation Command
Execute from the `dashboard_flutter` directory to generate the release artifact:
```bash
flutter build apk --release --target-platform android-arm,android-arm64 --obfuscate --split-debug-info=./build/app/outputs/symbols
```

---

## Task 15.2: Windows Executable Production Overrides

### 1. `windows/CMakeLists.txt` Overrides
To compile a true standalone `.exe` without the debug console window persisting in the background, this override must be placed at the bottom of the Windows CMake configuration:

```cmake
# Hide the Windows terminal console in Release builds
if(MSVC)
  # Override default subsystem from CONSOLE to WINDOWS
  set_target_properties(${BINARY_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
  )
  
  # Force static linking for MSVC to prevent missing VCRUNTIME.dll errors on client machines
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
```

### 2. Windows EXE Compilation Command
Execute from the `dashboard_flutter` directory (Must be run on a Windows host machine):
```powershell
flutter build windows --release
```
The compiled, standalone executable will output to:
`build\windows\x64\runner\Release\synth_os_dashboard.exe`
