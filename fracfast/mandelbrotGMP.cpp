
#include "mandelbrot.h"
#include "types.h"

#include <gmp.h>

#include <cstring>


// With border trace
void Mandelbrot::calcScreenGMP(const HighPrecDomain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    // const ShapeVector s = (data == nullptr ? ShapeVector() : *(ShapeVector*)data);
    const unsigned int dX = r.xMax - r.xMin,
                       dY = r.yMax - r.yMin;

    // Set border trace struct up
    HighPrecBorderTrace bt;
    mpf_inits(bt.rMin, bt.iMax, bt.cr, bt.ci, bt.zr, bt.zi, bt.zSquaredr, bt.zSquaredi, bt.dist, bt.pixelSize, NULL);

    // const double ps = (domain.rMax - domain.rMin) / (double)res.w;
    mpf_sub(bt.pixelSize, domain.rMax, domain.rMin);
    mpf_div_ui(bt.pixelSize, bt.pixelSize, res.w);
    
    // bt.rMin = domain.rMin; bt.iMax = domain.iMax;
    mpf_set(bt.rMin, domain.rMin);
    mpf_set(bt.iMax, domain.iMax);

    bt.pixels = pixels; bt.data = nullptr;
    bt.w = res.w; bt.h = res.h;
    bt.xMin = r.xMin; bt.xMax = r.xMax; bt.yMin = r.yMin; bt.yMax = r.yMax; bt.dX = dX; bt.dY = dY;

    // Border trace
    edgeInQueue(bt);
    while(!bt.pixelQueue.empty()) {
        checkNeighbors(bt, bt.pixelQueue.front());
        bt.pixelQueue.pop();
    }
    fillEmptyPixels(bt);

    return;

    // Prevent error
    pixels = (uint32_t*)data;
}


void Mandelbrot::calcScreenGMPBruteforce(const HighPrecDomain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    mpf_t cr, ci,
          zr, zi,
          zSquaredr, zSquaredi,
          dist,
          pixelSize;
    mpf_inits(cr, ci, zr, zi, zSquaredr, zSquaredi, dist, pixelSize, NULL);

    //pixelSize = (rMax - rMin) / xRes;
    mpf_sub(pixelSize, domain.rMax, domain.rMin);
    mpf_div_ui(pixelSize, pixelSize, res.w);

    iter_t n;
    for(unsigned int y = r.yMin; y < r.yMax; y++) {
        //ci = iMax + (y * pixelSize);
        mpf_mul_ui(ci, pixelSize, y);
        mpf_sub(ci, domain.iMax, ci);

        for(unsigned int x = r.xMin; x < r.xMax; x++) {
            //cr = rMin + (x * pixelSize);
            mpf_mul_ui(cr, pixelSize, x);
            mpf_add(cr, domain.rMin, cr);

            mpf_set_ui(zr, 0);
            mpf_set_ui(zi, 0);
            mpf_set_ui(zSquaredr, 0);
            mpf_set_ui(zSquaredi, 0);

            n = 0;
            for(; n < nMax; n++) {
                // if((zr * zr) + (zi * zi) < 4) break;
                mpf_add(dist, zSquaredr, zSquaredi);
                if(mpf_cmp_ui(dist, 4) > 0)
                    break;

                mpf_mul(zi, zr, zi);
                mpf_mul_ui(zi, zi, 2);
                mpf_sub(zr, zSquaredr, zSquaredi);

                mpf_add(zr, zr, cr);
                mpf_add(zi, zi, ci);

                mpf_mul(zSquaredr, zr, zr);
                mpf_mul(zSquaredi, zi, zi);
            }

            pixels[y * res.w + x] = calcColor(n);
        }
    }

    mpf_clears(cr, ci, zr, zi, zSquaredr, zSquaredi, dist, pixelSize, NULL);
    return;

    // Prevent error
    pixels = (uint32_t*)data;
}

// To call this function with initializer list (low precision domain)
void Mandelbrot::calcScreenGMP(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    HighPrecDomain d;
    mpf_inits(d.rMin, d.rMax, d.iMin, d.iMax, NULL);
    mpf_set_d(d.rMin, domain.rMin); mpf_set_d(d.rMax, domain.rMax); mpf_set_d(d.iMin, domain.iMin); mpf_set_d(d.iMax, domain.iMax);

    calcScreenGMP(d, res, r, data, pixels);

    mpf_clears(d.rMin, d.rMax, d.iMin, d.iMax, NULL);
}

void Mandelbrot::calcScreenGMP(const Domain& domain, const Resolution& res, uint32_t* pixels) const {
    calcScreenGMP(domain, res, {0, res.w, 0, res.h}, nullptr, pixels);
}
