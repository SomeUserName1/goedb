#include "data-struct/bitmap.h"
#include "data-struct/htable.h"
#include "disk_file.h"
#include "page.h"
#include "page_cache.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "data-struct/linked_list.h"
#include "physical_database.h"

static const size_t NUM_TEST_PAGES = 42;

void
test_page_cache_create(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );
    assert(pc);
    assert(pc->pdb == pdb);
    assert(pc->num_pins == 0);
    assert(pc->num_unpins == 0);
    assert(llist_ul_size(pc->free_frames) == CACHE_N_PAGES);
    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        assert(get_bit(pc->pinned, i) == 0);
    }
    assert(queue_ul_size(pc->recently_referenced) == 0);

    for (size_t i = 0; i < invalid; ++i) {
        assert(dict_ul_ul_size(pc->page_map[i]) == 0);
        dict_ul_ul_destroy(pc->page_map[i]);
    }

    assert(pc->cache);

#ifdef VERBOSE
    assert(pc->log_file);

    if (fclose(pc->log_file) != 0) {
        printf("page cache test - create: failed to close log file! %s\n",
               strerror(errno));
    }
#endif

    bitmap_destroy(pc->pinned);
    llist_ul_destroy(pc->free_frames);
    queue_ul_destroy(pc->recently_referenced);
    free(pc->cache[0]->data);
    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        page_destroy(pc->cache[i]);
    }
    free(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - create successful!\n");
}

void
test_page_cache_destroy(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    page_cache_destroy(pc);

    assert(pdb);
    phy_database_delete(pdb);

    printf("Test Page Cache - destroy successful!\n");
}

void
test_new_page(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    page* n_page = new_page(pc, node_file);

    assert(n_page);
    assert(n_page->page_no == pc->pdb->files[node_file]->num_pages - 1);
    assert(n_page->dirty == false);
    assert(n_page->ft == node_file);
    assert(n_page->pin_count == 1);

    n_page->pin_count = 0;
    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - new page successful!\n");
}

void
test_pin_page(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_file, NUM_TEST_PAGES);

    page* test_page_1 = pin_page(pc, 0, node_file);
    page* test_page_2 = pin_page(pc, NUM_TEST_PAGES - 1, node_file);

    size_t frame_no_1 = dict_ul_ul_get_direct(pc->page_map[node_file], 0);
    size_t frame_no_2 =
          dict_ul_ul_get_direct(pc->page_map[node_file], NUM_TEST_PAGES - 1);

    assert(pc->num_pins == 2);
    assert(pc->num_unpins == 0);
    assert(llist_ul_size(pc->free_frames) == CACHE_N_PAGES - 2);
    assert(queue_ul_size(pc->recently_referenced) == 0);

    assert(test_page_1);
    assert(pc->cache[frame_no_1] == test_page_1);
    assert(get_bit(pc->pinned, frame_no_1));
    assert(test_page_1->page_no == 0);
    assert(test_page_1->dirty == false);
    assert(test_page_1->ft == node_file);
    assert(test_page_1->pin_count == 1);

    assert(test_page_2);
    assert(pc->cache[frame_no_2] == test_page_2);
    assert(get_bit(pc->pinned, frame_no_2));
    assert(test_page_2->page_no == NUM_TEST_PAGES - 1);
    assert(test_page_2->dirty == false);
    assert(test_page_2->ft == node_file);
    assert(test_page_2->pin_count == 1);

    page* test_page_3 = pin_page(pc, 0, node_file);
    assert(test_page_3);
    assert(test_page_3 == test_page_1);
    assert(test_page_3->pin_count == 2);
    assert(pc->num_pins == 3);
    assert(llist_ul_size(pc->free_frames) == CACHE_N_PAGES - 2);

    page* test_page_4 = pin_page(pc, 0, node_file);
    assert(test_page_4);
    assert(test_page_4 == test_page_1);
    assert(test_page_4->pin_count == 3);
    assert(pc->num_pins == 4);
    assert(llist_ul_size(pc->free_frames) == CACHE_N_PAGES - 2);

    test_page_1->pin_count = 0;
    test_page_2->pin_count = 0;
    test_page_3->pin_count = 0;
    test_page_4->pin_count = 0;
    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - pin page successful!\n");
}

