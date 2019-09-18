
// #include "borderTrace.h"

#include "fractal.h"
#include "shapes.h"

#include <queue>
#include <cstdint>


// TODO: See which getColor() is faster
uint32_t Fractal::getColor(BorderTrace& bt, const unsigned int pixel) const {
    if(bt.pixels[pixel] & COLORED)
        return bt.pixels[pixel] & COLOR;

    const unsigned int x = pixel % bt.w,
                       y = pixel / bt.w;
    
    double c[2];
    c[0] = bt.rMin + (x * bt.pixelSize);
    c[1] = bt.iMax - (y * bt.pixelSize);

    bt.pixels[pixel] = calcPixel(c, bt.data) | COLORED;

    return bt.pixels[pixel] & COLOR;
}

uint32_t Fractal::getColor(BorderTrace& bt, const unsigned int x, const unsigned int y) const {
    const unsigned int _p = y * bt.w + x;

    if(bt.pixels[_p] & COLORED)
        return bt.pixels[_p] & COLOR;
    
    double c[2];
    c[0] = bt.rMin + ((x + bt.xMin) * bt.pixelSize);
    c[1] = bt.iMax - ((y + bt.yMin) * bt.pixelSize);

    bt.pixels[_p] = calcPixel(c, bt.data) | COLORED;

    return bt.pixels[_p] & COLOR;
}

uint32_t Fractal::getColor(BorderTrace& bt, const unsigned int pixel[2]) const {
    const unsigned int _p = pixel[1] * bt.w + pixel[0];

    if(bt.pixels[_p] & COLORED)
        return bt.pixels[_p] & COLOR;
    
    double c[2];
    c[0] = bt.rMin + ((pixel[0] + bt.xMin) * bt.pixelSize);
    c[1] = bt.iMax - ((pixel[1] + bt.yMin) * bt.pixelSize);

    bt.pixels[_p] = calcPixel(c, bt.data) | COLORED;

    return bt.pixels[_p] & COLOR;
}


void Fractal::addQueue(BorderTrace& bt, const unsigned int pixel) const {
    if(bt.pixels[pixel] & QUEUED)
        return;

    bt.pixelQueue.push(pixel);
    bt.pixels[pixel] |= QUEUED;
}


void Fractal::edgeInQueue(BorderTrace& bt) const {
    // This function is only called at start of border trace, so clear queue.
    bt.pixelQueue = std::queue<unsigned int>();
    
    for(unsigned int y = bt.yMin; y < bt.yMax; y++) {
        addQueue(bt, y * bt.w + bt.xMin);
        addQueue(bt, y * bt.w + bt.xMin + (bt.dX - 1));
        // bt.pixels[y * bt.w + bt.xMin] = 0xFFFFFFFF;  // Color borders white
        // bt.pixels[y * bt.w + bt.xMin + (bt.dX - 1)] = 0xFFFFFFFF;
    }
    for(unsigned int x = bt.xMin + 1; x < bt.xMax - 1; x++) {
        addQueue(bt, x + (bt.yMin * bt.w));
        addQueue(bt, x + (bt.yMin * bt.w) + ((bt.dY - 1) * bt.w));
        // bt.pixels[x + (bt.yMin * bt.w)] = 0xFFFFFFFF;
        // bt.pixels[x + (bt.yMin * bt.w) + ((bt.dY - 1) * bt.w)] = 0xFFFFFFFF;
    }
}

