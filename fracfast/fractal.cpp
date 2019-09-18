
#include "fractal.h"
#include "borderTrace.cpp"

#include <omp.h>

#include <cstring>
#include <vector>


Fractal::Fractal() : fractalType(Fractals::None), defaultDomain{-2, 2, 0} {
    nMax = 256;
    lineDetail = 5000;
}

Fractal::Fractal(iter_t n) : fractalType(Fractals::None), defaultDomain{-2, 2, 0} {
    nMax = n;
    lineDetail = 5000;
}

Fractal::Fractal(const Fractals f, const double rMin, const double rMax, const double iBase) : fractalType(f), defaultDomain{rMin, rMax, iBase} {
    nMax = 256;
    lineDetail = 5000;
}

Fractal::~Fractal() {
}


void Fractal::setLineDetail(const double d) {
    lineDetail = d;
}


void Fractal::getDefaultDomain(double dd[3]) const {
    for(int i = 0; i < 3; i++) {
        dd[i] = defaultDomain[i];
    }
}


void Fractal::setnMax(iter_t n) {
    nMax = n;
}

iter_t Fractal::getnMax() const {
    return nMax;
}

// void Fractal::changenMax(const int n) {
//     // TODO: underflow detection!
//     nMax += n;
// }


uint32_t* Fractal::render(const Domain& domain, const Resolution& res, const Range& range, void* data) const {
    uint32_t* pixels = new uint32_t[res.w * res.h];
    memset(pixels, 0x0, res.w * res.h * sizeof(uint32_t));  // Init all pixel 0, because least significant byte is used for control flow in border trace

    // HighPrecDomain d;
    // mpf_inits(d.rMin, d.rMax, d.iMin, d.iMax, NULL);
    // mpf_set_d(d.rMin, domain.rMin); mpf_set_d(d.rMax, domain.rMax); mpf_set_d(d.iMin, domain.iMin); mpf_set_d(d.iMax, domain.iMax);
    // calcScreenGMP(d, res, range, data, pixels);
    // mpf_clears(d.rMin, d.rMax, d.iMin, d.iMax, NULL);
    calcScreen(domain, res, range, data, pixels);

    return pixels;
}


uint32_t* Fractal::threadedRender(const Domain& domain, const Resolution& res, const Range& range, void* data, int cores, int splits) const {
    uint32_t* sharedPixels = new uint32_t[res.w * res.h];
    memset(sharedPixels, 0x0, res.w * res.h * sizeof(uint32_t));  // Init all pixel 0, because least significant byte is used for control flow in border trace
    
    // int cores = omp_get_num_procs();
    // int splits = 6;  // n splits leads to 2^n blocks

    // Split screen in smaller blocks
    std::vector<Range> blocks = {range};
    for(int i = 0; i < splits; i++) {
        std::vector<Range> newBlocks;
        for(auto& b : blocks) {
            if(b.xMax - b.xMin > b.yMax - b.yMin) {  // If there are more pixels in the x axis, split it in 2
                newBlocks.push_back({b.xMin, b.xMin + ((b.xMax - b.xMin) / 2), b.yMin, b.yMax});
                newBlocks.push_back({b.xMin + ((b.xMax - b.xMin) / 2), b.xMax, b.yMin, b.yMax});
            }
            else {
                newBlocks.push_back({b.xMin, b.xMax, b.yMin, b.yMin + ((b.yMax - b.yMin) / 2)});
                newBlocks.push_back({b.xMin, b.xMax, b.yMin + ((b.yMax - b.yMin) / 2), b.yMax});
            }
        }
        blocks = newBlocks;
    }

    // Concurrently calculate all blocks
    int lastBlock = 0;
    int totalBlocks = 1 << splits;
    #pragma omp parallel num_threads(cores)
    {
        while(true) {
            int blocknum;
            #pragma omp critical
            {
               blocknum = lastBlock;
               lastBlock++;
            }
            if(blocknum >= totalBlocks)
                break;

            calcScreen(domain, res, blocks[blocknum], data, sharedPixels);
        }
    }

    return sharedPixels;
}


uint32_t Fractal::calcColor(const iter_t n) const {
    uint32_t rgba = 0;
    const double t = n / (double)nMax;
    // const int t = n % 256;

    rgba |= (uint8_t)(9 * (1 - t) * t * t * t * 255) << 24;  // r
    rgba |= (uint8_t)(15 * (1 - t) * (1 - t) * t * t * 255) << 16;  // g
    rgba |= (uint8_t)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255) << 8;  // b

    return rgba;
}

// Ugly coloring from thesis
// Use nMax = 500 for graphics in thesis
// uint32_t Fractal::calcColor(const iter_t n) const {
//     double t = n / (double)nMax;
//     return (uint32_t)(t * 256 * 256 * 256) << 8;

