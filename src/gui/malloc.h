#ifndef KLIKOS_GUI_MALLOC_H
#define KLIKOS_GUI_MALLOC_H

#include <kernel/types.h>

void *malloc(u32 size);
void  free(void *ptr);

#endif /* KLIKOS_GUI_MALLOC_H */
