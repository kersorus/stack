#ifndef _RELIZ_
    #define _USE_HASH_STRUCT_      // enable hash for struct _stack_ (aka stack_t)
    #define _USE_HASH_BUFFER_      // enable hash for buffer with stack data
    #define _USE_CNRY_STRUCT_      // enable canaries for struct _stack_ (aka stack_t)
    #define _USE_CNRY_BUFFER_      // enable canaries for buffer with stack data
    #define _USE_POISON_           // enable using of poison (data that is not used in stack right now has poison value)
    #define _USE_INCAPSULATION_    // enable incapsulation: struct is deleted from "stack.h"
#else
    #define NDEBUG                 // stops assert() and mem-checking
#endif
