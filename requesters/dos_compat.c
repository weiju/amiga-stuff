#include <stddef.h>
#include <string.h>

#include "dos_compat.h"

const char *dc_PathPart(const char *path)
{
    if (path) {
        int slen = strlen(path);
        if (slen == 0) return path;
        int i = slen - 1;
        for (i = slen - 1; i > 0; i--) {
            if (path[i] == '/') break;
            if (path[i] == ':' && i < (slen - 1)) return &(path[i + 1]);
        }
        return &(path[i]);
    }
    return NULL;
}

const char *dc_FilePart(const char *path)
{
    if (path) {
        int slen = strlen(path);
        if (slen == 0) return path;
        int i = slen - 1;
        for (i = slen - 1; i > 0; i--) {
            if (path[i] == '/') return &(path[i + 1]);
            if (path[i] == ':' && i < (slen - 1)) return &(path[i + 1]);
        }
        return &(path[i]);
    }
    return NULL;
}

int dc_AddPart(char *dest_path, const char *part_to_add, int dest_size)
{
    if (dest_size == 0) return 0;
    return 0;
}
