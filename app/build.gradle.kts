plugins {
    id("com.android.application")
    kotlin("android")
}

android {
    compileSdk = 35
    buildToolsVersion = "35.0.0"
    ndkVersion = "27.2.12479018"

    defaultConfig {
        applicationId = "com.mefazm.rdvr"
        minSdk = 32
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"
        ndk {
            abiFilters += listOf("arm64-v8a")
        }
        externalNativeBuild {
            cmake {
                arguments.add("-Wno-deprecated")
            }
        }
    }

    signingConfigs {
        create("release") {
            storeFile = file("key.jks")
            storePassword = "fDfNfH421!"
            keyAlias = "key"
            keyPassword = "fDfNfH421!"
        }
    }

    buildTypes {
        getByName("release") {
            isMinifyEnabled = true
            signingConfig = signingConfigs.getByName("release")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = "17"
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.31.4"
        }
    }

    buildFeatures {
        dataBinding = true
        viewBinding = true
    }

    namespace = "com.mefazm.rdvr"
}

dependencies {
    implementation("androidx.core:core-ktx:1.15.0")
    implementation("androidx.appcompat:appcompat:1.7.0")
    implementation("com.google.android.material:material:1.12.0")
    implementation("androidx.constraintlayout:constraintlayout:2.2.0")
    implementation("androidx.datastore:datastore-preferences:1.1.1")
}
