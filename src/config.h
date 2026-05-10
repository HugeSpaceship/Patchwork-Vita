//
// Created by Henry Asbridge on 10/05/2026.
//

#ifndef PATCHWORK_VITA_CONFIG_H
#define PATCHWORK_VITA_CONFIG_H
#include <stdbool.h>

typedef struct
{
    char* serverUrl;
    char* joinKey;
    char* digestKey;
    bool useJoinKey;
} PatchworkConfig;

int LoadConfig(const char* configFile, PatchworkConfig* out);

#endif //PATCHWORK_VITA_CONFIG_H
