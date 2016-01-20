#include "dos13.h"

#include <stdlib.h>
#include <string.h>

#include <dos/dosextens.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_stdio_protos.h>

extern struct Library *DOSBase;

/*
  accessing the list of logical volumes is hairy under AmigaOS 1.x - it is
  "hidden" in the DosBase structure, and shared among all tasks.

  Effectively this means, we need to nest the retrieval of the assigns into
  Forbid()/Permit() calls to lock the assign list to prevent possible
  race conditions.

  Reminder: We need to do some BCPL->C conversion here
  BPTR are address pointers divided by 4, use BADDR to convert
  BSTR are BPTRs to 0-terminated ASCII char arrays with a prefix length byte

  For now, we will omit device typed entries, because writing/reading to them
  usually doesn't make too much sense.
*/
struct FileListEntry *scan_dir(const char *dirpath, int *num_entries)
{
    struct DosLibrary *dosbase = (struct DosLibrary *) DOSBase;
    Forbid();
    struct DosInfo *dosinfo = BADDR(dosbase->dl_Root->rn_Info);
    struct DevInfo *dev_head = BADDR(dosinfo->di_DevInfo);
    struct DevInfo *current = dev_head;
    struct FileListEntry *cur_entry = NULL, *result = NULL, *tmp;
    int n = 0;
    char fname_len;

    while (current) {
        if (current->dvi_Type != DLT_DEVICE) {
            tmp = calloc(1, sizeof(struct FileListEntry));
            tmp->file_type = FILETYPE_VOLUME;
            strncpy(tmp->name, ((char *) BADDR(current->dvi_Name)) + 1, MAX_FILENAME_LEN);
            fname_len = strlen(tmp->name);
            // add the colon character to point out that we have a logical volume
            if (fname_len < MAX_FILENAME_LEN) tmp->name[fname_len] = ':';
            if (!cur_entry) result = cur_entry = tmp;
            else {
                cur_entry->next = tmp;
                tmp->prev = cur_entry;
                cur_entry = tmp;
            }
            n++;
        }
        current = BADDR(current->dvi_Next);
    }
    Permit();
    /* scan current directory */
    /*
      on AmigaOS versions before 36 (essentially all 1.x versions), the
      function GetCurrentDirName() does not exist, but it's obtainable
      by calling Cli() and querying the returned CommandLineInterface
      structure
    */
    /*
    puts("scanning directory...");
    // on AmigaOS 1.x, this function does not exist !!!
    GetCurrentDirName(dirname, PATHBUFFER_SIZE);
    printf("current dir: '%s'\n", dirname);
    flock = Lock(dirname, SHARED_LOCK);
    if (Examine(flock, &fileinfo)) {
        while (ExNext(flock, &fileinfo)) {
            print_fileinfo(&fileinfo);
        }
        error = IoErr();
        if (error != ERROR_NO_MORE_ENTRIES) {
            puts("unknown I/O error, TODO handle");
        }
    }
    UnLock(flock);
    */
    /* The result is an unsorted list, which is usually not what we want.
       TODO: sort the list
     */
    *num_entries = n;
    return result;
}

void free_file_list(struct FileListEntry *entries)
{
    struct FileListEntry *cur = entries, *next;
    while (cur) {
        next = cur->next;
        free(cur);
        cur = next;
    }
}
