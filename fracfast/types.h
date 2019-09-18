
#ifndef TYPES_H
#define TYPES_H


#include <gmp.h>


// TODO: Add compiler flag to set this
typedef unsigned int iter_t;


struct Domain {
    double rMin, rMax;
    double iMin, iMax;
};

struct HighPrecDomain {
    mpf_t rMin, rMax;
    mpf_t iMin, iMax;
};


struct Resolution {
    unsigned int w;
    unsigned int h;
};


struct Range {
    unsigned int xMin, xMax;
    unsigned int yMin, yMax;
};


#endif  // TYPES_H