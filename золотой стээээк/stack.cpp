#include "config.h"
#include "stack.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "memchecker.h"

#if defined(_USE_HASH_STRUCT_) || defined(_USE_HASH_BUFFER_)
    #include "hash.h"
#endif

#define CHECKR(condition,message,errcode)\
        if (condition)\
        {\
            fprintf(stderr, message);\
            st->stk.error += errcode;\
        }\

//*************************************************************************************************

#ifdef _USE_INCAPSULATION_
typedef struct _stack_
{
#ifdef _USE_CNRY_STRUCT_
    cnry_t      canary1_stk;
#endif
    size_t      size;
    size_t      capacity;
    size_t      esize;
    void        *data;
    void        (*print_data)(const void *ptr, FILE *output);
    create_info create;
    stk_info    stk;
#ifdef _USE_HASH_STRUCT_
    hash_t      hash_struct;
#endif
#ifdef _USE_HASH_BUFFER_
    hash_t      hash_buffer;
#endif
#ifdef _USE_CNRY_STRUCT_
    cnry_t      canary2_stk;
#endif
}stack_t;
#endif

//*************************************************************************************************

call_info call = {};
call_info vercall = {};

//*************************************************************************************************

#ifdef _USE_INCAPSULATION_
stack_t *stk_ator()
{
    stack_t *st = (stack_t *) calloc(1, sizeof(stack_t));

    return st;
}
#endif

//*************************************************************************************************

int stk_ctor__(stack_t *st, const size_t esize, const size_t cap, void (*print_data)(const void *, FILE *))
{
    assert(st);
    assert(print_data);
    CHECKR(!mem_check(st, sizeof(stack_t)), "ptr *st is protected", MEMERROR)

    st->stk.error = STKNOERR;
    CHECKR(st->stk.state == CREATED, "one more try to create created stack\n", SECCTORU)
    CHECKR(!esize, "elemsize is 0\n", BADESIZE)
    CHECKR(!cap, "start capacity is 0\n", BADSTCAP)
#ifdef _USE_CNRY_STRUCT_
    st->canary1_stk = st->canary2_stk = CNRY_ULI;
#endif
    st->esize = esize;
    st->capacity = cap;
    st->size = 0;
    st->print_data = print_data;
#ifdef _USE_CNRY_BUFFER_
    st->data = malloc(sizeof(cnry_t) + cap * esize + sizeof(cnry_t));
    CHECKR(!st->data, "can't allocate memory for stk_data\n", STKALLOC)
    *((cnry_t *) st->data) = CNRY_ULI;
    *((cnry_t *) st->data + cap * esize / sizeof(cnry_t) + 1) = CNRY_ULI;
    st->data = (char *) st->data + sizeof(cnry_t);
#else
    st->data = malloc(cap * esize);
    CHECKR(!st->data, "can't allocate memory for stk_data\n", STKALLOC)   
#endif
    st->stk.state = CREATED;
#ifdef _USE_POISON_
    memset(st->data, POISON, cap * esize);
#endif

#ifdef _USE_HASH_STRUCT_
    st->hash_struct = COUNT_HASH_STRUCT(st)
#endif
#ifdef _USE_HASH_BUFFER_
    st->hash_buffer = COUNT_HASH_BUFFER(st)
#endif

    stk_verify(st);

    return st->stk.error;
}

//*************************************************************************************************

int stk_dtor__(stack_t *st)
{
    assert(st);
    CHECKR(!mem_check(st, sizeof(stack_t)), "ptr *st is protected", MEMERROR)

    if (st->stk.state == DESTROYED)
    {
        st->stk.error += SECDTORU;
        fprintf(stderr, "one more try to destroy destroyed stack\n");
        stk_verify(st);
        return EXIT_FAILURE;
    }
    
    stk_verify(st);

#ifdef _USE_POISON_
    memset(st->data, POISON, st->size * st->esize);
#endif
#ifdef _USE_CNRY_BUFFER_
    free((char *) st->data - sizeof(cnry_t));
#else
    free(st->data);
#endif
#ifndef _USE_INCAPSULATION_
    st->stk.state = DESTROYED;
#else
    free(st);
#endif

    return 0;
}