//     // This method was also used for the graphics in the thesis, it changes red->blue, blue->green and green->red, because it prettier
//     uint32_t rgba = (uint32_t)(t * 256 * 256 * 256) << 8;
//     uint8_t r = (rgba & 0xFF000000) >> 24;
//     uint8_t g = (rgba & 0x00FF0000) >> 16;
//     uint8_t b = (rgba & 0x0000FF00) >> 8;
    
//     return (r << 8) | (g << 24) | (b << 16);
// }

// Black-white coloring
// uint32_t Fractal::calcColor(const iter_t n) const {
//     if(n == nMax)
//         return 0x0;
//     else
//         return 0xFFFFFFFF;
// }


uint32_t* Fractal::threadedRenderGMP(const HighPrecDomain& domain, const Resolution& res, const Range& range, void* data, int cores, int splits) const {
    uint32_t* sharedPixels = new uint32_t[res.w * res.h];
    memset(sharedPixels, 0x0, res.w * res.h * sizeof(uint32_t));  // Init all pixel 0, because least significant byte is used for control flow in border trace
    
    // int cores = omp_get_num_procs();
    // int splits = 6;  // n splits leads to 2^n blocks

    // Split screen in smaller blocks
    std::vector<Range> blocks = {range};
    for(int i = 0; i < splits; i++) {
        std::vector<Range> newBlocks;
        for(auto& b : blocks) {
            if(b.xMax - b.xMin > b.yMax - b.yMin) {  // If there are more pixels in the x axis, split it in 2
                newBlocks.push_back({b.xMin, b.xMin + ((b.xMax - b.xMin) / 2), b.yMin, b.yMax});
                newBlocks.push_back({b.xMin + ((b.xMax - b.xMin) / 2), b.xMax, b.yMin, b.yMax});
            }
            else {
                newBlocks.push_back({b.xMin, b.xMax, b.yMin, b.yMin + ((b.yMax - b.yMin) / 2)});
                newBlocks.push_back({b.xMin, b.xMax, b.yMin + ((b.yMax - b.yMin) / 2), b.yMax});
            }
        }
        blocks = newBlocks;
    }

    // Concurrently calculate all blocks
    int lastBlock = 0;
    int totalBlocks = 1 << splits;
    #pragma omp parallel num_threads(cores)
    {
        while(true) {
            int blocknum;
            #pragma omp critical
            {
               blocknum = lastBlock;
               lastBlock++;
            }
            if(blocknum >= totalBlocks)
                break;

            calcScreenGMP(domain, res, blocks[blocknum], data, sharedPixels);
        }
    }

    return sharedPixels;
}

uint32_t* Fractal::threadedRenderBruteforce(const Domain& domain, const Resolution& res, const Range& range, void* data, int cores, int splits) const {
    uint32_t* sharedPixels = new uint32_t[res.w * res.h];
    memset(sharedPixels, 0x0, res.w * res.h * sizeof(uint32_t));  // Init all pixel 0, because least significant byte is used for control flow in border trace
    
    // Split screen in smaller blocks
    std::vector<Range> blocks = {range};
    for(int i = 0; i < splits; i++) {
        std::vector<Range> newBlocks;
        for(auto& b : blocks) {
            if(b.xMax - b.xMin > b.yMax - b.yMin) {  // If there are more pixels in the x axis, split it in 2
                newBlocks.push_back({b.xMin, b.xMin + ((b.xMax - b.xMin) / 2), b.yMin, b.yMax});
                newBlocks.push_back({b.xMin + ((b.xMax - b.xMin) / 2), b.xMax, b.yMin, b.yMax});
            }
            else {
                newBlocks.push_back({b.xMin, b.xMax, b.yMin, b.yMin + ((b.yMax - b.yMin) / 2)});
                newBlocks.push_back({b.xMin, b.xMax, b.yMin + ((b.yMax - b.yMin) / 2), b.yMax});
            }
        }
        blocks = newBlocks;
    }

    // Concurrently calculate all blocks
    int lastBlock = 0;
    int totalBlocks = 1 << splits;
    #pragma omp parallel num_threads(cores)
    {
        while(true) {
            int blocknum;
            #pragma omp critical
            {
               blocknum = lastBlock;
               lastBlock++;
            }
            if(blocknum >= totalBlocks)
                break;

            calcScreenBruteforce(domain, res, blocks[blocknum], data, sharedPixels);

        }
    }

    return sharedPixels;
}


inline uint32_t* Fractal::render(const Domain& domain, const Resolution& res, void* data) const {
    return render(domain, res, {0, res.w, 0, res.h}, data);
}

inline uint32_t* Fractal::threadedRender(const Domain& domain, const Resolution& res, void* data, int cores, int splits) const {
    return threadedRender(domain, res, {0, res.w, 0, res.h}, data, cores, splits);
}
