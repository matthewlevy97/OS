#include <atomic.h>

// TODO: Use actually atomic methods here

__inline void atomic_set(atomic_t *v, uint32_t i)
{
    v->counter = i;
#if 0
    __asm__ __volatile__ ("mov %1, %0"
        : "+m" (v->counter)
        : "ir" (i));
#endif
}

__inline uint32_t atomic_get(atomic_t *v)
{
    return v->counter;
#if 0
    uint32_t i;
    __asm__ __volatile__ ("mov %1, %0"
        : "+m" (i)
        : "ir" (v->counter));
    return i;
#endif
}

__inline void atomic_inc(atomic_t *v)
{
    v->counter++;
#if 0
    __asm__ __volatile__ ("LOCK incl %0"
	    : "+m" (v->counter));
#endif
}

__inline void atomic_dec(atomic_t *v)
{
    v->counter--;
#if 0
    __asm__ __volatile__ ("LOCK decl %0"
	    : "+m" (v->counter));
#endif
}

__inline char atomic_dec_and_test(atomic_t *v)
{
    return ((--(v->counter)) == 0);
#if 0
    char ret;
    __asm__ __volatile__ ("LOCK decl %0\n\t" \
        "setz %1" 

	    : "+m" (v->counter)
        : "r" (ret));
    
    return ret;
#endif
}