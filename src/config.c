#include "config.h"

#include <stdio.h>
#include <string.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/clib.h>

#include "badtoml/toml.h"
#include "badtoml/keymap.h"
#include "badtoml/tokenizer.h"

int LoadConfig(const char* configFile, PatchworkConfig* out)
{
    char fbuf[200];


    SceUID cfgFile = sceIoOpen(configFile, SCE_O_RDONLY, 0755);
    if (cfgFile < 0)
    {
        return 1;
    }
    SceOff size = sceIoLseek(cfgFile, 0, SEEK_SET);
    if (size > 200)
    {
        return 2;
    }
    sceIoRead(cfgFile, fbuf, sizeof(fbuf));

    Lexer l = MakeLexer(fbuf);

    TOMLEntry entries[4];
    TOMLReadBuffer(&l, entries, 4);

    TOMLKeyMap key_map[] = {
        {MAIN_CONFIG_SECTION, "server_url", TOML_TYPE_STRING, &out->serverUrl},
        {MAIN_CONFIG_SECTION, "join_key", TOML_TYPE_STRING, &out->joinKey},
        {MAIN_CONFIG_SECTION, "digest_key", TOML_TYPE_STRING, &out->digestKey},
        {MAIN_CONFIG_SECTION, "enable_join_key", TOML_TYPE_BOOL, &out->useJoinKey},
    };

    TOMLApplyEntriesToKeyMap(entries, 4, key_map, 4);

    if (!out->serverUrl)
    {
        sceClibPrintf("missing server URL, defaulting to refresh\n");
        out->serverUrl = "http://lbp.lbpbonsai.com/lbp";
    }
    // If it loaded correctly and the last character is a / remove it, this is to make sure the game doesn't accidentally format double //
    if (out->serverUrl[strlen(out->serverUrl) - 1] == '/')
    {
        out->serverUrl[strlen(out->serverUrl) - 1] = '\0';
    }

    if (!out->joinKey)
    {
        sceClibPrintf("missing joinKey, randomizing\n");
    }
    if (!out->digestKey)
    {
        sceClibPrintf("missing digestKey, not patching\n");
    }
    return 0;
}