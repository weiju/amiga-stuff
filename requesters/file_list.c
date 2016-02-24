#include "file_list.h"
#include <stdlib.h>
#include <string.h>

void free_file_list(struct FileListEntry *entries)
{
    struct FileListEntry *cur = entries, *next;
    while (cur) {
        next = cur->next;
        free(cur);
        cur = next;
    }
}

static struct FileListEntry *splice(struct FileListEntry *list)
{
    struct FileListEntry *fast = list, *slow = list;
    while (fast->next && fast->next->next) {
        fast = fast->next->next;
        slow = slow->next;
    }
    struct FileListEntry *temp = slow->next;
    slow->next = NULL;
    return temp;
}

static struct FileListEntry *merge(struct FileListEntry *list1, struct FileListEntry *list2,
                                   BOOL asc)
{
    if (!list1) return list2;
    if (!list2) return list1;
    int val = strncmp(list1->name, list2->name, MAX_FILENAME_LEN);
    val = asc ? val : -val; // reverse the comparison value if direction reversed

    if (val < 0) {
        list1->next = merge(list1->next, list2, asc);
        list1->next->prev = list1;
        list1->prev = NULL;
        return list1;
    } else {
        list2->next = merge(list1, list2->next, asc);
        list2->next->prev = list2;
        list2->prev = NULL;
        return list2;
    }
}

static struct FileListEntry *_sort_file_list(struct FileListEntry *list, BOOL asc)
{
    if (!list || !list->next) return list;
    struct FileListEntry *list2 = splice(list);
    list = _sort_file_list(list, asc);
    list2 = _sort_file_list(list2, asc);
    return merge(list, list2, asc);
}

struct FileListEntry *sort_file_list(struct FileListEntry *list, BOOL asc)
{
    struct FileListEntry *result = _sort_file_list(list, asc), *cur = result;
    int index = 0;
    // fix the index after sorting
    while (cur) {
        cur->index = index++;
        cur = cur->next;
    }
    return result;
}
struct FileListEntry *new_file_list_entry()
{
    return calloc(1, sizeof(struct FileListEntry));
}
