#pragma once

#define GDT_PRESENT                    (1 << 15)
#define GDT_CODE                       (3 << 11)
#define GDT_LONG                       (1 << 21)