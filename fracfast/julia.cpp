
#include "julia.h"

#include <cmath>
#include <cstring>
#include <cstdint>


static double LINEWIDTH;


Julia::Julia() : Fractal(Fractals::Julia, -2.0, 2.0, 0.0) {
    // c[0] = 0.4;
    // c[1] = 0.325;
    c[0] = -0.4;
    c[1] = 0.6;
}

Julia::Julia(const double _c[2]) : Fractal(Fractals::Julia, -2.0, 2.0, 0.0) {
    c[0] = _c[0];
    c[1] = _c[1];
}

Julia::~Julia() {
    
}


void Julia::setC(const double c0, const double c1) {
    c[0] = c0;
    c[1] = c1;
}

void Julia::moveC(const double dX, const double dY) {
    c[0] += dX;
    c[1] += dY;
}

void Julia::getC(double& c0, double& c1) const {
    c0 = c[0];
    c1 = c[1];
}


void Julia::calcScreen(const Domain& domain, const Resolution& res, const Range& r, const double _c[2], void* data, uint32_t* pixels) {
    c[0] = _c[0];
    c[1] = _c[1];

    calcScreen(domain, res, r, data, pixels);
}


// void Julia::deepenRender(uint32_t* pixels, const Domain& domain, const Resolution& res) const {
//     return;
// }


// TODO: Fix distance coloring Julia sets
inline uint32_t Julia::colorDistance(const double d) const {
    if(d < LINEWIDTH)
        return 0x0;
    else
        return 0xFFFFFFFF;
}

// Exterior distance estimation
inline uint32_t Julia::calcDistance(const double z0[2]) const {
    double z[2] = {z0[0], z0[1]},
           zSquared[2] = {z0[0] * z0[0], z0[1] * z0[1]},
           dz[2] = {1.0, 0.0},
           dzNew;

    unsigned int n = 0;
    for(; n < nMax && zSquared[0] + zSquared[1] <= 4.0; n++) {
        //dz = (2.0 * z * dz);
        dzNew = 2.0 * ((z[0] * dz[0]) - (z[1] * dz[1]));
        dz[1] = 2.0 * ((z[0] * dz[1]) + (z[1] * dz[0]));
        dz[0] = dzNew;

        // z = z^2 + c
        z[1] = z[0] * z[1] * 2.0;
        z[0] = zSquared[0] - zSquared[1];
        z[0] += c[0];
        z[1] += c[1];

        zSquared[0] = z[0] * z[0];
        zSquared[1] = z[1] * z[1];
    }

    if(n == nMax)
        return 0x0;

    // Calculate modulus
    z[0] = sqrt(zSquared[0] + zSquared[1]);
    dz[0] = sqrt((dz[0] * dz[0]) + (dz[1] * dz[1]));
    return colorDistance((log(z[0] * z[0]) * z[0]) / dz[0]);
}

void Julia::calcScreenDistance(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    const double pixelSize = (domain.rMax - domain.rMin) / (double)res.w;

    LINEWIDTH = (domain.rMax - domain.rMin) / lineDetail;

    // Normal calculation
    double z[2];
    for(unsigned int y = r.yMin; y < r.yMax; y++) {
        z[1] = domain.iMax - (y * pixelSize);

        for(unsigned int x = r.xMin; x < r.xMax; x++) {
            z[0] = domain.rMin + (x * pixelSize);
            pixels[y * res.w + x] = calcDistance(z);
        }
    }

    // return pixels;
    return;

    // Prevent error
    z[0] = *(double*)data;
}


uint32_t Julia::calcPixel(const double z0[2], void* data) const {
    double zSquared[2] = {z0[0] * z0[0], z0[1] * z0[1]};

    // Points outside radius 2 are not part of the set, so shouldn't be black
    if(zSquared[0] + zSquared[1] > 4.0)
        return calcColor(1);

    double z[2] = {z0[0], z0[1]};
    unsigned int n = 0;
    for(; n < nMax && zSquared[0] + zSquared[1] <= 4.0; n++) {
        z[1] = z[0] * z[1] * 2.0;
        z[0] = zSquared[0] - zSquared[1];

        z[0] += c[0];
        z[1] += c[1];

        zSquared[0] = z[0] * z[0];
        zSquared[1] = z[1] * z[1];
    }

    return calcColor(n);

    // Prevent error
    n = *(uint32_t*)data;
}


// With border trace and caching
void Julia::calcScreen(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    const double ps = (domain.rMax - domain.rMin) / (double)res.w;
    const unsigned int dX = r.xMax - r.xMin,
                       dY = r.yMax - r.yMin;

    // Set border trace struct up
    BorderTrace bt;
    bt.pixels = pixels; bt.pixelSize = ps; bt.data = nullptr;
    bt.rMin = domain.rMin; bt.iMax = domain.iMax;
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

    // Prevent warning
    pixels[0] = *(uint32_t*)data;
}


void Julia::calcScreenGMP(const HighPrecDomain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    return;

    // Prevent error
    mpf_t a;mpf_init(a);mpf_add(a, domain.rMax, domain.rMin);mpf_clear(a); int i = res.w; i = r.xMin; int* j = ((int*)&data); pixels[0] = i; pixels[0] = *j;
}


void Julia::calcScreenBruteforce(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    const double pixelSize = (domain.rMax - domain.rMin) / (double)res.w;

    double z[2];
    for(unsigned int y = r.yMin; y < r.yMax; y++) {
        z[1] = domain.iMax - (y * pixelSize);

        for(unsigned int x = r.xMin; x < r.xMax; x++) {
            z[0] = domain.rMin + (x * pixelSize);
            pixels[y * res.w + x] = calcPixel(z, data);
        }
    }

    // return pixels;
    return;

    z[0] = *(uint32_t*)data;
}


void Julia::calcOrbit(const double z0[2], Orbit& points) const {
    points.push_back({z0[0], z0[1]});
    double zSquared[2] = {z0[0] * z0[0], z0[1] * z0[1]};

    double z[2] = {z0[0], z0[1]};
    unsigned int n = 0;
    for(; n < nMax && zSquared[0] + zSquared[1] <= 4.0; n++) {
        z[1] = z[0] * z[1] * 2.0;
        z[0] = zSquared[0] - zSquared[1];

        z[0] += c[0];
        z[1] += c[1];

        zSquared[0] = z[0] * z[0];
        zSquared[1] = z[1] * z[1];

        points.push_back({z[0], z[1]});
    }
}
