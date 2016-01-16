// dos13 - a module for DOS 1.x interaction
#pragma once
#ifndef __DOS13_H__
#define __DOS13_H__

#include <exec/types.h>

#define FILETYPE_FILE   0
#define FILETYPE_DIR    1
#define FILETYPE_VOLUME 2

// This file requester is only for 1.x, so 31 characters is the
// maximum
#define MAX_FILENAME_LEN 31

/*
 * Store file list entries in these entries.
 */
struct FileListEntry {
    struct FileListEntry *next;
    UWORD file_type;
    char name[MAX_FILENAME_LEN + 1];
};

/*
 * scan the current directory
 */
extern struct FileListEntry *scan_current_dir();

/* free the resources allocated in the specified entry list */
void free_file_list(struct FileListEntry *entries);

#endif
