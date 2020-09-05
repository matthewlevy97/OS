#pragma once

#include <stddef.h>

int klog_init(void *buffer, size_t length);

void klog(const char *msg);