#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pal.h>
#include <common.h>

/* stb image */
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#define BINARY ("filters2-dev-" XSTRING(DEVICE))

/* macros */
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

void float_to_ubyte(unsigned char *dest, float *src, int size)
{
    int i;
    for (i = 0; i < size; i++)
        dest[i] = (unsigned char)CLAMP(src[i] * 255, 0, 255);
}

int main(int argc, char *argv[])
{
    float *data;
    float *dest;
    unsigned char *data_ub; /* ubyte data */
    int w, h, n; /* with, height, n-components*/
    int i, size;

    int err;


    /* read and convert image */
    {
        float uctof = 1.0f / 255.0f;

        /* read */
        data_ub = stbi_load(ABS_TOP_SRCDIR "/examples/image/dataset/lena-small.tga",
                            &w, &h, &n, 0);
        if (! data_ub) {
            printf("unable to read image\n");
            return EXIT_FAILURE;
        }

        /* allocate memory */
        size = w * h;
        data = malloc(size * sizeof(float));
        dest = malloc(size * sizeof(float));

        /* convert to float */
        if (n == 1) /* already greyscale */
            for (i = 0; i < size; i++)
                data[i] = (float)data_ub[i] * uctof;
        else if(n >= 3) /* convert to greyscale */
            for (i = 0; i < size; i++)
                data[i] = (data_ub[i*n]*0.2f + data_ub[i*n + 1]*0.7f + data_ub[i*n + 2]*0.1f) * uctof;
    }

    // initialize device and team
    p_dev_t dev = p_init(P_DEV_EPIPHANY, 0);

    // load a program from file system
    p_prog_t prog = p_load(dev, BINARY, 0);

    p_team_t team = p_open(dev, 0, 1); // create a team

    // "Allocate" output buffer. TODO: We need an API call for this
    p_mem_t out = p_map(dev, 0x8f900000, size * sizeof(float));
    void *empty = alloca(size * sizeof(float));
    memset(empty, 0, size * sizeof(float));
    p_write(&out, empty, 0, size * sizeof(float), 0);

    // Construct program arguments
    // TODO: We can hide the is_primitive bool param with a macro
    // __builtin_types_compatible_p(x, int8) || compatible(i16) ...
    // I.e, P_ARG(h, sizeof(h)) --> { &h, sizeof(h), true }
    // ((!is_primitive) == pass_by_reference)
    p_arg_t args[] = {
        { data, (size * sizeof(float)), false },
        /* HACK */ { out.ref, sizeof(uint32_t), false },
        { &h, sizeof(h), true },
        { &w, sizeof(w), true },
    };

    // Run program
    err = p_run(prog, "gauss3x3", team, 0, 1, ARRAY_SIZE(args), args, 0);

    // Read back result. TODO: Use PAL API.
    p_read(&out, data, 0, size * sizeof(float), 0);
    //memcpy(data, (void *) out, size * sizeof(float));

    // TODO: Perform on device
    float_to_ubyte(data_ub, (float *) data, size);

    stbi_write_tga("dataset/lena_gaussian.tga", w, h, 1, data_ub);

    stbi_image_free(data_ub);

    // TODO: This is no-op atm.
    p_close(team);

    p_finalize(dev);
}
