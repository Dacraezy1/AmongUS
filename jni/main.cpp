#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <android/log.h>

#define LOG_TAG "AmongUsMod"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

uintptr_t il2cpp_base = 0;

uintptr_t get_module_base(const char* module_name) {
    uintptr_t addr = 0;
    char line[512];
    FILE* fp = fopen("/proc/self/maps", "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                addr = (uintptr_t)strtoull(line, NULL, 16);
                break;
            }
        }
        fclose(fp);
    }
    return addr;
}

enum RoleTypes : int {
    Crewmate = 0,
    Impostor = 1,
    Scientist = 2,
    Engineer = 3,
    GuardianAngel = 4,
    Shapeshifter = 5
};

struct NetworkedPlayerInfo {
    char pad_0000[0x35];
    uint8_t PlayerId;       // Offset 0x35
    char pad_0036[0x1A];    
    RoleTypes RoleType;     // Offset 0x50
};

struct PlayerControl {
    char pad_0000[0x70];
    NetworkedPlayerInfo* CachedPlayerData; // Offset 0x70
};

// Global player tracking layout data
char tracking_buffer[1024] = "Waiting for game to initialize...";

void* mod_main_thread(void*) {
    while (il2cpp_base == 0) {
        il2cpp_base = get_module_base("libil2cpp.so");
        usleep(100000);
    }
    LOGI("[+] Linked onto libil2cpp at base memory location: %p", (void*)il2cpp_base);
    
    // In practice, a local looping script updates the tracking_buffer variable array 
    // from the player pointers found at base + 0x21B567C
    return nullptr;
}

// --- JNI LINK: Transmits string details directly to your Android UI overlay box ---
extern "C" JNIEXPORT jstring JNICALL
Java_com_mod_OverlayService_getGameTrackingUpdate(JNIEnv* env, jobject thiz) {
    // Return tracking updates straight to the floating menu screen
    return env->NewStringUTF(tracking_buffer);
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    pthread_t thread;
    pthread_create(&thread, nullptr, mod_main_thread, nullptr);
    return JNI_VERSION_1_6;
}
