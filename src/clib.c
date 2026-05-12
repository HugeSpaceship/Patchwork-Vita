// This file provides implementations for some common libc functions we need, since we dont want to link against libc proper

#include <psp2/io/fcntl.h>
#include "clib.h"

char *strcpy(char *dest, const char *src)
{
    char *ret = dest;

    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';

    return ret;
}

int strlen(const char *s)
{
    int len = 0;
    while (*s++)
        len++;
    return len;
}

int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

static SceUID logFile;
void log_init()
{
    logFile = sceIoOpen("ux0:/data/allefresher.log", SCE_O_WRONLY | SCE_O_TRUNC | SCE_O_CREAT, 0777);
    sceIoWrite(logFile, "allefresher module start! looking for config...\n", 48);
}

void log_finish()
{
    sceIoClose(logFile);
}

void filelog(const char* line)
{
    sceIoWrite(logFile, line, strlen(line));
    sceIoWrite(logFile, "\n", 1);
}