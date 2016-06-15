#include <stddef.h>
#include <string.h>

#include "chibi.h"
#include "dos_compat.h"

CHIBI_TEST(TestPathPartNull)
{
    chibi_assert(dc_PathPart(NULL) == NULL);
}

CHIBI_TEST(TestPathPartEmpty)
{
    static char path[] = "";
    chibi_assert(dc_PathPart(path) == path);
}

CHIBI_TEST(TestPathPartVolumeOnly)
{
    static char path[] = "Workbench:";
    chibi_assert(dc_PathPart(path) == path);
}

CHIBI_TEST(TestPathPartVolumeAndFile)
{
    static char path[] = "Workbench:file1";
    chibi_assert(dc_PathPart(path) == &(path[10]));
}

CHIBI_TEST(TestPathPartVolumeDirAndFile)
{
    static char path[] = "Workbench:dir/file1";
    chibi_assert(dc_PathPart(path) == &(path[13]));
}

CHIBI_TEST(TestFilePartNull)
{
    chibi_assert(dc_FilePart(NULL) == NULL);
}

CHIBI_TEST(TestFilePartEmpty)
{
    static char path[] = "";
    chibi_assert(dc_FilePart(path) == path);
}

CHIBI_TEST(TestFilePartVolumeOnly)
{
    static char path[] = "Workbench:";
    chibi_assert(dc_FilePart(path) == path);
}

CHIBI_TEST(TestFilePartVolumeAndFile)
{
    static char path[] = "Workbench:file1";
    chibi_assert(dc_FilePart(path) == &(path[10]));
}

CHIBI_TEST(TestFilePartVolumeDirAndFile)
{
    static char path[] = "Workbench:dir/file1";
    chibi_assert(dc_FilePart(path) == &(path[14]));
}

CHIBI_TEST(TestAddPartAllNull)
{
    chibi_assert(dc_AddPart(NULL, NULL, 0) == 0);
}

int main(int argc, char **argv)
{
    chibi_summary_data summary;
    chibi_suite *suite = chibi_suite_new();

    chibi_suite_add_test(suite, TestPathPartNull);
    chibi_suite_add_test(suite, TestPathPartEmpty);
    chibi_suite_add_test(suite, TestPathPartVolumeOnly);
    chibi_suite_add_test(suite, TestPathPartVolumeAndFile);
    chibi_suite_add_test(suite, TestPathPartVolumeDirAndFile);

    chibi_suite_add_test(suite, TestFilePartNull);
    chibi_suite_add_test(suite, TestFilePartEmpty);
    chibi_suite_add_test(suite, TestFilePartVolumeOnly);
    chibi_suite_add_test(suite, TestFilePartVolumeAndFile);
    chibi_suite_add_test(suite, TestFilePartVolumeDirAndFile);

    chibi_suite_add_test(suite, TestAddPartAllNull);

    chibi_suite_run(suite, &summary);
    chibi_suite_delete(suite);
    return summary.num_failures;
}