//*************************************************************************************************

int stk_push__(stack_t *st, const void *val)
{
    assert(st);
    assert(val);
    CHECKR(!mem_check(st, sizeof(stack_t)), "ptr *st is protected", MEMERROR)

    stk_verify(st);

    if (st->size == st->capacity)
        stk_resize(st, RESIZE_MORE);
    memcpy((char *) st->data + st->size * st->esize, val, st->esize);
    st->size++;

#ifdef _USE_HASH_STRUCT_
    st->hash_struct = COUNT_HASH_STRUCT(st)
#endif
#ifdef _USE_HASH_BUFFER_
    st->hash_buffer = COUNT_HASH_BUFFER(st)
#endif

    stk_verify(st);

    return st->stk.error;
}

//*************************************************************************************************

int stk_pop__(stack_t *st, void *val)
{
    assert(st);
    assert(val);
    CHECKR(!mem_check(st, sizeof(stack_t)), "ptr *st is protected", MEMERROR)
    CHECKR(!mem_check(val, st->esize), "ptr *val is protected", MEMERROR)

    stk_verify(st);

    if ((st->size - 1) == (st->capacity / (WIDECOEFF * WIDECOEFF)))
        stk_resize(st, RESIZE_LESS);
    st->size--;
    memcpy(val, (char *) st->data + st->size * st->esize,  st->esize);
#ifdef _USE_POISON_
    memset((char *) st->data + st->size * st->esize, POISON, st->esize);
#endif

#ifdef _USE_HASH_STRUCT_
    st->hash_struct = COUNT_HASH_STRUCT(st)
#endif
#ifdef _USE_HASH_BUFFER_
    st->hash_buffer = COUNT_HASH_BUFFER(st)
#endif

    stk_verify(st);

    return st->stk.error;
}

//*************************************************************************************************

int stk_resize(stack_t *st, int param)
{
    assert(st);
    CHECKR(!mem_check(st, sizeof(stack_t)), "ptr *st is protected", MEMERROR)

    if (param == RESIZE_LESS)
    {
        st->capacity /= WIDECOEFF;
#ifdef _USE_CNRY_BUFFER_
        st->data = realloc((char *) st->data - sizeof(cnry_t), sizeof(cnry_t) + st->capacity * st->esize + sizeof(cnry_t));
        CHECKR(!st->data, "can't reallocate less memory for stk_data\n", STKRLLOC)
        st->data = (char *) st->data + sizeof(cnry_t);
        *((cnry_t *) st->data + st->capacity * st->esize) = CNRY_ULI;
#else
        st->data = realloc(st->data, st->capacity * st->esize);
        CHECKR(!st->data, "can't reallocate less memory for stk_data\n", STKRLLOC)
#endif
    }
    if (param == RESIZE_MORE)
    {
        st->capacity *= WIDECOEFF;
#ifdef _USE_CNRY_BUFFER_
        st->data = realloc((char *) st->data - sizeof(cnry_t), sizeof(cnry_t) + st->capacity * st->esize + sizeof(cnry_t));
        CHECKR(!st->data, "can't reallocate more memory for stk_data\n", STKRLLOC)
        st->data = (char *) st->data + sizeof(cnry_t);
#ifdef _USE_POISON
        memset((char *) st->data + st->size * st->esize, POISON, st->esize * (st->capacity - st->size));
#endif
        *((cnry_t *) st->data + st->capacity * st->esize) = CNRY_ULI;
#else
        st->data = realloc(st->data, st->capacity * st->esize);
        CHECKR(!st->data, "can't reallocate more memory for stk_data\n", STKRLLOC)
#ifdef _USE_POISON_
        memset((char *) st->data + st->size * st->esize, POISON, st->esize * (st->capacity - st->size));
#endif
#endif
    }

#ifdef _USE_HASH_STRUCT_
    st->hash_struct = COUNT_HASH_STRUCT(st)
#endif
#ifdef _USE_HASH_BUFFER_
    st->hash_buffer = COUNT_HASH_BUFFER(st)
#endif

    return st->stk.error;
}

