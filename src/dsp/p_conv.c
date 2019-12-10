#include <pal.h>

/**

 * Computes the convolution of two vectors 'x' and 'h', and places the
 * results in vector 'r'.
 *
 * @param x       Pointer to input vector of size 'nr' elements
 *
 * @param h       Pointer to 'nh' filter coefficients
 *
 * @param r       Output vector of size 'nr+nh-1'
 *
 * @param nx      The number of input samples
 *
 * @param nh      The number of coefficients of the filter
 *
 * @return        None
 *
 */
void p_conv_f32(const float *x, const float *h, float *r, int nx, int nh)
{
    float *rx = r;
    int i,j ;
    for ( i = 0; i < nx+nh-1; i++) { *(rx++) = 0; }
    rx = r ;
    for ( i = 0; i < nx; i++) {
        float xv = *x++;
        for (j = 0; j < nh; j++) {
          *(rx + j) += xv * *(h + j);
        }
        rx++;
    }
}
