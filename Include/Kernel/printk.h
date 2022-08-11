#pragma once

#include "character_device.h"

__attribute__((format(printf, 1, 2))) int printk(const char *format, ...);

__attribute__((format(printf, 1, 2))) int debugk(const char *format, ...);
