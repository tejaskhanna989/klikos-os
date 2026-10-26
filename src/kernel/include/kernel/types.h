#ifndef KLIKOS_TYPES_H
#define KLIKOS_TYPES_H

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned long   u64;
typedef signed char     s8;
typedef signed short    s16;
typedef signed int      s32;
typedef signed long     s64;

typedef u64  size_t;
typedef u64  uintptr_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

#endif /* KLIKOS_TYPES_H */
