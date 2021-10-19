#include <stdlib.h>
#include "hash.h"

//*************************************************************************************************

hash_t count_hash(const void *ptr, size_t amnt)
{
    hash_t hash = 0;
    for (size_t num = 0; num < amnt; num++)
        hash += (num + 1) * *((char *) ptr + num);

    return hash;
}
