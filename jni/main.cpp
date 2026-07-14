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

void (*old_SetKillTimer)(PlayerControl* instance, float time) = nullptr;

void hooked_SetKillTimer(PlayerControl* instance, float time) {
    if (instance != nullptr && instance->CachedPlayerData != nullptr) {
        NetworkedPlayerInfo* playerInfo = instance->CachedPlayerData;
        LOGI("[+] Intercepted Player ID: %d | Role: %d", playerInfo->PlayerId, (int)playerInfo->RoleType);
    }
    if (old_SetKillTimer) {
        old_SetKillTimer(instance, time);
    }
}

void* mod_main_thread(void*) {
    while (il2cpp_base == 0) {
        il2cpp_base = get_module_base("libil2cpp.so");
        usleep(100000);
    }
    return nullptr;
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    pthread_t thread;
    pthread_create(&thread, nullptr, mod_main_thread, nullptr);
    return JNI_VERSION_1_6;
}
