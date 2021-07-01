#include "disk_file.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"

#define PiB_OFFSET 50

disk_file*
disk_file_create(char* file_name)
{
    if (!file_name) {
        printf("disk file - create: Invalid arguments!\n");
        exit(EXIT_FAILURE);
    }

    disk_file* df = calloc(1, sizeof(disk_file));

    if (!df) {
        printf("disk file - create: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    df->file_name = file_name;
    df->file      = fopen(file_name, "ab+");

    if (df->file == NULL) {
        printf("disk file - create: failed to fopen %s: %s\n",
               file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    df->f_buf = calloc(PAGE_SIZE << 3, sizeof(char));

    if (!df->f_buf) {
        printf("disk file - create: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    if (setvbuf(df->file, df->f_buf, _IOFBF, PAGE_SIZE << 3) != 0) {
        printf("disk file - create: failed to set the internal buffer of file "
               "%s: %s\n",
               file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fseek(df->file, 0, SEEK_END) == -1) {
        printf("disk file - create: failed to fseek %s: %s\n",
               file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    df->file_size = ftell(df->file);

    if (df->file_size == -1) {
        printf("disk file - create: failed to ftell %s: %s\n",
               file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    df->num_pages = df->file_size / PAGE_SIZE;

    return df;
}

void
disk_file_destroy(disk_file* df)
{
    if (!df) {
        printf("disk file - destroy: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (fclose(df->file) != 0) {
        printf("disk file - destroy: Error closing file: %s", strerror(errno));
    }

    free(df->f_buf);
    free(df);
}

void
disk_file_delete(disk_file* df)
{
    if (!df) {
        printf("disk file - delete: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (fclose(df->file) != 0) {
        printf("disk file - delete: Error closing file: %s", strerror(errno));
    }

    if (remove(df->file_name) != 0) {
        printf("disk file - delete: Error removing file: %s\n",
               strerror(errno));
    }
    free(df->f_buf);
}

void
read_page(disk_file* df, size_t page_no, unsigned char* buf)
{
    read_pages(df, page_no, page_no, buf);
}

void
read_pages(disk_file* df, size_t fst_page, size_t lst_page, unsigned char* buf)
{
    if (!df || !buf) {
        printf("disk file - read pages: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (fst_page > MAX_PAGE_NO || lst_page > MAX_PAGE_NO
        || fst_page > df->num_pages || lst_page > df->num_pages) {
        printf("disk file - read pages: One of the page numbers you specified "
               "(%lu or %lu) are "
               "too large!\n "
               "The current size of the database is %lu pages and %li bytes.\n"
               "The size limit of the database is currently %li PiB, that is "
               "the maximal page number is %li",
               fst_page,
               lst_page,
               df->num_pages,
               df->file_size,
               LONG_MAX >> PiB_OFFSET,
               MAX_PAGE_NO);
        exit(EXIT_FAILURE);
    }

    if (fst_page > lst_page) {
        printf("disk file - read pages: The number of the last page to be read "
               "needs to be larger than the number of the first page to be "
               "read!\n");
        exit(EXIT_FAILURE);
    }

    long offset = (long)(PAGE_SIZE * fst_page);

    size_t num_pages_read = 1 + ((lst_page - fst_page) / PAGE_SIZE);

    if (fseek(df->file, offset, SEEK_SET) == -1) {
        printf("disk file - read pages: failed to fseek %s: %s\n",
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (fread(buf, PAGE_SIZE, num_pages_read, df->file)
        != PAGE_SIZE * num_pages_read) {
        printf("disk file - read pages: Failed to read the pages from %lu to "
               "%lu from file %s: %s\n",
               fst_page,
               lst_page,
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void
write_page(disk_file* df, size_t page_no, unsigned char* data)
{
    write_pages(df, page_no, page_no, data);
}

void
write_pages(disk_file*     df,
            size_t         fst_page,
            size_t         lst_page,
            unsigned char* data)
{
    if (!df || !data) {
        printf("disk file - write page: Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (fst_page > MAX_PAGE_NO || lst_page > MAX_PAGE_NO) {
        printf("disk file - write page: One of the page numbers you specified "
               "(%lu or %lu) are "
               "too large!\n "
               "The size limit of the database is currently %li PiB, that is "
               "the maximal page number is %li",
               fst_page,
               lst_page,
               LONG_MAX >> PiB_OFFSET,
               MAX_PAGE_NO);
        exit(EXIT_FAILURE);
    }

    if (fst_page > lst_page) {
        printf(
              "disk file - write pages: The number of the last page to be read "
              "needs to be larger than the number of the first page to be "
              "read!\n");
        exit(EXIT_FAILURE);
    }

    if (lst_page + 1 > df->num_pages) {
        // +1 as the page indexes start at 0 while the number of pages starts
        // counting at 1
        size_t grow_by = lst_page - df->num_pages + 1;
        disk_file_grow(df, grow_by);
    }

    long offset = (long)(PAGE_SIZE * fst_page);

    size_t num_pages_write = 1 + ((lst_page - fst_page) / PAGE_SIZE);

    if (fseek(df->file, offset, SEEK_SET) == -1) {
        printf("disk file - write page: failed to fseek %s: %s\n",
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fwrite(data, PAGE_SIZE, num_pages_write, df->file)
        != PAGE_SIZE * num_pages_write) {
        printf("disk file - write pages: Failed to write the pages from %lu to "
               "%lu from file %s: %s\n",
               fst_page,
               lst_page,
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void
clear_page(disk_file* df, size_t page_no)
{
    unsigned char* data = calloc(1, PAGE_SIZE);

    if (!data) {
        printf("disk page - clear page: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    write_page(df, page_no, data);
    free(data);
}

void
disk_file_grow(disk_file* df, size_t by_num_pages)
{
    if (!df) {
        printf("disk file - grow: invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (df->num_pages + by_num_pages >= MAX_PAGE_NO) {
        printf("disk file - grow: Cannot grow database by %lu pages! Exceeds "
               "max database size "
               "of %li PiB!\n",
               by_num_pages,
               LONG_MAX >> PiB_OFFSET);
        exit(EXIT_FAILURE);
    }

    if (fseek(df->file, 0, SEEK_END) == -1) {
        printf("disk file - grow: failed to fseek with errno %d\n", errno);
        exit(EXIT_FAILURE);
    }

    char* data = calloc(1, PAGE_SIZE);

    if (!data) {
        printf("disk page - grow: Failed to allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    if (fwrite(data, PAGE_SIZE, by_num_pages, df->file)
        != PAGE_SIZE * by_num_pages) {
        printf("disk file - grow pages: Failed to grow file %s: %s\n",
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }

    free(data);

    df->file_size = ftell(df->file);

    if (df->file_size == -1) {
        printf("disk file - grow: failed to ftell file %s: %s\n",
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/**
 * DANGER ZONE!
 * This method assumes that the empty pages have been moved to the end.
 * It will simply shrink the file by cutting of the last by_no_pages pages.
 * If these are not empty, the records on these pages will be lost!
 */
void
disk_file_shrink(disk_file* df, size_t by_num_pages)
{
    if (!df || by_num_pages > MAX_PAGE_NO) {
        printf("disk file - shrink: invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (df->num_pages - by_num_pages >= 0) {
        printf("disk file - shrink: Cannot shrink database by %lu pages! The "
               "database would have "
               "less than 0 pages!\n",
               by_num_pages);
        exit(EXIT_FAILURE);
    }

    int fd = fileno(df->file);

    if (fd == -1) {
        printf("disk file - shrink: Failed to get file descriptor from stream "
               "of file %s: %s\n",
               df->file_name,
               strerror(errno));
    }

    long shrink_by_bytes = PAGE_SIZE * by_num_pages;

    if (ftruncate(fd, df->file_size - shrink_by_bytes) == -1) {
        printf("disk file - shrink: Failed to truncate the file %s: %s\n",
               df->file_name,
               strerror(errno));
        exit(EXIT_FAILURE);
    }
}
