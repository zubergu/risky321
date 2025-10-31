/*
 * gcc requires that freestanding environment provides these functions:
 * memcpy()
 * memmove()
 * memcmp()
 * memset()
 *
 * GCC can generate during compilation calls to these functions even if you don't call them explicitly.
 * In hosted environment, these functions are implemented in glibc in <string> library
 * 
*/
#include "klibc.h"

void *memcpy(void *dest, const void *src, size_t size)
{
    uint8_t *d = (uint8_t *) dest;
    uint8_t *s = (uint8_t *) src;

    for(size_t i = 0; i < size; i++)
    {
        d[i] = s[i];
    }

    return dest;
}

/*
 * memmove is like memcopy, except src and dest can overlap
 * so strictly copying could destroy source and distort data
*/
void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *) dest;
    uint8_t *s = (uint8_t *) src;
    
    if(d == s)
    {
        return d;
    }

    /* dest before src, safe to copy to the front of dest first*/
    if(d < s)
    {
        for(size_t i = 0; i<n; i++)
        {
            d[i] = s[i];
        }
    }
    else /* src before dest, safe to copy to the back of dest first */
    {
        for(size_t i = n - 1; n >=0; i--)
        {
            d[i] = s[i];
        }
    }

    return dest;
}

void *memset(void *dest, int c, size_t n)
{
    uint8_t *ptr = (uint8_t *) dest;
    uint8_t val = (uint8_t) c;

    for(int i=0; i<n; i++)
    {
        ptr[i] = val;
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    uint8_t *us1 = (uint8_t *) s1;
    uint8_t *us2 = (uint8_t *) s2;

    while(n > 0 && (*us1++ == *us2++))
    {
        n--;
    }

    if(n == 0)
    {
        return 0;
    }

    return *us1 - *us2;
}

int strcmp(const char *s1, const char *s2)
{
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}
