#ifndef FIBONACCI_HEAP
#define FIBONACCI_HEAP

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static const double log_golden_ratio_factor = 2.1;

typedef struct fib_node
{
    double           key;
    unsigned long    value;
    struct fib_node* parent;
    struct fib_node* child;
    struct fib_node* left;
    struct fib_node* right;
    unsigned int     degree;
    bool             mark;
} fib_node;

typedef struct fib_heap
{
    fib_node*     min;
    unsigned long num_nodes;
} fib_heap_t;

fib_node*
create_fib_node(double key, unsigned long value)
{
    if (key == -DBL_MAX) {
        printf("A key of value DBL_MAX is not allowed!"
               "Please use somthing greater than"
               "-DBL_MAX as smallest priority!");
        exit(-1);
    }

    fib_node* node = malloc(sizeof(*node));

    if (!node) {
        printf("fibonacci heap - create_fib_node: Memory Allocation failed!\n");
        exit(-1);
    }

    node->key    = key;
    node->value  = value;
    node->parent = NULL;
    node->child  = NULL;
    node->left   = NULL;
    node->right  = NULL;
    node->degree = 0;
    node->mark   = false;

    return node;
}

fib_heap_t*
create_fib_heap(void)
{
    fib_heap_t* heap = malloc(sizeof(*heap));

    if (!heap) {
        exit(-1);
    }

    heap->min       = NULL;
    heap->num_nodes = 0;

    return heap;
}

void
fib_heap_destroy(fib_heap_t* fh)
{
    if (fh->min) {
        fib_node* node;
        fib_node* next = fh->min->right;

        /* iterate over all trees */
        for (size_t i = 0; i < fh->num_nodes; ++i) {
            node = next;

            /* descend until the leaf */
            while (node->degree > 0) {
                next = node->child;
                node = next;
            }

            // In a leaf, choose a sibling as next node to delete
            if (node->right != node) {
                next              = node->right;
                node->right->left = node->left;
                node->left->right = node->right;
            } else if (node->parent) {
                // If there are no siblings in the leaf, check if the leaf has
                // parents
                next = node->parent;
            } else {
                // if a leaf (no children) has neither siblings nor parents it's
                // the last node to delete. This else statement is just to make
                // static analysis happy, without it the free outside of this
                // else block would be executed and the loop would terminate
                // anyway (remember, last node).
                free(node);
                break;
            }

            if (node->parent) {
                node->parent->degree--;
            }
            free(node);
        }
    }
    free(fh);
}

void
fib_heap_insert(fib_heap_t* fh, fib_node* node)
{
    if (!fh || !node) {
        printf("fibonacci heap - insert: Invalid Argumentd!\n");
        exit(-1);
    }

    if (!fh->min) {
        fh->min     = node;
        node->left  = node;
        node->right = node;
    } else {
        node->right       = fh->min->right;
        node->right->left = node;
        node->left        = fh->min;
        fh->min->right    = node;

        if (fh->min->key > node->key) {
            fh->min = node;
        }
    }

    fh->num_nodes++;
}

fib_node*
fib_heap_min(fib_heap_t* fh)
{
    if (!fh) {
        printf("fibonacci heap - min: Invalid Argumentd!\n");
        exit(-1);
    }
    if (!fh->min) {
        printf("fibonacci_heap: minimum is not set.");
        exit(-1);
    }
    return fh->min;
}

void
fib_heap_make_child(fib_node* x, fib_node* y)
{
    if (!x || !y) {
        printf("fibonacci heap - make_child: Invalid Argumentd!\n");
        exit(-1);
    }

    /* delete y from root list */
    y->left->right = y->right;
    y->right->left = y->left;

    /* make y child of x */
    y->parent = x;
    if (x->child) {
        y->right        = x->child->right;
        y->right->left  = y;
        y->left         = x->child;
        x->child->right = y;
    } else {
        x->child = y;
        y->left  = y;
        y->right = y;
    }
    y->mark = false;
    x->degree++;
}

void
fib_heap_consolidate(fib_heap_t* fh)
{
    if (!fh || !fh->min) {
        printf("fibonacci heap - consolidate: Invalid Argumentd!\n");
        exit(-1);
    }
    unsigned int max_degree =
          floor(log_golden_ratio_factor * logf((float)fh->num_nodes));
    fib_node** nodes_w_degree = calloc(max_degree, sizeof(fib_node*));

    fib_node*    first = fh->min->right;
    fib_node*    temp;
    fib_node*    x = fh->min->right;
    fib_node*    y;
    unsigned int d;

    /* Collapse all nodes with the same degree until all degrees are unique */
    do {
        d = x->degree;

        /* Find roots with the same degree */
        while (nodes_w_degree[d]) {
            y = nodes_w_degree[d];
            if (x == y) {
                break;
            }

            /* Make the root with the smaller key a child of the other. */
            /* Clear mark, increment degree */
            if (y->key < x->key) {
                temp = x;
                x    = y;
                y    = temp;
            }

            fib_heap_make_child(x, y);

            if (y == first) {
                first = x;
            }

            nodes_w_degree[d] = NULL;
            ++d;
        }
        nodes_w_degree[d] = x;
        x                 = x->right;
    } while (x != first);

    /* rebuild root list */
    fh->min = NULL;

    for (size_t i = 0; i < max_degree; ++i) {
        if (nodes_w_degree[i]) {
            x = nodes_w_degree[i];
            if (!fh->min) {
                fh->min  = x;
                x->left  = x;
                x->right = x;
            } else {
                x->right             = fh->min->right;
                x->left              = fh->min;
                fh->min->right->left = x;
                fh->min->right       = x;

                if (x->key < fh->min->key) {
                    fh->min = x;
                }
            }
        }
    }
    free(nodes_w_degree);
}