void Fractal::checkNeighbors(BorderTrace& bt, const unsigned int pixel) const {
    const unsigned int x = pixel % bt.w,
                       y = pixel / bt.w;

    // Calculate current pixel
    const uint32_t pixelColor = getColor(bt, pixel);

    // Bools for existence of left-, right-, up- and down-neighbor
    // Cache the results, because they are used often
    const bool rightExists = x < bt.xMax - 1,
               leftExists = x > bt.xMin,
               downExists = y < bt.yMax - 1,
               upExists = y > bt.yMin;

    // First calculate 4 the neighbors and check if they are different
    bool rightDifferent = false, leftDifferent = false, downDifferent = false, upDifferent = false;
    if(rightExists)
        rightDifferent = getColor(bt, pixel + 1) != pixelColor;
    if(leftExists)
        leftDifferent = getColor(bt, pixel - 1) != pixelColor;
    if(downExists)
        downDifferent = getColor(bt, pixel + bt.w) != pixelColor;
    if(upExists)
        upDifferent = getColor(bt, pixel - bt.w) != pixelColor;

    // Check neighbors of the neighbors which are different
    if(rightDifferent)
        addQueue(bt, pixel + 1);
    if(leftDifferent)
        addQueue(bt, pixel - 1);
    if(downDifferent)
        addQueue(bt, pixel + bt.w);
    if(upDifferent)
        addQueue(bt, pixel - bt.w);

    // Same for diagonals
    bool rdDifferent = false, ruDifferent = false, ldDifferent = false, luDifferent = false;
    if(rightExists && downExists)
        rdDifferent = getColor(bt, pixel + bt.w + 1) != pixelColor;
    if(rightExists && upExists)
        ruDifferent = getColor(bt, pixel - bt.w + 1) != pixelColor;
    if(leftExists && downExists)
        ldDifferent = getColor(bt, pixel + bt.w - 1) != pixelColor;
    if(leftExists && upExists)
        luDifferent = getColor(bt, pixel - bt.w - 1) != pixelColor;

    if(rdDifferent)
        addQueue(bt, pixel + bt.w + 1);
    if(ruDifferent)
        addQueue(bt, pixel - bt.w + 1);
    if(ldDifferent)
        addQueue(bt, pixel + bt.w - 1);
    if(luDifferent)
        addQueue(bt, pixel - bt.w - 1);

    // Optimization which intoduces more error
    // If any neighbor is different, then also add the diagonals touching this neighbor
    // if((rightExists && downExists) && (rightDifferent || downDifferent))
    //     addQueue(bt, pixel + bt.w + 1);
    // if((rightExists && upExists) && (rightDifferent || upDifferent))
    //     addQueue(bt, pixel - bt.w + 1);
    // if((leftExists && downExists) && (leftDifferent || downDifferent))
    //     addQueue(bt, pixel + bt.w - 1);
    // if((leftExists && upExists) && (leftDifferent || upDifferent))
    //     addQueue(bt, pixel - bt.w - 1);
}


void Fractal::fillEmptyPixels(BorderTrace& bt) const {
    unsigned int pix;
    for(unsigned int y = bt.yMin; y < bt.yMax; y++) {
        for(unsigned int x = bt.xMin + 1; x < bt.xMax; x++) {
            pix = y * bt.w + x;
            if(!(bt.pixels[pix] & COLORED))
                bt.pixels[pix] = bt.pixels[pix - 1];

            // Maybe clear alpha bits?
            // bt.pixels[pix] &= 0xFFFFFF00;
        }
    }
}



uint32_t Fractal::calcGMPPixel(HighPrecBorderTrace& bt) const {
    mpf_set_ui(bt.zr, 0);
    mpf_set_ui(bt.zi, 0);
    mpf_set_ui(bt.zSquaredr, 0);
    mpf_set_ui(bt.zSquaredi, 0);

    iter_t n = 0;
    for(; n < nMax; n++) {
        // if((zr * zr) + (zi * zi) < 4) break;
        mpf_add(bt.dist, bt.zSquaredr, bt.zSquaredi);
        if(mpf_cmp_ui(bt.dist, 4) > 0)
            break;

        mpf_mul(bt.zi, bt.zr, bt.zi);
        mpf_mul_ui(bt.zi, bt.zi, 2);
        mpf_sub(bt.zr, bt.zSquaredr, bt.zSquaredi);

        mpf_add(bt.zr, bt.zr, bt.cr);
        mpf_add(bt.zi, bt.zi, bt.ci);

        mpf_mul(bt.zSquaredr, bt.zr, bt.zr);
        mpf_mul(bt.zSquaredi, bt.zi, bt.zi);
    }

    return calcColor(n);
}

