#include <pal.h>

#include "p_sqrt.h"

/**
 *
 * Calculates the square root of the input vector 'a'.
 *
 * @param a     Pointer to input vector
 *
 * @param c     Pointer to output vector
 *
 * @param n     Size of 'a' and 'c' vector.
 *
 * @return      None
 *
 */
void PSYM(p_sqrt)(const PTYPE *a, PTYPE *c, int n)
{

    int i;
    for (i = 0; i < n; i++) {
        c[i] = _p_sqrt(a[i]);
    }
}
