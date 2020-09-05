#pragma once

#define MSR_EFER                       (0xC0000080)

#define EFER_SYSCALL                   (1 << 0)  /* Enable SYSCALL/SYSRET */
#define EFER_LME                       (1 << 8)  /* Long Mode Enable */
#define EFER_LMA                       (1 << 10) /* Long Mode Active */
#define EFER_NXE                       (1 << 11) /* No-Execute Enable*/