#ifndef PATCHWORK_HOOKS_H
#define PATCHWORK_HOOKS_H

#include <taihen.h>
#include <string.h>


static const uint8_t ORIGINAL_NETWORK_KEY[16] = {0x38, 0x82, 0x67, 0x39, 0x3e, 0xad, 0x90, 0x42, 0x28, 0x3d, 0xef, 0x11, 0x0f, 0x2e, 0x3c, 0x89};

static char GAME_URL[256];

static uint8_t NETWORK_KEY[16];

void add_hooks();
void remove_hooks();

uint32_t network_encrypt(int a1,int a2,uint32_t *a3,uint32_t a4,uint8_t* networkKey,int a6);
void user_agent_swap(int* idkDude, char* userAgent);
uint32_t resource_check(int* csr, uint32_t size);
char *getHttpsUrl(int arg1);
char *getHttpUrl(int arg1);
char *getResourceUrl(char *out, char *hash);

#endif