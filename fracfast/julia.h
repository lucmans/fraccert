
#ifndef JULIA_H
#define JULIA_H


#include "fractal.h"
#include "types.h"

#include <cstdint>


class Julia : public Fractal {
    public:
        Julia();
        Julia(const double c[2]);
        ~Julia();

        void setC(const double c0, const double c1);
        void moveC(const double dX, const double dY);
        void getC(double& c0, double& c1) const;

        // void deepenRender(uint32_t* pixels, const Domain& domain, const Resolution& res) const;

        inline uint32_t colorDistance(const double d) const;
        inline uint32_t calcDistance(const double z0[2]) const;
        void calcScreenDistance(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const;

        uint32_t calcPixel(const double z0[2], void* data) const;

        void calcScreen(const Domain& domain, const Resolution& res, const Range& r, const double _c[2], void* data, uint32_t* pixels);
        void calcScreen(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const;
        void calcScreenBruteforce(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const;

        void calcScreenGMP(const HighPrecDomain& domain, const Resolution& res, const Range& range, void* data, uint32_t* pixels) const;

        void calcOrbit(const double z0[2], Orbit& points) const;



    private:
        double c[2];
};


#endif  // JULIA_H