uint32_t Fractal::getColor(HighPrecBorderTrace& bt, const unsigned int pixel) const {
    if(bt.pixels[pixel] & COLORED)
        return bt.pixels[pixel] & COLOR;

    const unsigned int x = pixel % bt.w,
                       y = pixel / bt.w;

    // c[0] = bt.rMin + (x * bt.pixelSize);
    mpf_mul_ui(bt.cr, bt.pixelSize, x);
    mpf_add(bt.cr, bt.rMin, bt.cr);

    // c[1] = bt.iMax - (y * bt.pixelSize);
    mpf_mul_ui(bt.ci, bt.pixelSize, y);
    mpf_sub(bt.ci, bt.iMax, bt.ci);

    bt.pixels[pixel] = calcGMPPixel(bt) | COLORED;

    return bt.pixels[pixel] & COLOR;
}


// TODO: Use templates to remove duplicate code
void Fractal::addQueue(HighPrecBorderTrace& bt, const unsigned int pixel) const {
    if(bt.pixels[pixel] & QUEUED)
        return;

    bt.pixelQueue.push(pixel);
    bt.pixels[pixel] |= QUEUED;
}


void Fractal::edgeInQueue(HighPrecBorderTrace& bt) const {
    // This function is only called at start of border trace, so clear queue.
    bt.pixelQueue = std::queue<unsigned int>();
    
    for(unsigned int y = bt.yMin; y < bt.yMax; y++) {
        addQueue(bt, y * bt.w + bt.xMin);
        addQueue(bt, y * bt.w + bt.xMin + (bt.dX - 1));
    }
    for(unsigned int x = bt.xMin + 1; x < bt.xMax - 1; x++) {
        addQueue(bt, x + (bt.yMin * bt.w));
        addQueue(bt, x + (bt.yMin * bt.w) + ((bt.dY - 1) * bt.w));
    }
}


void Fractal::checkNeighbors(HighPrecBorderTrace& bt, const unsigned int pixel) const {
    const unsigned int x = pixel % bt.w,
                       y = pixel / bt.w;

    // Calculate current pixel
    const uint32_t pixelColor = getColor(bt, pixel);

    // Bools for existence of left-, right-, up- and down-neighbor
    // Cache the results, because they are used often
    const bool rightExists = x < bt.xMax - 1,
               leftExists = x > bt.xMin,
               downExists = y < bt.yMax - 1,
               upExists = y > bt.yMin;

    // First calculate 4 the neighbors and check if they are different
    bool rightDifferent = false, leftDifferent = false, downDifferent = false, upDifferent = false;
    if(rightExists)
        rightDifferent = getColor(bt, pixel + 1) != pixelColor;
    if(leftExists)
        leftDifferent = getColor(bt, pixel - 1) != pixelColor;
    if(downExists)
        downDifferent = getColor(bt, pixel + bt.w) != pixelColor;
    if(upExists)
        upDifferent = getColor(bt, pixel - bt.w) != pixelColor;

    // Check neighbors of the neighbors which are different
    if(rightDifferent)
        addQueue(bt, pixel + 1);
    if(leftDifferent)
        addQueue(bt, pixel - 1);
    if(downDifferent)
        addQueue(bt, pixel + bt.w);
    if(upDifferent)
        addQueue(bt, pixel - bt.w);

    // If any neighbor is different, then also add the diagonals touching this neighbor
    if((rightExists && downExists) && (rightDifferent || downDifferent))
        addQueue(bt, pixel + bt.w + 1);
    if((rightExists && upExists) && (rightDifferent || upDifferent))
        addQueue(bt, pixel - bt.w + 1);
    if((leftExists && downExists) && (leftDifferent || downDifferent))
        addQueue(bt, pixel + bt.w - 1);
    if((leftExists && upExists) && (leftDifferent || upDifferent))
        addQueue(bt, pixel - bt.w - 1);
}

void Fractal::fillEmptyPixels(HighPrecBorderTrace& bt) const {
    unsigned int pix;
    for(unsigned int y = bt.yMin; y < bt.yMax; y++) {
        for(unsigned int x = bt.xMin + 1; x < bt.xMax; x++) {
            pix = y * bt.w + x;
            if(!(bt.pixels[pix] & COLORED))
                bt.pixels[pix] = bt.pixels[pix - 1];

            // Maybe clear alpha bits?
            // bt.pixels[pix] &= 0xFFFFFF00;
        }
    }
}
