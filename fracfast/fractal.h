
#ifndef FRACTAL_H
#define FRACTAL_H


#include "types.h"
#include "borderTrace.h"

#include <cstdint>
#include <list>
#include <array>


typedef std::list<std::array<double, 2>> Orbit;
typedef std::array<double, 2> Point;

enum class Fractals {
    None,
    Mandelbrot,
    Julia
};


class Fractal {
    public:
        Fractal();
        Fractal(iter_t n);
        Fractal(const Fractals f, const double rMin, const double rMax, const double iBase);
        virtual ~Fractal();

        void setLineDetail(const double d);

        void getDefaultDomain(double dd[3]) const;

        void setnMax(const iter_t n);
        iter_t getnMax() const;
        // void changenMax(const int n);

        virtual void calcScreen(const Domain& domain, const Resolution& res, const Range& range, void* data, uint32_t* pixels) const = 0;
        virtual void calcScreenGMP(const HighPrecDomain& domain, const Resolution& res, const Range& range, void* data, uint32_t* pixels) const = 0;
        virtual void calcScreenBruteforce(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const = 0;

        uint32_t* render(const Domain& domain, const Resolution& res, const Range& range, void* data) const;
        uint32_t* threadedRender(const Domain& domain, const Resolution& res, const Range& range, void* data, int cores = 8, int splits = 7) const;
        uint32_t* threadedRenderGMP(const HighPrecDomain& domain, const Resolution& res, const Range& range, void* data, int cores = 8, int splits = 7) const;
        uint32_t* threadedRenderBruteforce(const Domain& domain, const Resolution& res, const Range& range, void* data, int cores = 8, int splits = 7) const;
        // To support Range as optional argument, because can't set range to values in res in C++
        inline uint32_t* render(const Domain& domain, const Resolution& res, void* data) const;
        inline uint32_t* threadedRender(const Domain& domain, const Resolution& res, void* data, int cores = 8, int splits = 7) const;

        // virtual uint32_t calcPixel(const double z0[2]) const = 0;
        virtual uint32_t calcPixel(const double z0[2], void* data) const = 0;

        // virtual void deepenRender(uint32_t* pixels, const Domain& domain, const Resolution& res) const = 0;

        uint32_t calcColor(const iter_t n) const;

        virtual void calcOrbit(const double c[2], Orbit& points) const = 0;

        const Fractals fractalType;


    protected:
        // Width for exterior distance coloring. Higher value is more detail (thinner line)
        double lineDetail;
        
        iter_t nMax;

        // Border tracing functions
        uint32_t getColor(BorderTrace& bt, const unsigned int pixel) const;
        uint32_t getColor(BorderTrace& bt, const unsigned int x, const unsigned int y) const;
        uint32_t getColor(BorderTrace& bt, const unsigned int pixel[2]) const;
        void addQueue(BorderTrace& bt, const unsigned int pixel) const;
        void edgeInQueue(BorderTrace& bt) const;
        void checkNeighbors(BorderTrace& bt, const unsigned int pixel) const;
        void fillEmptyPixels(BorderTrace& bt) const;

        uint32_t calcGMPPixel(HighPrecBorderTrace& bt) const;
        uint32_t getColor(HighPrecBorderTrace& bt, const unsigned int pixel) const;
        void addQueue(HighPrecBorderTrace& bt, const unsigned int pixel) const;
        void edgeInQueue(HighPrecBorderTrace& bt) const;
        void checkNeighbors(HighPrecBorderTrace& bt, const unsigned int pixel) const;
        void fillEmptyPixels(HighPrecBorderTrace& bt) const;


    private:
        const double defaultDomain[3];  // rMin, rMax, iBase
};


#endif  // FRACTAL_H
