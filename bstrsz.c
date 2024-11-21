#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_STATIC
#include <stb_image_resize2.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

uint8_t *pImage, *pTarget = NULL;
int width, height, comps;
int newWidth, newHeight;
int doResize = 0;
float ratioX, ratioY, ratio;

unsigned short flr(double x) {
    return (unsigned short)x - (x < (unsigned short)x);
}

unsigned short cl(double x) {
    return (unsigned short)x + (x > (unsigned short)x);
}

/* Home made resize */
uint8_t *resizeImage(const uint8_t *source, uint16_t width, uint16_t height,
                     uint8_t comps, uint8_t *target, uint16_t newWidth,
                     uint16_t newHeight) {
    unsigned short x_l, y_l, x_h, y_h;
    float x_weight, y_weight;

    uint8_t ar, br, cr, dr;
    uint8_t ag, bg, cg, dg;
    uint8_t ab, bb, cb, db;
    uint8_t r, g, b;

    target = (uint8_t *)malloc(newWidth * newHeight * comps);

    float x_ratio = newWidth > 1 ? ((float)(width - 1) / (newWidth - 1)) : 0;
    float y_ratio = newHeight > 1 ? ((float)(height - 1) / (newHeight - 1)) : 0;

    for (uint16_t i = 0; i < newHeight; i++) {
        for (uint16_t j = 0; j < newWidth; j++) {
            x_l = flr(x_ratio * j);
            y_l = flr(y_ratio * i);
            x_h = cl(x_ratio * j);
            y_h = cl(y_ratio * i);

            x_weight = (x_ratio * j) - x_l;
            y_weight = (y_ratio * i) - y_l;

            ar = source[(y_l * width * comps) + x_l * comps];
            br = source[(y_l * width * comps) + x_h * comps];
            cr = source[(y_h * width * comps) + x_l * comps];
            dr = source[(y_h * width * comps) + x_h * comps];

            ag = source[(y_l * width * comps) + x_l * comps + 1];
            bg = source[(y_l * width * comps) + x_h * comps + 1];
            cg = source[(y_h * width * comps) + x_l * comps + 1];
            dg = source[(y_h * width * comps) + x_h * comps + 1];

            ab = source[(y_l * width * comps) + x_l * comps + 2];
            bb = source[(y_l * width * comps) + x_h * comps + 2];
            cb = source[(y_h * width * comps) + x_l * comps + 2];
            db = source[(y_h * width * comps) + x_h * comps + 2];

            r = ar * (1 - x_weight) * (1 - y_weight) +
                br * x_weight * (1 - y_weight) +
                cr * y_weight * (1 - x_weight) + dr * x_weight * y_weight;

            g = ag * (1 - x_weight) * (1 - y_weight) +
                bg * x_weight * (1 - y_weight) +
                cg * y_weight * (1 - x_weight) + dg * x_weight * y_weight;

            b = ab * (1 - x_weight) * (1 - y_weight) +
                bb * x_weight * (1 - y_weight) +
                cb * y_weight * (1 - x_weight) + db * x_weight * y_weight;

            target[i * newWidth * comps + j * comps] = r;
            target[i * newWidth * comps + j * comps + 1] = g;
            target[i * newWidth * comps + j * comps + 2] = b;
        }
    }

    return target;
}

int main(int argc, char **argv) {
    printf("Loading image\n");
    pImage = stbi_load(argc > 1 ? argv[1] : "", &width, &height, &comps, 3);

    if (!pImage) {
        printf("Error on loading image !\n");
        getchar();
        return 0;
    }

    printf("Original size: %d*%d\n", width, height);

    doResize = 0;
    if (width >= 320) {
        ratioX = width / 320.0f;
        printf("ratio x %f\n", ratioX);
        doResize = 1;
    }

    if (height > 200) {
        ratioY = height / 200.0f;
        printf("ratio y %f\n", ratioY);
        doResize = 1;
    }

    if (doResize) {
        ratio = MAX(ratioX, ratioY);
        printf("ratio %f\n", ratio);
        newWidth = (int)(width / ratio);
        newHeight = (int)(height / ratio);
        printf("%d*%d\n", newWidth, newHeight);
    }

    /* pTarget = stbir_resize_uint8_linear(pImage, width, height, comps * width,
     * pTarget, newWidth, newHeight, newWidth * comps, STBIR_RGB); */
    pTarget =
        resizeImage(pImage, width, height, comps, pTarget, newWidth, newHeight);

    printf("target image at %p\n", pTarget);

    printf("saving image as rsz.png\n");
    stbi_write_png("rsz.png", newWidth, newHeight, comps, pTarget,
                   newWidth * comps);

    printf("done");
    getchar();

    stbi_image_free(pImage);
}