void
test_unpin_page(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_file, NUM_TEST_PAGES);

    page* test_page_1 = pin_page(pc, 0, node_file);
    page* test_page_2 = pin_page(pc, 0, node_file);

    size_t frame_no_1 = dict_ul_ul_get_direct(pc->page_map[node_file], 0);

    assert(pc->num_pins == 2);
    assert(pc->num_unpins == 0);
    assert(llist_ul_size(pc->free_frames) == CACHE_N_PAGES - 1);
    assert(queue_ul_size(pc->recently_referenced) == 0);
    assert(test_page_1);
    assert(pc->cache[frame_no_1] == test_page_1);
    assert(test_page_1 == test_page_2);
    assert(get_bit(pc->pinned, frame_no_1));
    assert(test_page_1->page_no == 0);
    assert(test_page_1->dirty == false);
    assert(test_page_1->ft == node_file);
    assert(test_page_1->pin_count == 2);

    unpin_page(pc, 0, node_file);

    assert(test_page_1->pin_count == 1);
    assert(pc->num_unpins == 1);
    assert(pc->num_pins == 2);
    assert(queue_ul_size(pc->recently_referenced) == 1);
    assert(queue_ul_get(pc->recently_referenced, 0) == frame_no_1);
    assert(get_bit(pc->pinned, frame_no_1) == 1);

    unpin_page(pc, 0, node_file);
    assert(pc->num_unpins == 2);
    assert(test_page_1->pin_count == 0);
    assert(queue_ul_size(pc->recently_referenced) == 1);
    assert(queue_ul_get(pc->recently_referenced, 0) == frame_no_1);
    assert(get_bit(pc->pinned, frame_no_1) == 0);

    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - unpin page successful!\n");
}

void
test_evict_page(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_file, CACHE_N_PAGES + 1);

    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        pin_page(pc, i, node_file);
    }

    assert(llist_ul_size(pc->free_frames) == 0);

    size_t frame_nos[EVICT_LRU_K];
    for (size_t i = 0; i < EVICT_LRU_K; ++i) {
        frame_nos[i] = dict_ul_ul_get_direct(pc->page_map[node_file], i);
        unpin_page(pc, i, node_file);
    }

    assert(llist_ul_size(pc->free_frames) == 0);

    size_t reads_before = pdb->files[node_file]->read_count;
    pin_page(pc, 0, node_file);
    assert(reads_before == pdb->files[node_file]->read_count);
    assert(llist_ul_size(pc->free_frames) == 0);
    unpin_page(pc, 0, node_file);

    evict_page(pc);

    assert(llist_ul_size(pc->free_frames) == EVICT_LRU_K);

    for (size_t i = 0; i < EVICT_LRU_K; ++i) {
        assert(!dict_ul_ul_contains(pc->page_map[node_file], i));
        assert(!queue_ul_contains(pc->recently_referenced, frame_nos[i]));
    }

    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        pc->cache[i]->pin_count = 0;
        pc->cache[i]->dirty     = false;
    }

    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - evict successful!\n");
}

void
test_flush_page(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_file, CACHE_N_PAGES + 1);

    page* p = pin_page(pc, 0, node_file);
    write_ulong(p, 0, 1);

    unsigned long writes_before = pdb->files[node_file]->write_count;

    flush_page(pc, dict_ul_ul_get_direct(pc->page_map[node_file], 0));

    assert(writes_before + 1 == pdb->files[node_file]->write_count);

    unsigned char buf[PAGE_SIZE];
    read_page(pdb->files[node_file], 0, buf);
    unsigned long content;
    memcpy(&content, buf, sizeof(unsigned long));

    assert(content == 1);
    assert(!p->dirty);

    unpin_page(pc, 0, node_file);
    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - flush successful!\n");
}

void
test_flush_all_pages(void)
{
    char* file_name = "test";

#ifdef VERBOSE
    char* log_name_pdb   = "log_test_pdb";
    char* log_name_cache = "log_test_cache";
#endif

    phy_database* pdb = phy_database_create(file_name
#ifdef VERBOSE
                                            ,
                                            log_name_pdb
#endif
    );
    page_cache* pc = page_cache_create(pdb
#ifdef VERBOSE
                                       ,
                                       log_name_cache
#endif
    );

    allocate_pages(pc->pdb, node_file, CACHE_N_PAGES);

    page* p;
    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        p = pin_page(pc, i, node_file);
        write_ulong(p, 0, 1);
    }

    flush_all_pages(pc);

    printf("finished flushing\n");

    unsigned char buf[PAGE_SIZE];
    unsigned long content;
    for (size_t i = 0; i < CACHE_N_PAGES; ++i) {
        read_page(pdb->files[node_file], i, buf);
        content = 0;
        memcpy(&content, buf, sizeof(unsigned long));
        assert(content == 1);
        pc->cache[i]->pin_count = 0;
        pc->cache[i]->dirty     = false;
    }

    page_cache_destroy(pc);
    phy_database_delete(pdb);

    printf("Test Page Cache - flush all successful!\n");
}

int
main(void)
{
    test_page_cache_create();
    test_page_cache_destroy();
    test_new_page();
    test_pin_page();
    test_unpin_page();
    test_evict_page();
    test_flush_page();
    test_flush_all_pages();

    return 0;
}
