#pragma once

char *strcpy(char *dest, const char *src);

int isspace(int c);

void log_init();
void log_finish();
void filelog(const char* line);