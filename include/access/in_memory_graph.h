#ifndef IN_MEMORY_GRAPH_H
#define IN_MEMORY_GRAPH_H

#include "access/node.h"
#include "access/relationship.h"

typedef struct
{
    dict_ul_node* cache_nodes;
    dict_ul_rel*  cache_rels;
    unsigned long n_nodes;
    unsigned long n_rels;
} in_memory_graph;

in_memory_graph*
in_memory_graph_create(void);

void
in_memory_graph_destroy(in_memory_graph* db);

unsigned long
in_memory_create_node(in_memory_graph* db, char* label);

unsigned long
in_memory_create_relationship(in_memory_graph* db,
                              unsigned long    node_from,
                              unsigned long    node_to,
                              char*            label);
unsigned long
in_memory_create_relationship_weighted(in_memory_graph* db,
                                       unsigned long    node_from,
                                       unsigned long    node_to,
                                       double           weight,
                                       char*            label);

node_t*
in_memory_get_node(in_memory_graph* db, unsigned long id);

relationship_t*
in_memory_get_relationship(in_memory_graph* db, unsigned long id);

inm_alist_node*
in_memory_get_nodes(in_memory_graph* db);

inm_alist_relationship*
in_memory_get_relationships(in_memory_graph* db);

unsigned long
in_memory_next_relationship_id(in_memory_graph* db,
                               unsigned long    node_id,
                               relationship_t*  rel,
                               direction_t      direction);
inm_alist_relationship*
in_memory_expand(in_memory_graph* db,
                 unsigned long    node_id,
                 direction_t      direction);
relationship_t*
in_memory_contains_relationship_from_to(in_memory_graph* db,
                                        unsigned long    node_from,
                                        unsigned long    node_to,
                                        direction_t      direction);

#endif
