#ifndef _STACK_H_
#define _STACK_H_

#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(_USE_HASH_STRUCT_) || defined(_USE_HASH_BUFFER_)
    #include "hash.h"
#endif

//*************************************************************************************************

typedef struct _create_info_
{
    const char *name;
    const char *type;
    const char *file;
    int line;
    const char *func;
}create_info;

typedef struct _calling_info_
{
    const char *file;
    const char *func;
    int line;
}call_info;

//*************************************************************************************************

#if defined(_USE_CNRY_STRUCT_) || defined(_USE_CNRY_BUFFER_)
typedef unsigned long int cnry_t;
#endif

//*************************************************************************************************

typedef struct _stack_info_
{
    int state;
    int error;
}stk_info;

#ifndef _USE_INCAPSULATION_
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
#else
typedef struct _stack_ stack_t; 
#endif

//*************************************************************************************************

enum STK_CONSTS
{
    DESTROYED    = -1,
    CREATED      =  1,
    WIDECOEFF    =  2,
    RESIZE_LESS  =  3,
    RESIZE_MORE  =  4,
    DUMP_USRCALL =  5,
    DUMP_VERCALL =  6,
    POISON       = 0xC3,
    CNRY_ULI     = 0x3713AFBEAFBE3713
};

enum STK_ERRORS
{
    STKNOERR = 0b0000000000000000,
    STKALLOC = 0b0000000000000001,
    STKRLLOC = 0b0000000000000010,
    SECCTORU = 0b0000000000000100,
    BADCAPAS = 0b0000000000001000,
    SIZERROR = 0b0000000000010000,
    BADESIZE = 0b0000000000100000,
    BADSTCAP = 0b0000000001000000,
    SECDTORU = 0b0000000010000000,
    WHASHBUF = 0b0000000100000000,
    LCNRYSTK = 0b0000001000000000,
    RCNRYSTK = 0b0000010000000000,
    LCNRYBUF = 0b0000100000000000,
    RCNRYBUF = 0b0001000000000000,
    WHASHSTK = 0b0010000000000000,
    MEMERROR = 0b0100000000000000
};

//*************************************************************************************************

#ifdef _USE_INCAPSULATION_
stack_t *stk_ator    ();
#endif
int  stk_ctor__      (stack_t *st, const size_t esize, const size_t cap, void (*print_data)(const void *, FILE *));
int  stk_dtor__      (stack_t *st);
int  stk_push__      (stack_t *st, const void *val);
int  stk_pop__       (stack_t *st, void *val);
int  stk_resize      (stack_t *st, int param);
int  stk_verify__    (stack_t *st, const char *file, const char *func, const int line);
void get_create_info (stack_t *st, const char *name, const char *type, const char *file, const int line, const char *func);
void get_calling_info(const char *file, const char *func, const int line);
void stk_dump__      (const stack_t *st, FILE *output, int param);
void print_data_byte (const void *ptr, FILE *output);
void print_data_int  (const void *ptr, FILE *output);

#define stk_ctor(stack,type,cap,print);\
        get_create_info(stack, #stack, #type, __FILE__, __LINE__, __PRETTY_FUNCTION__);\
        get_calling_info(__FILE__, __PRETTY_FUNCTION__, __LINE__);\
        stk_ctor__(stack, sizeof(type), cap, print);
#define stk_dtor(stack);\
        get_calling_info(__FILE__, __PRETTY_FUNCTION__, __LINE__);\
        stk_dtor__(stack);
#define stk_push(stack,val);\
        get_calling_info(__FILE__, __PRETTY_FUNCTION__, __LINE__);\
        stk_push__(stack, val);
#define stk_pop(stack,val);\
        get_calling_info(__FILE__, __PRETTY_FUNCTION__, __LINE__);\
        stk_pop__(stack, val);
#define stk_verify(stack);\
        stk_verify__(stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);
#define stk_dump(stack,output);\
        get_calling_info(__FILE__, __PRETTY_FUNCTION__, __LINE__);\
        stk_dump__(stack, output, DUMP_USRCALL);

#endif
