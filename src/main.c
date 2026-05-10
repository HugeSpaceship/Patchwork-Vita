#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/rng.h>
#include <stdlib.h>

#include "clib.h"
#include "sha256.h"
#include "hooks.h"
#include "config.h"


static char LOBBY_PASSWORD[256];

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize argc, const void *args)
{
    _init_vita_newlib(); // this some janky shit because I'm too lazy to do my own memory alloc, I'm sure it will be fine
    sceClibPrintf("allefresher module start! looking for config...\n");
    log_init();
    PatchworkConfig config = {};
    int status = LoadConfig("ux0:/data/allefresher.yaml", &config);
    if (status != 0)
    {
        sceClibPrintf("config loader failed!\n");
    }
    strcpy(GAME_URL, config.serverUrl);

    // Try to load the URL from the file, if it fails, just use the default URL
    if (strlen(config.joinKey) != 0)
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
        sha256_update(sha256, (unsigned char*)config.joinKey, 16);
        sha256_final(sha256, outbuf);
        sceClibMemcpy(NETWORK_KEY, outbuf, 16);

        sceClibPrintf("Loaded user provided lobby password %s\n", LOBBY_PASSWORD);
    }

    sceClibPrintf("Hooking functions...\n");

    add_hooks(config.useJoinKey);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    remove_hooks();
    log_finish();

    return SCE_KERNEL_STOP_SUCCESS;
}
