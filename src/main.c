#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/rng.h>
#include <stdlib.h>

#include "clib.h"
#include "reader.h"
#include "sha256.h"
#include "hooks.h"


static char LOBBY_PASSWORD[256];

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize argc, const void *args)
{
    sceClibPrintf("allefresher module start! looking for config...\n");
    log_init();

    // Try to load the URL from the file, if it fails, just use the default URL
    if (readFileFirstLine("ux0:/allefresher.txt", GAME_URL) == 0)
    {
        sceClibPrintf("Failed to read allefresher.txt, using default URL\n");

        strcpy(GAME_URL, "http://refresh.jvyden.xyz:2095/lbp");
    }
    else
    {
        // If it loaded correctly and the last character is a / remove it, this is to make sure the game doesn't accidentally format double //
        if (GAME_URL[strlen(GAME_URL) - 1] == '/')
        {
            GAME_URL[strlen(GAME_URL) - 1] = '\0';
        }

        sceClibPrintf("Loaded user provided URL %s\n", GAME_URL);
    }

    sceClibPrintf("Final base URL: %s\n", GAME_URL);

    // Try to load the URL from the file, if it fails, just use the default URL
    if (readFileFirstLine("ux0:/allefresher_lobby_password.txt", LOBBY_PASSWORD) == 0)
    {
        sceClibPrintf("Failed to read allefresher_lobby_password.txt, randomizing network key\n");

        // definitely random, don't worry about it
        unsigned int* randbuf = alloca(64);
        sceKernelGetRandomNumber(randbuf, 64);
        sceClibMemcpy(NETWORK_KEY, randbuf, 16);
    }
    else
    {
        // SHA256 the password
        unsigned char* outbuf = alloca(32);
        SHA256_CTX* sha256 = alloca(sizeof(SHA256_CTX));
        sha256_init(sha256);
        sha256_update(sha256, (unsigned char*)LOBBY_PASSWORD, 16);
        sha256_final(sha256, outbuf);
        sceClibMemcpy(NETWORK_KEY, outbuf, 16);

        sceClibPrintf("Loaded user provided lobby password %s\n", LOBBY_PASSWORD);
    }

    sceClibPrintf("Hooking functions...\n");

    add_hooks();

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    remove_hooks();
    log_finish();

    return SCE_KERNEL_STOP_SUCCESS;
}
