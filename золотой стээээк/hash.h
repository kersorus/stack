#ifndef _HASH_H_
#define _HASH_H_

#define COUNT_HASH_STRUCT(st) count_hash((char *) st + sizeof(cnry_t), sizeof(stack_t) - 2 * sizeof(cnry_t) - 2 * sizeof(hash_t));
#define COUNT_HASH_BUFFER(st) count_hash(st->data, st->esize * st->size);

//*************************************************************************************************

typedef unsigned long int hash_t;

//*************************************************************************************************

hash_t count_hash (const void *ptr, size_t amnt);

#endif
