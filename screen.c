#include <math.h>
#include <osbind.h>
#include <stdint.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_STATIC
#include <stb_image_resize2.h>

#include "wu.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define WHITE 1
#define BLACK 0

// when linking with libcmini
/* void __assert_fail(const char * assertion, const char * file, unsigned int
 * line, const char * function) */
/* { */
/*     fprintf(stderr, "My custom message\n"); */
/*     abort(); */
/* } */

const unsigned short power[16] = {32768U, 16384, 8192, 4096, 2048, 1024,
                                  512,    256,   128,  64,   32,   16,
                                  8,      4,     2,    1};
const short LOW_RES = 0;
const short MEDIUM_RES = 1;
unsigned short currentRes;
uint16_t atariPalette[16];
uint16_t previousPalette[16];
uint8_t threshold[3];
uint32_t palette[16];
uint32_t dawnbringer[16] = {0x140c1c, 0x442434, 0x30346d, 0x4e4a4e,
                            0x854c30, 0x346524, 0xd04648, 0x757161,
                            0x597dce, 0xd27d2c, 0x8595a1, 0x6daa2c,
                            0xd2aa99, 0x6dc2ca, 0xdad45e, 0xdeeed6};
uint8_t paletteCache[8][8][8];
uint32_t start, end;

short BAYER_2_2[2 * 2] = {0, 2, 3, 1};
int BAYER_2_2_SIZE[2] = {2, 2};

static short BAYER_4_4[4 * 4] = {0, 8,  2, 10, 12, 4, 14, 6,
                                 3, 11, 1, 9,  15, 7, 13, 5};
static int BAYER_4_4_SIZE[2] = {4, 4};

static short BAYER_8_8[8 * 8] = {
    0,  48, 12, 60, 3,  51, 15, 63, 32, 16, 44, 28, 35, 19, 47, 31,
    8,  56, 4,  52, 11, 59, 7,  55, 40, 24, 36, 20, 43, 27, 39, 23,
    2,  50, 14, 62, 1,  49, 13, 61, 34, 18, 46, 30, 33, 17, 45, 29,
    10, 58, 6,  54, 9,  57, 5,  53, 42, 26, 38, 22, 41, 25, 37, 21};
static int BAYER_8_8_SIZE[2] = {8, 8};

struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t ra;
    uint8_t ga;
    uint8_t ba;
};
typedef struct Pixel Pixel;

static FILE *g_pInFile;
static uint32_t g_nInFileSize;
static uint32_t g_nInFileOfs;
int width, height, comps;
uint8_t *pImage;

uint32_t getTime(char *label) {
    uint16_t startSec = (start & 31) * 2;
    uint16_t startMin = (start & 2016) >> 5;
    uint16_t startHour = (start & 63488) >> 11;
    uint16_t endSec = (end & 31) * 2;
    uint16_t endMin = (end & 2016) >> 5;
    uint16_t endHour = (end & 63488) >> 11;
    /* printf("%s start:%d   %dh %dm %ds\n", label, start,startHour,
     * startMin, startSec); */
    /* printf("%s end:%d  %dh %dm %ds\n", label, end, endHour, endMin,
     * endSec); */
    uint32_t sec = ((endHour * 3600 + endMin * 60 + endSec) -
                    (startHour * 3600 + startMin * 60 + startSec));
    printf("%s time:%d\n", label, sec);
}

void putPixel(uint16_t x, uint16_t y, uint16_t color) {
    unsigned short *screenPointer;
    unsigned short px;
    uint8_t i, r;
    unsigned long plans;
    unsigned short v;

    screenPointer = (unsigned short *)Physbase();
    plans = (y * 320L + x) / 16L;
    screenPointer += ((short)plans * 4);

    px = x % 16;
    v = (unsigned short)(power[px]);

    i = 0;
    while (color) {
        r = color % 2;
        if (r) {
            *(screenPointer + i) |= v;
        } else {
            *(screenPointer + i) &= (~v);
        }
        color /= 2;
        i++;
    }

    if (color < 2) {
        while (i < 4) {
            *(screenPointer + i) &= (~v);
            i++;
        }
    } else if (color < 4) {
        while (i < 3) {
            *(screenPointer + i) &= (~v);
            i++;
        }
    } else if (color < 8) {
        while (i < 1) {
            *(screenPointer + i) &= (~v);
            i++;
        }
    }
}

