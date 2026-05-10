#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <psp2/kernel/clib.h>

#include "../libraries/tomlc17/src/tomlc17.h"

int LoadConfig(const char* configFile, PatchworkConfig* out)
{
    toml_result_t result = toml_parse_file_ex(configFile);

    // Check for parse error
    if (!result.ok)
    {
        return 1;
    }

    toml_datum_t serverURL = toml_seek(result.toptab, "patchwork.server_url");
    toml_datum_t joinKey = toml_seek(result.toptab, "patchwork.join_key");
    toml_datum_t digestKey = toml_seek(result.toptab, "patchwork.digest_key");
    toml_datum_t useJoinKey = toml_seek(result.toptab, "patchwork.use_join_key");

    if (serverURL.type != TOML_STRING)
    {
        printf("missing server URL, defaulting to refresh\n");
        out->serverUrl = "http://lbp.lbpbonsai.com/lbp";
    }
    strcpy(out->serverUrl, serverURL.u.s);
    // If it loaded correctly and the last character is a / remove it, this is to make sure the game doesn't accidentally format double //
    if (out->serverUrl[strlen(out->serverUrl) - 1] == '/')
    {
        out->serverUrl[strlen(out->serverUrl) - 1] = '\0';
    }


    if (joinKey.type != TOML_STRING)
    {
        printf("missing joinKey, randomizing\n");
    }
    strcpy(out->joinKey, joinKey.u.s);
    if (digestKey.type != TOML_STRING)
    {
        printf("missing digestKey, not patching\n");
    }
    strcpy(out->digestKey, digestKey.u.s);
    if (useJoinKey.type != TOML_BOOLEAN)
    {
        printf("missing useJoinKey, enabling by default\n");
        out->useJoinKey = true;
    }
    out->useJoinKey = useJoinKey.u.boolean;

    toml_free(result);

    return 0;
}