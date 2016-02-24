#pragma once
#ifndef __FILE_LIST_H__
#define __FILE_LIST_H__

/*
 * File list implementation, this is a system independent module that can be unit tested
 * easily.
 */
#ifdef __VBCC__
#include <exec/types.h>

#else

#include <stdint.h>
typedef uint16_t UWORD;
typedef uint8_t BOOL;
#ifndef NULL
#define NULL (0L)
#endif
#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#endif /* __VBCC__ */

#define FILETYPE_FILE   0
#define FILETYPE_DIR    1
#define FILETYPE_VOLUME 2

// This file requester is only for 1.x, so 31 characters is the
// maximum
#define MAX_FILENAME_LEN 31

/*
 * Store file list entries in these entries. Storing a previous pointer allows us to
 * navigate backwards, e.g. for scrolling up a file list.
 */
struct FileListEntry {
    struct FileListEntry *next, *prev;
    UWORD file_type;
    UWORD index; // index in the list
    UWORD selected;
    char name[MAX_FILENAME_LEN + 1];
};

extern void free_file_list(struct FileListEntry *entries);

extern struct FileListEntry *new_file_list_entry();
extern struct FileListEntry *sort_file_list(struct FileListEntry *list, BOOL asc);

#endif /* __FILE_LIST_H__ */
