#pragma once

#include <stdbool.h>
#include <stddef.h>

#define ListEntry_HEAD \
    struct _list_entry *next, *prev;

#define list_foreach(l, e) \
    for(typeof(l) e = (l); e != NULL; e = e->next)

typedef struct _list_entry {
    ListEntry_HEAD
} *list_entry_t;

list_entry_t list_prepend(list_entry_t head, list_entry_t entry);
list_entry_t list_append(list_entry_t head, list_entry_t entry);

bool list_insert_before(list_entry_t entry, list_entry_t new_entry);
bool list_insert_after(list_entry_t entry, list_entry_t new_entry);

list_entry_t list_remove(list_entry_t entry);