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

int main(int argc, char **argv) 
{
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
        newWidth = (int) (width / ratio);
        newHeight = (int) (height / ratio);
        printf("%d*%d\n", newWidth, newHeight);
    }

    /* pTarget = stbir_resize_uint8(pImage, width, width, 0, pTarget, (int) newWidth, (int) newHeight, 0, comp); */
    pTarget = stbir_resize_uint8_linear(pImage, width, height, comps * width, pTarget, newWidth, newHeight, newWidth * comps, STBIR_RGB);

    printf("target image at %p\n", pTarget);

    printf("saving image as rsz.png\n");
    stbi_write_png(
		"rsz.png", newWidth, newHeight, comps, pTarget, newWidth * comps);

    printf("done");
    getchar();

    stbi_image_free(pImage);
}
