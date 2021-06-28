#include "data-struct/array_list.h"
#include "data-struct/cbs.h"

ARRAY_LIST_IMPL(array_list_ul, unsigned long);
array_list_ul_cbs al_ul_cbs = { unsigned_long_eq, NULL, NULL };

inline array_list_ul*
al_ul_create(void)
{
    return array_list_ul_create(al_ul_cbs);
}

ARRAY_LIST_IMPL(array_list_l, long);
array_list_l_cbs al_l_cbs = { long_eq, NULL, NULL };

inline array_list_l*
al_l_create(void)
{
    return array_list_l_create(al_l_cbs);
}

