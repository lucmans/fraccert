
#ifndef BORDER_TRACE
#define BORDER_TRACE


#include <queue>
#include <cstdint>

#include "shapes.h"


#define COLORED 0b01
#define QUEUED  0b10

#define COLOR 0xFFFFFF00


struct BorderTrace {
    std::queue<unsigned int> pixelQueue;
    uint32_t* pixels;
    double rMin, iMax, pixelSize;
    unsigned int w, h;
    unsigned int xMin, xMax, yMin, yMax, dX, dY;
    void* data;
};

struct HighPrecBorderTrace {
    std::queue<unsigned int> pixelQueue;
    uint32_t* pixels;
    mpf_t rMin, iMax, pixelSize;
    unsigned int w, h;
    unsigned int xMin, xMax, yMin, yMax, dX, dY;
    void* data;

    // "Global", so they don't have to be initialized every function call
    mpf_t zr, zi,
          cr, ci,
          zSquaredr, zSquaredi,
          dist;
};


#endif  // BORDER_TRACE
