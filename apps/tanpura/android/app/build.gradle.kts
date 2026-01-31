import java.io.File

plugins {
    id("com.android.application")
    // START: FlutterFire Configuration
    id("com.google.gms.google-services")
    // END: FlutterFire Configuration
    id("kotlin-android")
    // The Flutter Gradle Plugin must be applied after the Android and Kotlin Gradle plugins.
    id("dev.flutter.flutter-gradle-plugin")
}

android {
    namespace = "com.tanpurax.tanpura"
    compileSdk = flutter.compileSdkVersion
    ndkVersion = flutter.ndkVersion
    flavorDimensions += "env"

    productFlavors {
    create("dev") {
        dimension = "env"
        applicationIdSuffix = ".dev"
        versionNameSuffix = "-dev"
        resValue("string", "app_name", "Tanpura Dev")
    }
    create("staging") {
        dimension = "env"
        applicationIdSuffix = ".stg"
        versionNameSuffix = "-stg"
        resValue("string", "app_name", "Tanpura Staging")
    }
    create("prod") {
        dimension = "env"
        // no suffix for production
        resValue("string", "app_name", "Tanpura")
    }


}

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    kotlinOptions {
        jvmTarget = JavaVersion.VERSION_11.toString()
    }

    defaultConfig {
        // TODO: Specify your own unique Application ID (https://developer.android.com/studio/build/application-id.html).
        applicationId = "com.tanpurax.tanpura"
        // You can update the following values to match your application needs.
        // For more information, see: https://flutter.dev/to/review-gradle-config.
        minSdk = 23
        targetSdk = flutter.targetSdkVersion
        versionCode = flutter.versionCode
        versionName = flutter.versionName
    }

    buildTypes {
        release {
            // TODO: Add your own signing config for the release build.
            // Signing with the debug keys for now, so `flutter run --release` works.
            signingConfig = signingConfigs.getByName("debug")
        }
    }
}

gradle.taskGraph.whenReady {

    val taskNames = gradle.startParameter.taskNames.joinToString(" ")

    val flavor = when {
        taskNames.contains("Dev", ignoreCase = true) -> "dev"
        taskNames.contains("Staging", ignoreCase = true) -> "staging"
        taskNames.contains("Prod", ignoreCase = true) -> "prod"
        else -> null
    }

    flavor?.let{it->
        val sourceFile = File(
            projectDir,
            "src/$it/google-services.json"
        )

        val destinationFile = File(
            projectDir,
            "google-services.json"
        )

        if (!sourceFile.exists()) {
            throw GradleException(
                "❌ Missing google-services.json for flavor: $it"
            )
        }

        sourceFile.copyTo(destinationFile, overwrite = true)
        println("✔ Firebase config applied for flavor: $it")
    }

}


flutter {
    source = "../.."
}
