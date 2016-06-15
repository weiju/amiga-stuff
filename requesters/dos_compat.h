#pragma once
#ifndef __DOS_COMPAT_H__
#define __DOS_COMPAT_H__

// Compatibility functions to implement 2.x functionality on 1.x systems

extern const char *dc_PathPart(const char *path);
extern const char *dc_FilePart(const char *path);

extern int dc_AddPart(char *dest_path, const char *part_to_add, int dest_size);

#endif /* __DOS_COMPAT_H__ */

