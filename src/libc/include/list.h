#pragma once

#include <stdbool.h>
#include <stddef.h>

#define ListEntry_HEAD \
    struct _list_entry *next, *prev;

#define list_foreach(l, e) \
    for(__typeof__(l) e = (l); e != NULL; e = e->next)
#define list_foreach_circular(l, e) \
    for(__typeof__(l) e = (l)->next; e != NULL && e != l; e = e->next)

typedef struct _list_entry {
    ListEntry_HEAD
} *list_entry_t;

list_entry_t list_init(list_entry_t entry);
#define list_init(entry)               \
    do {                               \
        (entry)->next = NULL;          \
        (entry)->prev = NULL;          \
    } while(0);

list_entry_t list_circular_init(list_entry_t entry);
#define list_circular_init(entry)      \
    do {                               \
        (entry)->next = (entry);       \
        (entry)->prev = (entry);       \
    } while(0);

list_entry_t list_prepend(list_entry_t *head, list_entry_t entry);
list_entry_t list_append(list_entry_t *head, list_entry_t entry);

bool list_insert_before(list_entry_t entry, list_entry_t new_entry);
bool list_insert_after(list_entry_t entry, list_entry_t new_entry);

void list_remove(list_entry_t *entry);