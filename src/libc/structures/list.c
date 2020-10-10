#include <list.h>

// TODO: Make list functions thread safe

list_entry_t list_prepend(list_entry_t head, list_entry_t entry)
{
    if(NULL == head || NULL == entry) return entry;

    entry->prev = NULL;
    entry->next = head;
    head->prev  = entry;
    return entry;
}

list_entry_t list_append(list_entry_t head, list_entry_t entry)
{
    list_entry_t origin_head;

    if(NULL == head || NULL == entry) return entry;

    origin_head = head;
    while(head->next) head = head->next;

    entry->prev = head;
    entry->next = NULL;
    head->next  = entry;
    return origin_head;
}

bool list_insert_before(list_entry_t entry, list_entry_t new_entry)
{
    if(NULL == entry || NULL == new_entry) return false;

    new_entry->next = entry;
    new_entry->prev = entry->prev;

    if(entry->prev)
        entry->prev->next = new_entry;
    entry->prev = new_entry;

    return true;
}

bool list_insert_after(list_entry_t entry, list_entry_t new_entry)
{
    if(NULL == entry || NULL == new_entry) return false;

    new_entry->prev = entry;
    new_entry->next = entry->next;

    if(entry->next)
        entry->next->prev = new_entry;
    entry->next = new_entry;

    return true;
}

list_entry_t list_remove(list_entry_t entry)
{
    if(NULL == entry) return NULL;

    if(entry->prev)
        entry->prev->next = entry->next;
    if(entry->next)
        entry->next->prev = entry->prev;
    
    entry->next = NULL;
    entry->prev = NULL;
    return entry;
}