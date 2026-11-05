#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>

#include "clib.h"
#include "hooks.h"
#include "config.h"



void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize argc, const void *args)
{
    sceClibPrintf("patchwork module start! looking for config...\n");
    log_init();
    PatchworkConfig config = {};
    int status = LoadConfig("ux0:/data/patchwork.yaml", &config);
    if (status != 0)
    {
        sceClibPrintf("config loader failed!\n");
    }

    sceClibPrintf("Hooking functions...\n");

    add_hooks(config);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    remove_hooks();
    log_finish();

    return SCE_KERNEL_STOP_SUCCESS;
}
