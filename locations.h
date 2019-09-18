
#ifndef LOCATIONS_H
#define LOCATIONS_H


#include "fracfast/types.h"


struct Location {
    Domain dom;
    Resolution res;
    iter_t nMax;
};


namespace Locations {
    extern const Location home, limit, sym,
                          a, b, c, d, e, f, g, h, i;

    extern const Resolution averageRes;
}


#endif  // LOCATIONS_H
