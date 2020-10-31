#include <list.h>

// TODO: Make list functions thread safe

list_entry_t list_prepend(list_entry_t *head, list_entry_t entry)
{
    if(NULL == head)
        return entry;
    if(NULL == *head || NULL == entry) {
        *head = entry;
        return entry;
    }

    entry->prev   = (*head)->prev;
    entry->next   = *head;
    (*head)->prev = entry;

    // For circular linked lists
    if((*head)->prev)
        (*head)->prev->next = entry;
    return entry;
}

list_entry_t list_append(list_entry_t *head, list_entry_t entry)
{
    list_entry_t tail;

    if(NULL == head)
        return entry;
    if(NULL == *head || NULL == entry) {
        *head = entry;
        return entry;
    }

    tail = *head;
    if(tail->prev) {
        // Circular linked lists
        tail = tail->prev;
    } else {
        // Regular linked list
        while(tail->next) tail = tail->next;
    }

    entry->prev = tail;
    entry->next = tail->next;
    tail->next  = entry;
    return entry;
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

void list_remove(list_entry_t *entry)
{
    if(NULL == entry || NULL == *entry)
        return;

    if((*entry)->prev)
        (*entry)->prev->next = (*entry)->next;
    if((*entry)->next)
        (*entry)->next->prev = (*entry)->prev;
    
    (*entry)->next = NULL;
    (*entry)->prev = NULL;
}