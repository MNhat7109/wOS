#pragma once


typedef enum
{
    DEBUG_INFO,
    DEBUG_WARN,
    DEBUG_CRITICAL,
} KDEBUGF_MODE;

void kdebugf(int mode, const char* module, const char* fmt, ...);