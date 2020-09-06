#pragma once

#define __packed                       __attribute__((packed))
#define __align(bytes)                 __attribute__ ((aligned (bytes)))
#define __inline                       inline __attribute__((__always_inline__))