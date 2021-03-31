#include "in_memory_file.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../constants.h"
#include "../data-struct/dict_ul.h"
#include "../data-struct/list_node.h"
#include "../data-struct/list_rel.h"
#include "../record/node.h"
#include "../record/relationship.h"

in_memory_file_t*
create_in_memory_file()
{
    in_memory_file_t* file = (in_memory_file_t*)malloc(sizeof(*file));

    if (file == NULL) {
        exit(-1);
    }

    file->cache_nodes = create_dict_ul_node();
    file->cache_rels = create_dict_ul_rel();
    file->node_id_counter = 0;
    file->rel_id_counter = 0;

    return file;
}

void
in_memory_file_destroy(in_memory_file_t* db)
{
    dict_ul_node_destroy(db->cache_nodes);
    dict_ul_rel_destroy(db->cache_rels);
    free(db);
}

unsigned long
in_memory_create_node(in_memory_file_t* db)
{
    node_t* node = new_node();
    node->id = db->node_id_counter++;
    node->flags = 1;

    if (dict_ul_node_insert(db->cache_nodes, node->id, node) < 0) {
        printf("%s", "Inserting the new node failed\n");
        exit(-1);
    }

    return node->id;
}

node_t*
in_memory_get_node(in_memory_file_t* db, unsigned long id)
{
    return dict_ul_node_get_direct(db->cache_nodes, id);
}

list_node_t*
in_memory_get_nodes(in_memory_file_t* db)
{
    list_node_t* nodes = create_list_node();
    unsigned long* id = NULL;
    node_t* node = NULL;
    dict_ul_node_iterator_t* it = create_dict_ul_node_iterator(db->cache_nodes);

    while (dict_ul_node_iterator_next(it, &id, &node) > -1) {
        list_node_append(nodes, node);
    }
    dict_ul_node_iterator_destroy(it);
    return nodes;
}

relationship_t*
in_memory_get_relationship(in_memory_file_t* db, unsigned long id)
{
    return dict_ul_rel_get_direct(db->cache_rels, id);
}

list_relationship_t*
in_memory_get_relationships(in_memory_file_t* db)
{
    list_relationship_t* rels = create_list_relationship();
    unsigned long* id = NULL;
    relationship_t* rel = NULL;
    dict_ul_rel_iterator_t* it = create_dict_ul_rel_iterator(db->cache_rels);

    while (dict_ul_rel_iterator_next(it, &id, &rel) > -1) {
        list_relationship_append(rels, rel);
    }
    dict_ul_rel_iterator_destroy(it);
    return rels;
}

unsigned long
in_memory_create_relationship(in_memory_file_t* db,
                              unsigned long node_from,
                              unsigned long node_to)
{
    return in_memory_create_relationship_weighted(db, node_from, node_to, 1.0F);
}

unsigned long
in_memory_create_relationship_weighted(in_memory_file_t* db,
                                       unsigned long node_from,
                                       unsigned long node_to,
                                       double weight)
{
    unsigned long temp_id;
    relationship_t* last_rel_source = NULL;
    relationship_t* first_rel_source = NULL;
    relationship_t* last_rel_target = NULL;
    relationship_t* first_rel_target = NULL;
    node_t* source_node = NULL;
    node_t* target_node = NULL;

    if (!(source_node = dict_ul_node_get_direct(db->cache_nodes, node_from)) ||
        !(target_node = dict_ul_node_get_direct(db->cache_nodes, node_to))) {
        printf("%s: %lu, %lu\n",
               "One of the nodes which are refered to by the relationship"
               "to create does not exist:",
               node_from,
               node_to);
        exit(-1);
    }

    relationship_t* rel = new_relationship();
    rel->source_node = node_from;
    rel->target_node = node_to;
    rel->id = db->rel_id_counter++;
    rel->flags = 1;
    rel->weight = weight;

    // Find first and last relationship in source node's chain
    if (source_node->first_relationship == UNINITIALIZED_LONG) {
        last_rel_source = rel;
        first_rel_source = rel;
    } else {
        first_rel_source =
              in_memory_get_relationship(db, source_node->first_relationship);

        temp_id = source_node->id == first_rel_source->source_node
                        ? first_rel_source->prev_rel_source
                        : first_rel_source->prev_rel_target;

        last_rel_source = in_memory_get_relationship(db, temp_id);
    }

    // Find first and last relationship in target node's chain
    if (target_node->first_relationship == UNINITIALIZED_LONG) {
        last_rel_target = rel;
        first_rel_target = rel;
    } else {
        first_rel_target =
              in_memory_get_relationship(db, target_node->first_relationship);

        temp_id = target_node->id == first_rel_target->source_node
                        ? first_rel_target->prev_rel_source
                        : first_rel_target->prev_rel_target;

        last_rel_target = in_memory_get_relationship(db, temp_id);
    }

    // The amount of if clauses significantly decreases when we exclude
    // self-loops.
    // We are altering things appart from setting the first and last
    // relationships in chains as doing so meanwhile figuring out the first and
    // last rels, may produce garbage like when changing a collection during
    // iteration

    // Adjust previous and next pointer for source node in new relation
    rel->prev_rel_source = last_rel_source->id;
    rel->next_rel_source = first_rel_source->id;
    // Adjust previous and next pointer for target node in new relation
    rel->prev_rel_target = last_rel_target->id;
    rel->next_rel_target = first_rel_target->id;

    // Adjust next pointer in source node's previous relation
    if (last_rel_source->source_node == node_from) {
        last_rel_source->next_rel_source = rel->id;
    }
    if (last_rel_source->target_node == node_from) {
        last_rel_source->next_rel_target = rel->id;
    }
    // Adjust next pointer in target node's previous relation
    if (last_rel_target->source_node == node_to) {
        last_rel_target->next_rel_source = rel->id;
    }
    if (last_rel_target->target_node == node_to) {
        last_rel_target->next_rel_target = rel->id;
    }

    // Adjust previous pointer in source node's next relation
    if (first_rel_source->source_node == node_from) {
        first_rel_source->prev_rel_source = rel->id;
    }
    if (first_rel_source->target_node == node_from) {
        first_rel_source->prev_rel_target = rel->id;
    }
    // Adjust previous pointer in target node's next relation
    if (first_rel_target->source_node == node_to) {
        first_rel_target->prev_rel_source = rel->id;
    }
    if (first_rel_target->target_node == node_to) {
        first_rel_target->prev_rel_target = rel->id;
    }

    // Set the first relationship pointer, if the inserted rel is the first rel
    if (source_node->first_relationship == UNINITIALIZED_LONG) {
        relationship_set_first_target(rel);
        source_node->first_relationship = rel->id;
    }
    if (target_node->first_relationship == UNINITIALIZED_LONG) {
        relationship_set_first_source(rel);
        target_node->first_relationship = rel->id;
    }

    dict_ul_rel_insert(db->cache_rels, rel->id, rel);

    return rel->id;
}

