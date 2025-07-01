plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
    alias(libs.plugins.kotlin.compose)
}

android {
    namespace = "com.cyrus.example"
    compileSdk = 34

    signingConfigs {
        create("cyrus") {
            storeFile = rootProject.file("cyrus.jks")
            storePassword = "cyrus_studio"
            keyAlias = "cyrus_studio"
            keyPassword = "cyrus_studio"
        }
    }

    defaultConfig {
        applicationId = "com.cyrus.example"
        minSdk = 26
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        externalNativeBuild {
            cmake {
                cppFlags += ""
            }
        }

        ndk {
//            abiFilters += listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64")
//            abiFilters += "armeabi-v7a"
            abiFilters += "arm64-v8a"
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.30.3" // 你可以根据需求修改CMake版本
        }
    }

    buildTypes {
        debug {
            signingConfig = signingConfigs.getByName("cyrus")
        }
        release {
            signingConfig = signingConfigs.getByName("cyrus")
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    kotlinOptions {
        jvmTarget = "11"
    }
    buildFeatures {
        compose = true
        prefab = true
    }
    ndkVersion = "27.1.12297006"
}

dependencies {
    implementation(project(":vmp"))
    // ViewPager + Tab
    implementation(libs.accompanist.pager) // 或最新版
    implementation(libs.accompanist.pager.indicators)
    implementation(libs.okhttp)
    implementation(libs.logging.interceptor)
    implementation(libs.libsu.core)
    implementation(libs.libsu.service)
    implementation(libs.libsu.nio)
    implementation(libs.shadowhook)
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.lifecycle.runtime.ktx)
    implementation(libs.androidx.activity.compose)
    implementation(platform(libs.androidx.compose.bom))
    implementation(libs.androidx.ui)
    implementation(libs.androidx.ui.graphics)
    implementation(libs.androidx.ui.tooling.preview)
    implementation(libs.androidx.material3)
    implementation(libs.androidx.appcompat)
    implementation(libs.material)
    implementation(libs.androidx.activity)
    implementation(libs.androidx.constraintlayout)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(platform(libs.androidx.compose.bom))
    androidTestImplementation(libs.androidx.ui.test.junit4)
    debugImplementation(libs.androidx.ui.tooling)
    debugImplementation(libs.androidx.ui.test.manifest)
}