//*************************************************************************************************

int stk_verify__(stack_t *st, const char *file, const char *func, const int line)
{
    assert(st);
    assert(file);
    assert(func);
    CHECKR(!mem_check(st, sizeof(stack_t)), "ptr *st is protected", MEMERROR)

    vercall.file = file;
    vercall.func = func;
    vercall.line = line;

    CHECKR(st->size >= st->capacity, "problems with size: stk_size >= stk_capacity\n", SIZERROR)
    CHECKR(!st->esize, "size of stack element is 0 byte\n", BADESIZE)
    CHECKR(!st->capacity, "stack capacity is 0\n", BADCAPAS)
#ifdef _USE_CNRY_STRUCT_
    CHECKR(st->canary1_stk != CNRY_ULI, "left stk canary has wrong value\n", LCNRYSTK)
    CHECKR(st->canary2_stk != CNRY_ULI, "right stk canary has wrong value\n", RCNRYSTK)
#endif
#ifdef _USE_CNRY_BUFFER_
    CHECKR(*(cnry_t *)((char *) st->data - sizeof(cnry_t)) != CNRY_ULI, "left buf canary has wrong value\n", LCNRYBUF)
    CHECKR(*(cnry_t *)((char *) st->data + st->esize * st->capacity) != CNRY_ULI, "right buf canary has wrong value\n", RCNRYBUF)
#endif
#ifdef _USE_HASH_STRUCT
    hash_t hash_struct = COUNT_HASH_STRUCT(st)
    CHECKR(hash_struct != st->hash_struct, "bad struct hash\n", WHASHSTK)
#endif
#ifdef _USE_HASH_BUFFER_
    hash_t hash_buffer = COUNT_HASH_BUFFER(st)
    CHECKR(hash_buffer != st->hash_buffer, "bad buffer hash\n", WHASHBUF)
#endif

    if (st->stk.error)
    {
        FILE *log = fopen("log_stack", "a");
        stk_dump__(st, log, DUMP_VERCALL);
        fclose(log);
        exit(EXIT_FAILURE);
    }

    return st->stk.error;
}

//*************************************************************************************************

void get_create_info(stack_t *st, const char *name, const char *type, const char *file, const int line, const char *func)
{
    assert(st);
    assert(name);
    assert(type);
    assert(file);
    CHECKR(!mem_check(st, sizeof(stack_t)), "ptr *st is protected", MEMERROR)

#ifndef _USE_INCAPSULATION_
    st->create.name = name + 1;
#else
    st->create.name = name;
#endif
    st->create.type = type;
    st->create.file = file;
    st->create.line = line;
    st->create.func = func;
}

//*************************************************************************************************

void get_calling_info(const char *file, const char *func, const int line)
{
    assert(file);
    assert(func);

    call.file = file;
    call.func = func;
    call.line = line;
}

//*************************************************************************************************

#ifdef _USE_POISON_
int is_poisoned(const void *ptr, const size_t byte_amount)
{
    assert(ptr);

    for (size_t num = 0; num < byte_amount;)
    {
        if (*((char *) ptr + num) == (char) POISON)
            num++;
        else
            return 0;
    }

    return 1;
}
#endif

//*************************************************************************************************

