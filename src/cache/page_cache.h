#ifndef PAGE_CACHE_H
#define PAGE_CACHE_H

#include <stddef.h>

#include "data-struct/bitmap.h"
#include "data-struct/htable.h"
#include "data-struct/linked_list.h"
#include "page.h"
#include "physical_database.h"

typedef struct
{
    phy_database* pdb;
    size_t        total_pinned;
    size_t        total_unpinned;
    FILE*         log_file;
    llist_ul*     free_frames;
    bitmap*       pinned;
    queue_ul*     recently_referenced;
    dict_ul_ul*   page_map[4]; /* Page M is stored in frame N */
    page*         cache[];     /* Frame N contains page M */
} page_cache;

page_cache*
page_cache_create(phy_database* pdb
#ifdef VERBOSE
                  ,
                  const char* log_path
#endif
);

void
page_cache_destroy(page_cache* pc);

unsigned long
new_page(page_cache* pc, file_type ft);

page*
pin_page(page_cache* pc, size_t page_no, file_type ft);

void
unpin_page(page_cache* pc, size_t page_no, file_type ft);

size_t
evict_page(page_cache* pc);

void
flush_page(page_cache* pc, size_t frame_no);

void
flush_all_pages(page_cache* pc);

#endif
