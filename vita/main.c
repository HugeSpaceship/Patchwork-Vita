#include <stdio.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2kern/kernel/utils.h>
#include <psp2/kernel/rng.h>
#include <taihen.h>
#include <stdlib.h>

#include "reader.h"
#include "sha256.h"


char GAME_URL[256];

char LOBBY_PASSWORD[256];
uint8_t NETWORK_KEY[16];

static SceUID https_hook;
static tai_hook_ref_t https_ref;

static SceUID http_hook;
static tai_hook_ref_t http_ref;

static SceUID resource_hook;
static tai_hook_ref_t resource_ref;

static SceUID user_agent_hook;
static tai_hook_ref_t user_agent_ref;

static SceUID network_encrypt_hook;
static tai_hook_ref_t network_encrypt_ref;

static SceUID resource_check_hook;
static tai_hook_ref_t resource_check_ref;

static SceUID logFile;

void filelog(const char* line)
{
    sceIoWrite(logFile, line, strlen(line));
    sceIoWrite(logFile, "\n", 1);
}

// This is the HTTPS url the game uses, its fine if its not actually HTTPS
char *getHttpsUrl(int arg1)
{
    return GAME_URL;
}

// This is the HTTP url the game uses, its fine if its not actually HTTP
char *getHttpUrl(int arg1)
{
    return GAME_URL;
}

// This returns the URL the game uses to fetch a specific resource
char *getResourceUrl(char *out, char *hash)
{
    // http://%s/r/%s

    // TODO: allow resource URL to be set to a different server

    // Copy the base of the URL
    sceClibMemcpy(out, GAME_URL, strlen(GAME_URL));
    // Copy the resource subdir prefix
    sceClibMemcpy(out + strlen(GAME_URL), "/r/", 3);
    // Copy the hash to the end of the URL
    sceClibMemcpy(out + strlen(GAME_URL) + 3, hash, strlen(hash));
    // Null terminate the string
    out[strlen(GAME_URL) + strlen(hash) + 3] = '\0';

    return out;
}

const uint8_t ORIGINAL_NETWORK_KEY[16] = {0x38, 0x82, 0x67, 0x39, 0x3e, 0xad, 0x90, 0x42, 0x28, 0x3d, 0xef, 0x11, 0x0f, 0x2e, 0x3c, 0x89};


uint32_t network_encrypt(int a1,int a2,uint32_t *a3,uint32_t a4,uint8_t* networkKey,int a6) {
    if (sceClibMemcmp(ORIGINAL_NETWORK_KEY, networkKey, 16) == 0) {
      return TAI_CONTINUE(uint32_t, network_encrypt_ref, a1, a2, a3, a4, NETWORK_KEY, a6);
    }
    return TAI_CONTINUE(uint32_t, network_encrypt_ref, a1, a2, a3, a4, networkKey, a6);
}

const char* PATCHWORK_USER_AGENT = "PatchworkLBPV 1.0";

void oogily_boogily(int* idkDude, char* userAgent) {
    filelog("user-agent call");
    return TAI_CONTINUE(void, user_agent_ref, idkDude, PATCHWORK_USER_AGENT);
}

uint32_t resource_check(int* csr, uint32_t size)
{
    static unsigned char buf[64];
    sceClibMemcpy(buf, (void*)*csr, 64);

    if (buf[32] == 0xB) {
        filelog("blocking script");
        return 0;
    }
    return TAI_CONTINUE(uint32_t, resource_check_ref, csr, size);
}

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize argc, const void *args)
{
    logFile = sceIoOpen("ux0:/data/allefresher.log", SCE_O_WRONLY | SCE_O_TRUNC | SCE_O_CREAT, 0777);
    sceClibPrintf("allefresher module start! looking for config...\n");
    sceIoWrite(logFile, "allefresher module start! looking for config...\n", 48);

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
        unsigned int* outbuf = alloca(32);
        SHA256_CTX* sha256 = alloca(sizeof(SHA256_CTX));
        sha256_init(sha256);
        sha256_update(sha256, LOBBY_PASSWORD, 16);
        sha256_final(sha256, outbuf);
        sceClibMemcpy(NETWORK_KEY, outbuf, 16);
        

        sceClibPrintf("Loaded user provided lobby password %s\n", LOBBY_PASSWORD);
    }

    sceClibPrintf("Hooking functions...\n");

    tai_module_info_t info;
    info.size = sizeof(tai_module_info_t);

    taiGetModuleInfo(TAI_MAIN_MODULE, &info);

    // Patch the game's get_https_url function to return our own URL
    https_hook = taiHookFunctionOffset(
        &https_ref,
        info.modid,
        0,        // Segment index
        0x163a7e, // 0x81163a7e
        1,        // ARM/THUMB
        getHttpsUrl);
    sceClibPrintf("Hooked HTTPS: %08x\n", https_hook);

    // Patch the game's get_http_url function to return our own URL
    http_hook = taiHookFunctionOffset(
        &http_ref,
        info.modid,
        0,        // Segment index
        0x163994, // 0x81163994
        1,        // ARM/THUMB
        getHttpUrl);
    sceClibPrintf("Hooked HTTP: %08x\n", http_hook);

    // Patch the game's get_resource_url function to return our own formatted URLs
    resource_hook = taiHookFunctionOffset(
        &resource_ref,
        info.modid,
        0,        // Segment index
        0x163914, // 0x81163914
        1,        // ARM/THUMB
        getResourceUrl);
    sceClibPrintf("Hooked resource URL: %08x\n", resource_hook);

    // Patch the game's get_resource_url function to return our own formatted URLs
    user_agent_hook = taiHookFunctionOffset(
        &user_agent_ref,
        info.modid,
        0,        // Segment index
        0x0127dc, // The thing that sets up a request probably
        1,
        oogily_boogily);
    sceClibPrintf("Hooked user-agent: %08x\n", user_agent_hook);


    // Patch the game's xxtea encryption function to use our custom key
    network_encrypt_hook = taiHookFunctionOffset(
        &network_encrypt_ref,
        info.modid,
        0,        // Segment index
        0x00ce44, // The thing that sets up a request probably
        1,
        network_encrypt);
    sceClibPrintf("Hooked network encryption: %08x\n", network_encrypt_hook);

    // Patch the game's xxtea encryption function to use our custom key
    resource_check_hook = taiHookFunctionOffset(
        &resource_check_ref,
        info.modid,
        0,        // Segment index
        0x1b2086, //
        1,
        resource_check
        );
    sceClibPrintf("Hooked resource check: %08x\n", resource_check_hook);


    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    taiHookRelease(https_hook, https_ref);
    taiHookRelease(http_hook, http_ref);
    taiHookRelease(resource_hook, resource_ref);
    taiHookRelease(user_agent_hook, user_agent_ref);
    taiHookRelease(network_encrypt_hook, network_encrypt_ref);
    taiHookRelease(resource_check_hook, resource_check_ref);

    sceIoClose(logFile);

    return SCE_KERNEL_STOP_SUCCESS;
}