unsigned long
in_memory_next_relationship(in_memory_file_t* db,
                            unsigned long node_id,
                            relationship_t* rel,
                            direction_t direction)
{
    unsigned long start_rel_id = rel->id;
    unsigned long rel_id = node_id == rel->source_node ? rel->next_rel_source
                                                       : rel->next_rel_target;

    if (db == NULL || rel == NULL || node_id == UNINITIALIZED_LONG) {
        printf("Arguments must be not NULL!");
        exit(-1);
    }

    do {
        rel = dict_ul_rel_get_direct(db->cache_rels, rel_id);

        if (rel_id != start_rel_id &&
            ((rel->source_node == node_id && direction != INCOMING) ||
             (rel->target_node == node_id && direction != OUTGOING))) {
            return rel->id;
        }
        rel_id = node_id == rel->source_node ? rel->next_rel_source
                                             : rel->next_rel_target;

    } while (rel_id != start_rel_id);

    return UNINITIALIZED_LONG;
}

list_relationship_t*
in_memory_expand(in_memory_file_t* db,
                 unsigned long node_id,
                 direction_t direction)
{
    node_t* node = dict_ul_node_get_direct(db->cache_nodes, node_id);
    if (node == NULL || db == NULL) {
        printf("Arguments must be not NULL!");
        exit(-1);
    }
    list_relationship_t* result = create_list_relationship();
    relationship_t* rel = NULL;
    unsigned long rel_id = node->first_relationship;
    unsigned long start_id = rel_id;

    do {
        rel = dict_ul_rel_get_direct(db->cache_rels, rel_id);
        list_relationship_append(result, rel);
        rel_id = in_memory_next_relationship(db, node->id, rel, direction);
    } while (rel_id != start_id && rel_id != UNINITIALIZED_LONG);

    return result;
}

relationship_t*
in_memory_contains_relationship_from_to(in_memory_file_t* db,
                                        unsigned long node_from,
                                        unsigned long node_to,
                                        direction_t direction)
{
    relationship_t* rel;
    node_t* source_node = dict_ul_node_get_direct(db->cache_nodes, node_from);
    node_t* target_node = dict_ul_node_get_direct(db->cache_nodes, node_to);

    if (source_node->first_relationship == UNINITIALIZED_LONG ||
        target_node->first_relationship == UNINITIALIZED_LONG) {
        return NULL;
    }

    unsigned long next_id = source_node->first_relationship;
    unsigned long start_id = next_id;

    do {
        rel = dict_ul_rel_get_direct(db->cache_rels, next_id);
        if ((direction != INCOMING && rel->source_node == node_from &&
             rel->target_node == node_to) ||
            (direction != OUTGOING && rel->source_node == node_to &&
             rel->target_node == node_from)) {
            return rel;
        }
        next_id = in_memory_next_relationship(db, node_from, rel, direction);
    } while (next_id != start_id && next_id != UNINITIALIZED_LONG);

    return NULL;
}