fib_node*
fib_heap_extract_min(fib_heap_t* fh)
{
    if (!fh) {
        printf("fibonacci heap - consolidate: Invalid Argumentd!\n");
        exit(-1);
    }

    fib_node* z = fh->min;
    if (z) {
        fib_node* node = z->child;
        fib_node* next;

        /* iterate over all trees */
        while (node != NULL) {
            if (node->right != node) {
                next = node->right;
            } else {
                next = NULL;
            }

            node->parent = NULL;

            // Remove node from min's children
            node->left->right = node->right;
            node->right->left = node->left;

            // Insert node to the right of min
            node->left     = z;
            node->right    = z->right;
            z->right->left = node;
            z->right       = node;

            node = next;
        }

        // Remove min from the root list.
        z->left->right = z->right;
        z->right->left = z->left;

        if (z->right == z) {
            fh->min = NULL;
        } else {
            fib_heap_consolidate(fh);
        }
        fh->num_nodes--;
    } else {
        printf("fibonacci_heap: minimum is not set.");
        exit(-1);
    }
    return z;
}

fib_heap_t*
fib_heap_union(fib_heap_t* fh1, fib_heap_t* fh2)
{
    if (!fh1 || !fh2) {
        printf("fibonacci heap - union: Invalid Argumentd!\n");
        exit(-1);
    }

    fib_heap_t* fh = create_fib_heap();

    if (fh1->num_nodes == 0 && fh2->num_nodes == 0) {
        return fh;
    }
    if (fh1->num_nodes == 0 || fh2->num_nodes == 0) {
        fh->min = fh1->num_nodes == 0 ? fh2->min : fh1->min;
    } else if (fh1->num_nodes > 0 && fh2->num_nodes > 0) {
        fib_node* fh2_temp;
        fib_node* fh1_temp;

        fh->min = fh1->min;

        fh1_temp        = fh1->min->right;
        fh2_temp        = fh2->min->left;
        fh1->min->right = fh2->min;
        fh2->min->left  = fh1->min;
        fh1_temp->left  = fh2_temp;
        fh2_temp->right = fh1_temp;

        if (fh2->min->key < fh->min->key) {
            fh->min = fh2->min;
        }
    }

    fh->num_nodes = fh1->num_nodes + fh2->num_nodes;

    free(fh1);
    free(fh2);

    return fh;
}

void
fib_heap_cut(fib_heap_t* fh, fib_node* node, fib_node* parent)
{
    if (!fh || !node || !parent) {
        printf("fibonacci heap - cut: Invalid Argumentd!\n");
        exit(-1);
    }

    if (parent->degree == 1) {
        parent->child = NULL;
    } else {
        node->right->left = node->left;
        node->left->right = node->right;

        if (node == parent->child) {
            parent->child = node->right;
        }
    }
    parent->degree--;
    node->parent = NULL;
    node->mark   = false;

    node->right          = fh->min->right;
    node->left           = fh->min;
    fh->min->right->left = node;
    fh->min->right       = node;
}

void
fib_heap_cascading_cut(fib_heap_t* fh, fib_node* node)
{
    if (!fh || !node) {
        printf("fibonacci heap - cascading_cut: Invalid Argumentd!\n");
        exit(-1);
    }

    fib_node* parent;

    while ((parent = node->parent)) {
        if (!node->mark) {
            node->mark = true;
            break;
        }
        fib_heap_cut(fh, node, parent);
        node = parent;
    }
}

void
fib_heap_decrease_key_internal(fib_heap_t* fh,
                               fib_node*   node,
                               double      new_key,
                               bool delete)
{
    if (!fh || !node || new_key > node->key) {
        printf("fib heap - decrease key: Arguments null or previous key was "
               "larger then new key!\n");
        exit(-1);
    }

    if (!delete &&new_key == -DBL_MAX) {
        printf("A key of value DBL_MAX is not allowed!"
               "Please use somthing greater than"
               "-DBL_MAX as smallest priority!");
        exit(-1);
    }

    fib_node* parent;

    node->key = new_key;
    parent    = node->parent;

    if (parent && node->key < parent->key) {
        fib_heap_cut(fh, node, parent);
        fib_heap_cascading_cut(fh, parent);
    }
    if (node->key < fh->min->key) {
        fh->min = node;
    }
}

void
fib_heap_decrease_key(fib_heap_t* fh, fib_node* node, double new_key)
{
    fib_heap_decrease_key_internal(fh, node, new_key, false);
}

void
fib_heap_delete(fib_heap_t* fh, fib_node* node)
{
    fib_heap_decrease_key_internal(fh, node, -DBL_MAX, true);
    free(fib_heap_extract_min(fh));
}

#endif