void print_data_int(const void *ptr, FILE *output)
{
    assert(ptr);
    assert(output);

    const stack_t *st = (const stack_t *) ptr;

    for (size_t pos = 0; pos < st->capacity; pos++)
    {
        if (pos < st->size)
            fprintf(output, "       *");
        else
            fprintf(output, "        ");

        int elem = *((int *) st->data + pos);
        fprintf(output, "[%lu] = %d", pos, elem);
#ifdef _USE_POISON_
        if (is_poisoned(&elem, st->esize))
            fprintf (output, " (POISON)\n");
        else
            fprintf(output, "\n");
#else
        fprintf(output, "\n");
#endif
    }
}

//*************************************************************************************************

void print_data_byte(const void *ptr, FILE *output)
{
    assert(ptr);
    assert(output);

    const stack_t *st = (const stack_t *) ptr;

    for (size_t pos = 0; pos < st->capacity; pos++)
    {
        if (pos < st->size)
            fprintf(output, "       *");
        else
            fprintf(output, "        ");

        char *elem = (char *) st->data + pos * st->esize;
        fprintf(output, "[%lu] = ", pos);
        for (size_t byte_number = 0; byte_number < st->esize; byte_number++)
            fprintf(output, "%hhx ", *(elem + byte_number));
#ifdef _USE_POISON_
        if (is_poisoned(elem, st->esize))
            fprintf (output, "(POISON)\n");
        else
            fprintf(output, "\n");
#else
        fprintf(output, "\n");
#endif
    }   
}