void getImagePixel(Pixel *p, uint16_t x, uint16_t y) {
    p->r = pImage[y * width * comps + x * comps];
    p->g = pImage[y * width * comps + x * comps + 1];
    p->b = pImage[y * width * comps + x * comps + 2];
    /* p->ra = (pImage[y * width * comps + x * comps] * 8) / 255; */
    /* p->ga = (pImage[y * width * comps + x * comps + 1] * 8) / 255; */
    /* p->ba = (pImage[y * width * comps + x * comps + 2] * 8) / 255;; */
}

void get_precalculated_matrix(short *matrix, int *matrix_size,
                              float *pre_calc_matrix) {
    for (int i = 0; i < matrix_size[1] * matrix_size[0]; i++)
        pre_calc_matrix[i] =
            (matrix[i] + 0.0f) / (matrix_size[0] * matrix_size[1]);
}

uint16_t getColor(short colorIndex) { return Setcolor(colorIndex, -1); }

void savePalette() {
    for (int i = 0; i < 16; i++) {
        previousPalette[i] = getColor(i);
    }
}

void restorePalette() { Setpalette(previousPalette); }

void findNearestColor(Pixel *p, uint8_t *cp) {
    uint16_t dr, dg, db;
    uint8_t r, g, b, pr, pg, pb;
    uint16_t v = 65535;
    uint16_t diff;
    for (int i = 0; i < 16; i++) {
        r = (atariPalette[i] >> 8) & 7;
        g = (atariPalette[i] >> 4) & 7;
        b = (atariPalette[i] >> 0) & 7;

        dr = abs(p->ra - r);
        dg = abs(p->ga - g);
        db = abs(p->ba - b);

        diff = dr * dr + dg * dg + db * db;
        if (diff < v) {
            v = diff;
            *cp = i;
        }
    }
}

void preparePaletteCache() {
    Pixel p;
    uint8_t cp;
    for (int r = 0; r < 8; r++) {
        for (int g = 0; g < 8; g++) {
            for (int b = 0; b < 8; b++) {
                p.ra = r;
                p.ga = g;
                p.ba = b;
                findNearestColor(&p, &cp);
                paletteCache[r][g][b] = cp;
                /* printf("%d %d %d - %d= %d\n", r, g, b, cp,
                 * paletteCache[r][g][b]); */
            }
        }
    }
}

