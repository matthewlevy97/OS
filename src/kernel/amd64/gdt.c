#include <amd64/asm/segments.h>
#include <amd64/cpu.h>
#include <amd64/gdt.h>
#include <string.h>

static void fixup_tss(struct segment_descriptor *, void *);

void gdt_init(struct cpu_local_data *cpu)
{
    struct gdt_page *gdt_page;
    struct desc_ptr *ptr;

    gdt_page = &(cpu->gdt_page);
    ptr      = &(cpu->gdt_ptr);

    fixup_tss(&(gdt_page->gdt[GDT_ENTRY_TSS]), &(cpu->tss));
    ptr->address = (uint64_t)&(gdt_page->gdt);

    gdt_reload(ptr);
}

void tss_init(struct cpu_local_data *cpu)
{
    struct tss_data *tss;

    tss = &(cpu->tss);
    memset(tss, 0, sizeof(struct tss_data));
    tss->iomba = sizeof(struct tss_data);

    tss_install();
}

static inline void fixup_tss(struct segment_descriptor *tss_entry, void *tss)
{
    struct segment_xdescriptor *tss_xentry;

    tss_xentry = (struct segment_xdescriptor*)tss_entry;

    tss_xentry->low.base0 = (uint16_t)(uintptr_t)tss;
    tss_xentry->low.base1 = ((uint64_t)(tss) >> 16) & 0xFF;
    tss_xentry->low.base2 = ((uint64_t)(tss) >> 24) & 0xFF;
    tss_xentry->base3 = (((uint64_t)(tss) >> 32) & 0xFFFFFFFFULL);
}