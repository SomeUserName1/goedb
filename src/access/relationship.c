#include "access/relationship.h"

#include <stdio.h>
#include <stdlib.h>

#include "../constants.h"

relationship_t*
new_relationship()
{
    relationship_t* rel = malloc(sizeof(*rel));

    if (!rel) {
        exit(-1);
    }

    relationship_clear(rel);
    return rel;
}

int
relationship_read(relationship_t* record, const unsigned char* bytes)
{
    printf("%lu, %c", record->id, *bytes);
    return 0;
}

int
relationship_write(const relationship_t* record)
{
    printf("%lu", record->id);
    return 0;
}

void
relationship_clear(relationship_t* record)
{
    record->id              = UNINITIALIZED_LONG;
    record->flags           = UNINITIALIZED_BYTE;
    record->source_node     = UNINITIALIZED_LONG;
    record->target_node     = UNINITIALIZED_LONG;
    record->prev_rel_source = UNINITIALIZED_LONG;
    record->next_rel_source = UNINITIALIZED_LONG;
    record->prev_rel_target = UNINITIALIZED_LONG;
    record->next_rel_target = UNINITIALIZED_LONG;
    record->weight          = UNINITIALIZED_WEIGHT;
}

relationship_t*
relationship_copy(const relationship_t* original)
{
    relationship_t* copy = malloc(sizeof(*copy));

    if (copy == NULL) {
        return NULL;
    }

    copy->id              = original->id;
    copy->flags           = original->flags;
    copy->source_node     = original->source_node;
    copy->target_node     = original->target_node;
    copy->prev_rel_source = original->prev_rel_source;
    copy->next_rel_source = original->next_rel_source;
    copy->prev_rel_target = original->prev_rel_target;
    copy->next_rel_target = original->next_rel_target;
    copy->weight          = original->weight;

    return copy;
}

bool
relationship_equals(const relationship_t* first, const relationship_t* second)
{
    return ((first->id == second->id) && (first->flags == second->flags)
            && (first->source_node == second->source_node)
            && (first->target_node == second->target_node)
            && (first->prev_rel_source == second->prev_rel_source)
            && (first->next_rel_source == second->next_rel_source)
            && (first->prev_rel_target == second->prev_rel_target)
            && (first->weight == second->weight));
}

int
relationship_to_string(const relationship_t* record,
                       char*                 buffer,
                       size_t                buffer_size)
{
    int length = snprintf(NULL,
                          0,
                          "Relationship ID: %#lX\n"
                          "Flags: %#hhX\n"
                          "Source Node: %#lX\n"
                          "Target Node: %#lX\n"
                          "Source node's previous relationship: %#lX\n"
                          "Source node's next relationship: %#lX\n"
                          "Target node's previous relationship: %#lX\n"
                          "Target node's next relationship: %#lX\n"
                          "First Property: %.1f\n",
                          record->id,
                          record->flags,
                          record->source_node,
                          record->target_node,
                          record->prev_rel_source,
                          record->next_rel_source,
                          record->prev_rel_target,
                          record->next_rel_target,
                          record->weight);

    if (length < 0 || (size_t)length > buffer_size) {
        printf("Wrote relationship string representation to a buffer that was "
               "too small!\n");
        printf("String length: %lu, Buffer size, %lu\n",
               (size_t)length,
               buffer_size);

        return -1;
    }

    int result = snprintf(buffer,
                          length,
                          "Relationship ID: %#lX\n"
                          "Flags: %#hhX\n"
                          "Source Node: %#lX\n"
                          "Target Node: %#lX\n"
                          "Source node's previous relationship: %#lX\n"
                          "Source node's next relationship: %#lX\n"
                          "Target node's previous relationship: %#lX\n"
                          "Target node's next relationship: %#lX\n"
                          "First Property: %.1f\n",
                          record->id,
                          record->flags,
                          record->source_node,
                          record->target_node,
                          record->prev_rel_source,
                          record->next_rel_source,
                          record->prev_rel_target,
                          record->next_rel_target,
                          record->weight);

    return result > 0 ? 0 : result;
}

void
relationship_pretty_print(const relationship_t* record)
{
    printf("Relationship ID: %#lX\n"
           "Flags: %#hhX\n"
           "Source Node: %#lX\n"
           "Target Node: %#lX\n"
           "Source node's previous relationship: %#lX\n"
           "Source node's next relationship: %#lX\n"
           "Target node's previous relationship: %#lX\n"
           "Target node's next relationship: %#lX\n"
           "Weight: %.1f\n",
           record->id,
           record->flags,
           record->source_node,
           record->target_node,
           record->prev_rel_source,
           record->next_rel_source,
           record->prev_rel_target,
           record->next_rel_target,
           record->weight);
}

void
relationship_set_first_source(relationship_t* rel)
{
    if (!rel) {
        exit(-1);
    }

    if (rel->flags == UNINITIALIZED_BYTE) {
        rel->flags = FIRST_REL_SOURCE_FLAG;
    } else {
        rel->flags = rel->flags | FIRST_REL_SOURCE_FLAG;
    }
}

void
relationship_set_first_target(relationship_t* rel)
{
    if (!rel) {
        exit(-1);
    }

    if (rel->flags == UNINITIALIZED_BYTE) {
        rel->flags = FIRST_REL_TARGET_FLAG;
    } else {
        rel->flags = rel->flags | FIRST_REL_TARGET_FLAG;
    }
}