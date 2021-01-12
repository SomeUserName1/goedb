#include "node.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

node_t* new_node() {
    node_t *node = malloc(sizeof(*node));
    node_clear(node);
    return node;
}

int node_read(node_t* record, const unsigned char* bytes) {
    printf("%lu, %c", record->id, *bytes);
    return 0;
}

int node_write(const node_t* record) {
    printf("%lu", record->id);
    return 0;
}

void node_clear(node_t *record) {
    record->id = UNINITIALIZED_LONG;
    record->flags = UNINITIALIZED_BYTE;
    record->first_relationship = UNINITIALIZED_LONG;
    record->first_property = UNINITIALIZED_LONG;
    record->node_type = UNINITIALIZED_LONG;
}

node_t* node_copy(const node_t* original) {
    node_t* copy = malloc(sizeof(*copy));
    copy->id = original->id;
    copy->flags = original->flags;
    copy->first_relationship = original->first_relationship;
    copy->first_property = original->first_property;
    copy->node_type = original->node_type;

    return copy;
}

bool node_equals(const node_t* first, const node_t* second) {
    return ((first->id == second->id)
            && (first->flags == second->flags)
            && (first->first_relationship == second->first_relationship)
            && (first->first_property == second->first_property)
            && (first->node_type == second->node_type));
}

int node_to_string(const node_t* record, char* buffer, size_t buffer_size) {
   int result = sprintf(buffer, "Node ID: %#lX\n"
                    "In-Use: %#hhX\n"
                    "First Relationship: %#lX\n"
                    "First Property: %#lX\n"
                    "Node Type: %#lX\n",
                    record->id,
                    record->flags,
                    record->first_relationship,
                    record->first_property,
                    record->node_type);

   if (result < 0 || (size_t) result > buffer_size)  {
       printf("Wrote node string representation to a buffer that was too small!");
       return EOVERFLOW;
   }
    return 0;
}

void node_pretty_print(const node_t* record) {
   printf("Node ID: %#lX\n"
                    "In-Use: %#hhX\n"
                    "First Relationship: %#lX\n"
                    "First Property: %#lX\n"
                    "Node Type: %#lX\n",
                    record->id,
                    record->flags,
                    record->first_relationship,
                    record->first_property,
                    record->node_type);
}