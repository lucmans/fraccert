
#ifndef MANDELBROT_H
#define MANDELBROT_H


#include "fractal.h"
#include "shapes.h"
#include "types.h"

#include <cstdint>


class Mandelbrot : public Fractal {
    public:
        Mandelbrot();
        ~Mandelbrot();

        // Escape time coloring with bordertrace + symmetry
        void calcScreen(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const;
        // With shape checking
        uint32_t calcPixel(const double c[2], void* data) const;

        // Distance estimation coloring
        inline uint32_t colorDistance(const double d) const;
        inline uint32_t calcDistance(const double c[2], const ShapeVector& shapes) const;
        void calcScreenDistance(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const;


        // void deepenRender(uint32_t* pixels, const Domain& domain, const Resolution& res) const;

        void calcScreenGMP(const HighPrecDomain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const;
        void calcScreenGMPBruteforce(const HighPrecDomain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const;
        
void calcScreenGMP(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const;
void calcScreenGMP(const Domain& domain, const Resolution& res, uint32_t* pixels) const;


        // Different variants
        void calcScreenBruteforce(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const;
        inline uint32_t calcPixelNoShape(const double c[2]) const;
        void calcScreenBruteforceNoShape(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const;
        uint32_t calcPixelShapeWrong(const double c[2], void* data) const;
        void calcScreenShapeWrong(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const;

        void calcOrbit(const double c[2], Orbit& points) const;



    private: 
};


#endif  // MANDELBROT_H