void stk_dump__(const stack_t *st, FILE *output, int param)
{
    assert(st);
    assert(output);

    fprintf(output, "Stack [%p] ", st);

    if (st->create.type)
        fprintf(output, "(type: <%s>) ", st->create.type);
    else
        fprintf(output, "(type: <->) ");

    if (st->create.name)
        fprintf(output, "with name \"%s\", ", st->create.name);
    else
        fprintf(output, "with name \"-\"");

    if (st->create.func)
        fprintf(output, "created at %s (%d) ", st->create.func, st->create.line);
    else
        fprintf(output, "created at - (-) ");

    if (st->create.file)
        fprintf(output, "at %s ", st->create.file);
    else
        fprintf(output, "at - ");

    if (st->stk.error != STKNOERR)
        fprintf(output, "is BAD.\n");
    else
        fprintf(output, "is OK.\n");

    if (param == DUMP_VERCALL)
    {
        fprintf(output, "  Dump func is called at %s (%d) at  %s,\n", vercall.func, vercall.line, vercall.file);
        fprintf(output, "               called at %s (%d) at %s.\n", call.func, call.line, call.file);
    }
    if (param == DUMP_USRCALL)
        fprintf(output, "  Dump func is called at %s (%d) at %s.\n", call.func, call.line, call.file);

    fprintf(output, "%s\n{\n", st->create.name);

#ifdef _USE_CNRY_STRUCT_
    fprintf(output, "    left stk canary = ");
    for (size_t num = 0; num < sizeof(cnry_t); num++)
        fprintf(output, "%hhx", *((char *) &st->canary1_stk + num));
    if (st->stk.error & LCNRYSTK)
        fprintf(output, "    <---- WRONG CANARY VALUE");
    fprintf(output, ";\n");
#endif

    fprintf(output, "    size = %lu;\n", st->size);
    if (st->stk.error & SIZERROR)
        fprintf(output, "        BAD STACK SIZE: (size = %lu) >= (capacity = %lu)\n", st->size, st->capacity);

    fprintf(output, "    capacity = %lu;\n", st->capacity);
    if (st->stk.error & BADCAPAS)
        fprintf(output, "        BAD CAPACITY: stack capacity is 0\n");

    fprintf(output, "    esize = %lu;\n", st->esize);
    if (st->stk.error & BADESIZE)
        fprintf(output, "        BAD ESIZE: elem size is 0\n");

    int data_id = mem_check(st->data, st->esize * st->capacity);
    if (st->data && data_id)
    {
        fprintf(output, "    data [%p]\n    {\n", st->data);

#ifdef _USE_CNRY_BUFFER_
        fprintf(output, "        [left buf canary] = ");
        for (size_t num = 0; num < sizeof(cnry_t); num++)
            fprintf(output, "%hhx", *((char *) st->data - sizeof(cnry_t) + num));
        if (st->stk.error & LCNRYBUF)
            fprintf(output, "    <---- WRONG CANARY VALUE");
        fprintf(output, "\n");
#endif

        st->print_data(st, output);

#ifdef _USE_CNRY_BUFFER_
        fprintf(output, "        [right buf canary] = ");
        for (size_t num = 0; num < sizeof(cnry_t); num++)
            fprintf(output, "%hhx", *((char *) st->data + st->esize * st->capacity + num));
        if (st->stk.error & RCNRYBUF)
            fprintf(output, "    <---- WRONG CANARY VALUE");
        fprintf(output, "\n");
#endif

        fprintf(output, "    };\n");
    }
    else
        if (data_id)
            fprintf(output, "    data [%p] <-- BAD DATA POINTER, ", st->data);
        else
            fprintf(output, "    data [-] <-- BAD DATA POINTER: non-accessable, ");           

#ifdef _USE_CNRY_STRUCT_
    fprintf(output, "    right stk canary = ");
    for (size_t num = 0; num < sizeof(cnry_t); num++)
        fprintf(output, "%hhx", *((char *) &st->canary2_stk + num));
    if (st->stk.error & RCNRYSTK)
        fprintf(output, "    <---- WRONG CANARY VALUE");
    fprintf(output, ";\n");
#endif

    if (st->stk.error != STKNOERR)
    {
        fprintf(output, "\n    All stack errors:\n    {\n");
        if (st->stk.error & STKALLOC)
            fprintf(output, "        STKALLOC - allocation error;\n");
        if (st->stk.error & STKRLLOC)
            fprintf(output, "        STKRLLOC - reallocation error;\n");
        if (st->stk.error & SECCTORU)
            fprintf(output, "        SECCTORU - tried to create created stack;\n");
        if (st->stk.error & BADCAPAS)
            fprintf(output, "        BADCAPAS - bad stack capacity (capacity is 0);\n");
        if (st->stk.error & SIZERROR)
            fprintf(output, "        SIZERROR - bad stack size;\n");
        if (st->stk.error & BADSTCAP)
            fprintf(output, "        BADSTCAP - bad start stack capacity (start cpacity is 0);\n");
        if (st->stk.error & BADESIZE)
            fprintf(output, "        BADESIZE - user's elemsize is 0;\n");
        if (st->stk.error & SECDTORU)
            fprintf(output, "        SECDTORU - tried to destroy destroyed stack;\n");
#ifdef _USE_CNRY_STRUCT_
        if (st->stk.error & LCNRYSTK)
            fprintf(output, "        LCNRYSTK - left canary of struct stack has wrong value;\n");
        if (st->stk.error & RCNRYSTK)
            fprintf(output, "        RCNRYSTK - right canary of struct stack has wrong value;\n");
#endif
#ifdef _USE_CNRY_BUFFER_
        if (st->stk.error & LCNRYBUF)
            fprintf(output, "        LCNRYBUF - left canary of buffer with data has wrong value;\n");
        if (st->stk.error & RCNRYBUF)
            fprintf(output, "        RCNRYBUF - right canary of buffer with data has wrong value;\n");
#endif
#ifdef _USE_HASH_BUFFER_
        if (st->stk.error & WHASHBUF)
            fprintf(output, "        WHASHBUF - hash of data is WRONG;\n");
#endif
#ifdef _USE_HASH_STRUCT_
        if (st->stk.error & WHASHSTK)
            fprintf(output, "        WHASHSTK - hash of struct members is WRONG;\n");
#endif

        fprintf(output, "    }\n");
    }

    fprintf(output, "}\n\n");
}
