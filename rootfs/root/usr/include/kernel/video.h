#pragma once
#include <stdint.h>

typedef struct boot_info_t boot_info_t;
typedef struct console_backend_t console_backend_t;

void video_init(boot_info_t* boot_info);
console_backend_t* video_get_backend();