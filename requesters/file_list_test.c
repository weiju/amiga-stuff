#include <string.h>

#include "chibi.h"
#include "file_list.h"

CHIBI_TEST(TestMakeEntry)
{
    struct FileListEntry *entry = new_file_list_entry();
    chibi_assert_not_null(entry);
    chibi_assert(entry->next == NULL);
    chibi_assert(entry->prev == NULL);
    free_file_list(entry);
}

CHIBI_TEST(TestSortListNull)
{
    chibi_assert(sort_file_list(NULL, TRUE) == NULL);
    chibi_assert(sort_file_list(NULL, FALSE) == NULL);
}

CHIBI_TEST(TestSortListSingle)
{
    struct FileListEntry *entry = new_file_list_entry();
    chibi_assert(sort_file_list(entry, TRUE) == entry);
    chibi_assert(sort_file_list(entry, FALSE) == entry);
    chibi_assert_eq_int(entry->index, 0);
    free_file_list(entry);
}

CHIBI_TEST(TestSortListTwoElemsAsc)
{
    struct FileListEntry *entry1 = new_file_list_entry();
    struct FileListEntry *entry2 = new_file_list_entry();
    strcpy(entry2->name, "abc");
    strcpy(entry1->name, "bcd");
    entry1->next = entry2;
    entry2->prev = entry1;

    chibi_assert(sort_file_list(entry1, TRUE) == entry2);
    chibi_assert(entry2->prev == NULL);
    chibi_assert(entry1->next == NULL);
    chibi_assert_eq_int(0, entry2->index);
    chibi_assert_eq_int(1, entry1->index);
    free_file_list(entry2);
}

CHIBI_TEST(TestSortListTwoElemsDesc)
{
    struct FileListEntry *entry1 = new_file_list_entry();
    struct FileListEntry *entry2 = new_file_list_entry();
    strcpy(entry2->name, "abc");
    strcpy(entry1->name, "bcd");
    entry1->next = entry2;
    entry2->prev = entry1;

    chibi_assert(sort_file_list(entry1, FALSE) == entry1);
    chibi_assert(entry1->prev == NULL);
    chibi_assert(entry2->next == NULL);
    chibi_assert_eq_int(0, entry1->index);
    chibi_assert_eq_int(1, entry2->index);
    free_file_list(entry1);
}

int main(int argc, char **argv)
{
    chibi_summary_data summary;
    chibi_suite *suite = chibi_suite_new();
    chibi_suite_add_test(suite, TestMakeEntry);
    chibi_suite_add_test(suite, TestSortListNull);
    chibi_suite_add_test(suite, TestSortListSingle);
    chibi_suite_add_test(suite, TestSortListTwoElemsAsc);
    chibi_suite_add_test(suite, TestSortListTwoElemsDesc);

    chibi_suite_run(suite, &summary);

    chibi_suite_delete(suite);
    return summary.num_failures;
}
