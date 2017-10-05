#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static unsigned char randu8() {
    return rand() % 256;
}

// create a texture of random colors
// internal format will be BGR
// each component is an unsigned 8-bit value
// texture will be stored in buffer
void texRandom2d(int width, int height, char *buffer) {
    int row, col;
    unsigned char *data = (void*)buffer;

    for (row = 0; row < height; ++row) {
        for (col = 0; col < width; ++col) {
            data[0] = randu8();
            data[1] = randu8();
            data[2] = randu8();
            data += 3;
        }
    }
}

// create a greyscale texture using perlin noise
// internal format will be BGR
// each component is an unsigned 8-bit value
// texture will be stored in buffer
void texPerlin2d(int width, int height, char *buffer, int griddist) {
    // calculate grid size
    int gridnx = 2 + width / griddist;
    int gridny = 2 + height / griddist;

    int gridn = gridnx * gridny;

    // init grid
    float *gridx = malloc(gridn * sizeof(float));
    float *gridy = malloc(gridn * sizeof(float));

    int row, col;
    for (row = 0; row < gridny; ++row) {
        for (col = 0; col < gridnx; ++col) {
            float angle = (((float)(rand() % 1000)) / 1000.0f) * 2 * 3.141592653;

            gridx[row * gridnx + col] = cos(angle);
            gridy[row * gridnx + col] = sin(angle);
        }
    }

    unsigned char *data = (void*)buffer;

    // computes the dot product of the distance and gradient vectors
    float dot(int ix, int iy, float x, float y) {
        float dx = x - ix;
        float dy = y - iy;

        float gx = gridx[iy * gridnx + ix];
        float gy = gridy[iy * gridnx + ix];

        return (dx * gx + dy * gy);
    }

    // linear interpolation
    float lerp(float a0, float a1, float w) {
        w = (1.0f - cos(3.141592653589f * w)) / 2;
        return (1.0f - w) * a0 + w * a1;
    }

    // compute perlin noise at coordinates x, y
    float perlin2(float x, float y) {
        // determine grid cell coordinates
        int x0 = (int)x;
        int x1 = x0 + 1;
        int y0 = (int)y;
        int y1 = y0 + 1;

        // determine interpolation weights
        float sx = x - x0;
        float sy = y - y0;

        // interpolate between grid point gradients
        float n0, n1, ix0, ix1, value;

        n0 = dot(x0, y0, x, y);
        n1 = dot(x1, y0, x, y);
        ix0 = lerp(n0, n1, sx);

        n0 = dot(x0, y1, x, y);
        n1 = dot(x1, y1, x, y);
        ix1 = lerp(n0, n1, sx);

        value = lerp(ix0, ix1, sy);
        return value;
    }

    // calculate pixel colors
    for (row = 0; row < height; ++row) {
        for(col = 0; col < width; ++col) {
            float val = 0.5f * perlin2((float)col / griddist, (float)row / griddist) + 0.5f;

            unsigned char valc = (unsigned char)(val * 255);

            unsigned char red = valc;
            unsigned char green = valc;
            unsigned char blue = valc;

            data[0] = red;
            data[1] = green;
            data[2] = blue;

            data += 3;
        }
    }

    free(gridx);
    free(gridy);
}

// create a greyscale texture by adding different perlin noise textures
// internal format will be BGR
// each component is an unsigned 8-bit value
// texture will be stored in buffer
void texPerlinGrad2d(int width, int height, unsigned char *buffer) {
    unsigned char *tmp = calloc(3 * width * height, sizeof(unsigned char));

    int i;
    for (i = 0; i < 3 * width * height; ++i) buffer[i] = 0;

    float minside = (width < height) ? width : height;

    float griddist = minside / 10.0f;

    float opacity = 0.5f;

    while (griddist > 1.0f) {
        texPerlin2d(width, height, tmp, griddist);

        // add texture to buffer with opacity
        int i;
        for (i = 0; i < 3 * width * height; ++i) {
            unsigned char bit = tmp[i];

            bit = (unsigned char)(bit * opacity);

            if ((int)buffer[i] + bit > 255) buffer[i] = 255;
            else buffer[i] += bit;
        }

        griddist /= 2.0f;
        opacity /= 2.0f;
    }

    free(tmp);
}

void texMarble2d(int width, int height, unsigned char *buffer) {
    unsigned char *data = buffer;

    float xperiod = 5.0;
    float yperiod = 5.0;

    float turbpower = 5.0;

    int row, col;
    for (row = 0; row < height; ++row) {
        for (col = 0; col < width; ++col) {
            unsigned char val = data[0];
            float valf = (float)val / 255.0f;

            float xy = (col * xperiod / width + row * yperiod / height + turbpower * valf);

            float sineval = fabs(sinf(xy * 3.141592653));

            val = (unsigned char)(sineval * 255);

            data[0] = val;
            data[1] = val;
            data[2] = val;

            data += 3;
        }
    }
}

static float texGrad(float x1, float y1, float x2, float y2, float x) {
    float r = (x - x1) / (x2 - x1);
    return y1 + (y2 - y1) * r;
}

// add fire gradient to a 2d greyscale texture
// internal format will be BGR
// each component is an unsigned 8-bit value
// texture will be stored in buffer
void texFireGradient2d(int width, int height, unsigned char *buffer) {
    unsigned char *data = buffer;

    int i;
    for (i = 0; i < width * height; ++i) {
        unsigned char val = data[0];
        float valf = (float)val / 255.0f;

        float redf   = valf;
        float greenf = valf;
        float bluef  = valf;

        if (valf > 0.6f) {
            redf   = 0.0f;
            greenf = 0.0f;
            bluef  = 0.0f;
        }
        else if (valf > 0.5f) {
            redf   = texGrad(0.6f, 0.0f, 0.5f, 1.0f, valf);
            greenf = 0.0f;
            bluef  = 0.0f;
        }
        else if (valf > 0.3f) {
            redf   = 1.0f;
            greenf = texGrad(0.5f, 0.0f, 0.3f, 1.0f, valf);
            bluef  = 0.0f;
        }
        else {
            redf   = 1.0f;
            greenf = 1.0f;
            bluef  = texGrad(0.3f, 0.0f, 0.0f, 1.0f, valf);
        }

        data[0] = (unsigned char)(bluef * 255);
        data[1] = (unsigned char)(greenf * 255);
        data[2] = (unsigned char)(redf * 255);
        
        data += 3;
    }
}
