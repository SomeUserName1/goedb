#ifndef HEAP_FILE_H
#define HEAP_FILE_H

#include <stdio.h>

#include "node.h"
#include "page_cache.h"
#include "physical_database.h"
#include "relationship.h"

typedef struct
{
    page_cache*   cache;
    unsigned long n_nodes;
    unsigned long n_rels;
    unsigned long last_alloc_node_slot;
    unsigned long last_alloc_rel_slot;
    unsigned long num_reads_nodes;
    unsigned long num_updates_nodes;
    unsigned long num_reads_rels;
    unsigned long num_update_rels;
#ifdef VERBOSE
    FILE* log_file;
#endif
} heap_file;

heap_file*
heap_file_create(page_cache* pc
#ifdef VERBOSE
                 ,
                 const char* log_path
#endif
);

void
heap_file_destroy(heap_file* hf);

unsigned long
next_free_slots(heap_file* hf, bool node);

bool
check_record_exists(heap_file* hf, unsigned long id, bool node);

unsigned long
create_node(heap_file* hf, char* label);

unsigned long
create_relationship(heap_file*    hf,
                    unsigned long from_node_id,
                    unsigned long to_node_id,
                    double        weight,
                    char*         label);

node_t*
read_node(heap_file* hf, unsigned long node_id);

relationship_t*
read_relationship(heap_file* hf, unsigned long rel_id);

void
update_node(heap_file* hf, node_t* node_to_write);

void
update_relationship(heap_file* hf, relationship_t* rel_to_write);

void
delete_node(heap_file* hf, unsigned long node_id);

void
delete_relationship(heap_file* hf, unsigned long rel_id);

void
prepare_move_node(heap_file* hf, unsigned long id, unsigned long to_id);

void
prepare_move_relationship(heap_file* hf, unsigned long id, unsigned long to_id);

void
swap_page(heap_file* hf, size_t fst, size_t snd, file_type ft);

array_list_node*
get_nodes(heap_file* hf);

array_list_relationship*
get_relationships(heap_file* hf);

unsigned long
next_relationship_id(heap_file*      hf,
                     unsigned long   node_id,
                     relationship_t* rel,
                     direction_t     direction);

array_list_relationship*
expand(heap_file* hf, unsigned long node_id, direction_t direction);

relationship_t*
contains_relationship_from_to(heap_file*    hf,
                              unsigned long node_from,
                              unsigned long node_to,
                              direction_t   direction);

#endif
