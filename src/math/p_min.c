#include <pal.h>

/**
 *
 * Finds the minimum value in vector 'a'. Returns the min value and the index
 * of the minimum value.
 *
 * @param      a      Pointer to input vector
 *
 * @param      c      Pointer to output scalar
 *
 * @param[out] index  Pointer to return index of min
 *
 * @param n           Size of 'a' vector.
 * 
 * @return            None
 *
 */
void PSYM(p_min)(const PTYPE *a, PTYPE *c, int *index, int n)
{
    PTYPE min;
    int i, pos;

    min = *a;
    pos = 0;

    for (i = 1; i < n; i++) {
        if (*(a + i) < min) {
            pos = i;
            min = *(a + i);
        }
    }
    *c = min;
    *index = pos;
}
