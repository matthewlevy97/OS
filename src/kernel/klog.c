#include <klog.h>
#include <ring_buffer.h>
#include <stdarg.h>

static char hex_table[] = {
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'
};

static ring_buffer_t ring_buffer;

static inline void klog_int(unsigned long long, int);
static inline void klog_string(char*);

int klog_init(void *buffer, size_t length)
{
    if(!ring_buffer_init(&ring_buffer, buffer, length, 0))
        return -1;

    return 0;
}

void klog(const char *fmt, ...)
{
    char *ptr;
    va_list argp;

    disable_interrupts_for {
        va_start(argp, fmt);

        ptr = (char*)fmt;
        while(*ptr != '\0') {
            if(*ptr != '%') {
                ring_buffer_push(&ring_buffer, *ptr++);
                ring_buffer_push(&ring_buffer, 0x07);
            } else {
                ptr++;
                switch(*ptr)
                {
                case '\0':
                    break;
                case '%':
                    ring_buffer_push(&ring_buffer, *ptr);
                    ring_buffer_push(&ring_buffer, 0x07);
                    ptr++;
                    break;
                case 'd':
                    klog_int(va_arg(argp, unsigned long long), 10);
                    ptr++;
                    break;
                case 's':
                    klog_string(va_arg(argp, char*));
                    ptr++;
                    break;
                case 'x':
                    klog_int(va_arg(argp, unsigned long long), 16);
                    ptr++;
                    break;
                default:
                    break;
                }
            }
        }

        va_end(argp);
    }
}

static inline void klog_int(unsigned long long x, int base)
{
    char buf[32];
    size_t i;
    
    i = 0;
    do {
        buf[i++] = hex_table[x % base];
        x /= base;
    } while(x && i < sizeof(buf));

    while(i--) {
        ring_buffer_push(&ring_buffer, buf[i]);
        ring_buffer_push(&ring_buffer, 0x07);
    }
}

static inline void klog_string(char *s)
{
    while(*s != '\0') {
        ring_buffer_push(&ring_buffer, *s++);
        ring_buffer_push(&ring_buffer, 0x07);
    }
}