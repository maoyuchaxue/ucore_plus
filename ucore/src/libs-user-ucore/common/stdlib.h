#ifndef __LIBS_STDLIB_H__
#define __LIBS_STDLIB_H__

#include <types.h>

/* the largest number rand will return */
#define RAND_MAX    2147483647UL

#define WEXITSTATUS(status) (((status)>>8)&(0xFF)) // 根据syscall手写一个

/* libs/rand.c */
int rand(void);
void srand(unsigned int seed);

/* libs/hash.c */
uint32_t hash32(uint32_t val, unsigned int bits);

#endif /* !__LIBS_RAND_H__ */