void bubbleSort(uint8_t arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void convertToAtariPalette(uint32_t srcPalette[16]) {
    uint8_t rt = 0, gt = 0, bt = 0;
    uint8_t rvalues[16], gvalues[16], bvalues[16];
    for (int i = 0; i < 16; i++) {
        uint8_t r = (uint8_t)(srcPalette[i] >> 16);
        uint8_t g = (uint8_t)(srcPalette[i] >> 8);
        uint8_t b = (uint8_t)(srcPalette[i]);
        rvalues[i] = r;
        gvalues[i] = g;
        bvalues[i] = b;
        r = (r * 7) / 255;
        g = (g * 7) / 255;
        b = (b * 7) / 255;
        atariPalette[i] = (r << 8) | (g << 4) | b;
    }

    bubbleSort(rvalues, 16);
    bubbleSort(gvalues, 16);
    bubbleSort(bvalues, 16);

    for (int i = 0; i < 15; i++) {
        uint8_t diffR = abs(rvalues[i + 1] - rvalues[i]);
        uint8_t diffG = abs(gvalues[i + 1] - gvalues[i]);
        uint8_t diffB = abs(bvalues[i + 1] - bvalues[i]);
        if (diffR > rt)
            rt = diffR;
        if (diffG > gt)
            gt = diffG;
        if (diffB > bt)
            bt = diffB;
    }
    threshold[0] = rt;
    threshold[1] = gt;
    threshold[2] = bt;
}

/* convert color to linear space, very slow on cpu without fpu */
/* int linearSpace(float *x, float *y) { */
/*     *x = *x / 255.0f; */
/*     if (*x <= 0.04045f) { */
/*         *y = *x / 12.92f; */
/*     } else { */
/*         *y = powf((*x + 0.055f) / 1.055f, 2.4f); */
/*     } */
/*     return (int) (roundf(*y * 255)); */
/* } */

void ordered_ditherf(float *matrix, int *matrix_size) {
    short index_color;
    float mv;
    short percent, last_percent;
    uint8_t r, g, b;
    Pixel p, q;
    uint8_t c;
    short n = matrix_size[0] * matrix_size[1];
    for (short y = 0; y < height; y++) {
        for (short x = 0; x < width; x++) {
            getImagePixel(&p, x, y);
            mv = matrix[((y % matrix_size[1]) * matrix_size[0]) +
                        (x % matrix_size[0])];

            q.r = max(min(p.r + mv * threshold[0], 255), 0);
            q.g = max(min(p.g + mv * threshold[1], 255), 0);
            q.b = max(min(p.b + mv * threshold[2], 255), 0);

            q.ra = (q.r * 7) / 255;
            q.ga = (q.g * 7) / 255;
            q.ba = (q.b * 7) / 255;
            findNearestColor(&q, &c);
            if (x < 320 && y < 200)
                putPixel(x, y, c);
        }
    }
}

void ordered_dither(short *matrix, int *matrix_size) {
    short index_color;
    float map_value;
    short percent, last_percent;
    uint8_t r, g, b;
    float v, w;
    Pixel p, q;
    uint8_t c;
    short n = matrix_size[0] * matrix_size[1];
    short mv, mvr, mvg, mvb;
    for (short y = 0; y < (height < 200 ? height : 200); y++) {
        for (short x = 0; x < (width < 320 ? width : 320); x++) {
            getImagePixel(&p, x, y);

            mv = matrix[((y % matrix_size[1]) * matrix_size[0]) +
                        (x % matrix_size[0])];
            mvr = mv * threshold[0];
            mvr /= n;
            mvg = mv * threshold[1];
            mvg /= n;
            mvb = mv * threshold[2];
            mvb /= n;

            q.r = max(min(p.r + mvr, 255), 0);
            q.g = max(min(p.g + mvg, 255), 0);
            q.b = max(min(p.b + mvb, 255), 0);

            q.ra = (q.r * 7) / 255;
            q.ga = (q.g * 7) / 255;
            q.ba = (q.b * 7) / 255;
            c = paletteCache[q.ra][q.ga][q.ba];
            /* findNearestColor(&q, &c); */
            /* if (x < 320 && y < 200) */
            putPixel(x, y, c);
        }
    }
}

void initialize_screen() {
    currentRes = Getrez();
    Setscreen(Logbase(), Physbase(), LOW_RES);
}

/* used to test math lib */
/* void draw_log_base_graph() { */
/*     uint16_t x, y; */
/*     double log_value; */
/*     double base = 2.71828; /1* Natural logarithm base (e) *1/ */

/*     for (x = 1; x < SCREEN_WIDTH; ++x) { */
/*         log_value = log(x) / log(base); */
/*         y = SCREEN_HEIGHT - (uint16_t)(log_value * 10); */
/*         if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) { */
/*             putPixel(x, y, 3); */
/*         } */
/*     } */
/* } */

/* used to test putpixel speed */
/* void fillScreen() { */
/*     uint8_t c = 0; */
/*     for (int y = 0; y < 200; y++) { */
/*         for (int x = 0; x < 320; x++) { */
/*             putPixel(x, y, c % 16); */
/*             c++; */
/*         } */
/*     } */
/* } */

int main(int argc, char **argv) {
    initialize_screen();
    savePalette();

    /* Test math lib */
    /* draw_log_base_graph(); */

    /* convertToAtariPalette(dawnbringer); */
    /* Setpalette(atariPalette); */

    /* preparePaletteCache(); */

    printf("Loading image\n");
    /* start = Tgettime(); */
    /* pImage = stbi_load("wheel.png", &width, &height, &comps, 3); */
    pImage = stbi_load(argc > 1 ? argv[1] : "", &width, &height, &comps, 3);
    /* end = Tgettime(); */
    /* getTime("Loading"); */

    if (!pImage) {
        printf("Error on loading image !\n");
        getchar();
        restorePalette();
        Setscreen(Logbase(), Physbase(), currentRes);
        return 0;
    }

    printf("Search for optimal palette\n");
    guess_palette_wu(pImage, width, height, comps, palette, 16);
    convertToAtariPalette(palette);
    Setpalette(atariPalette);
    preparePaletteCache();

    printf("Displaying image %p\n", pImage);

    /* float version */
    /* float *matrix = (float *) malloc (sizeof (float) * BAYER_2_2_SIZE[0] *
     * BAYER_2_2_SIZE[1]); */
    /* get_precalculated_matrix(BAYER_2_2, BAYER_2_2_SIZE, matrix); */
    /* free(matrix); */

    /* start = Tgettime(); */
    ordered_dither(BAYER_2_2, BAYER_2_2_SIZE);
    /* ordered_dither(BAYER_4_4, BAYER_4_4_SIZE); */
    /* ordered_dither(BAYER_8_8, BAYER_8_8_SIZE); */
    /* end = Tgettime(); */
    /* getTime("Dithering"); */

    /* Cnecin(); */
    getchar();
    restorePalette();
    Setscreen(Logbase(), Physbase(), currentRes);
    stbi_image_free(pImage);

    return 0;
}
