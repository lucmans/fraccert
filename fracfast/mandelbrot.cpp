
#include "mandelbrot.h"
#include "mandelbrotGMP.cpp"

#include "types.h"

#include <cstring>
#include <cstdint>
#include <cmath>


// TODO: Put most efficient calcScreen function here. This allows us to easily change calcScreen to a more efficient one without changing any other code
// const auto calcScreen = newIdFromDatabase;


static double LINEWIDTH;


Mandelbrot::Mandelbrot() : Fractal(Fractals::Mandelbrot, -2.0, 1.0, 0.0) {

}

Mandelbrot::~Mandelbrot() {
    
}


// void Mandelbrot::deepenRender(uint32_t* pixels, const Domain& domain, const Resolution& res) const {
//     const double pixelSize = (domain.rMax - domain.rMin) / (double)res.w;

//     double z0[2];
//     for(unsigned int y = 0; y < res.h; y++) {
//         z0[1] = domain.iMax - (y * pixelSize);
        
//         for(unsigned int x = 0; x < res.w; x++) {
//             if(pixels[y * res.w + x] == 0x0) {
//                 z0[0] = domain.rMin + (x * pixelSize);
//                 pixels[y * res.w + x] = calcPixel(z0);
//             }
//         }
//     }
// }


inline uint32_t Mandelbrot::colorDistance(const double d) const {
    if(d < LINEWIDTH)
        return 0x0;
    else
        return 0xFFFFFF00;
}

// Exterior distance estimation
inline uint32_t Mandelbrot::calcDistance(const double c[2], const ShapeVector& shapes) const {
    // Check shapes
    for(auto& inShape : shapes)
        if(inShape(c))
            return 0x0;

    double z[2] = {0, 0}, zSquared[2] = {0, 0};
    double dzNew, dz[2] = {0, 0};

    iter_t n = 0;
    for(; n < nMax && zSquared[0] + zSquared[1] <= 4.0; n++) {
        //dz = (2.0 * z * dz) + 1.0;
        // TODO dzNew = fma(2.0, ((z[0] * dz[0]) - (z[1] * dz[1])), 1.0);  // fma(a, b, c) -> a * b + c
        dzNew = 2.0 * ((z[0] * dz[0]) - (z[1] * dz[1])) + 1.0;
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

void Mandelbrot::calcScreenDistance(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    const ShapeVector shapes = (data == nullptr ? ShapeVector() : *(ShapeVector*)data);
    const double pixelSize = (domain.rMax - domain.rMin) / (double)res.w;

    LINEWIDTH = (domain.rMax - domain.rMin) / lineDetail;

    // Normal calculation
    double c[2];
    for(unsigned int y = r.yMin; y < r.yMax; y++) {
        c[1] = domain.iMax - (y * pixelSize);
        
        for(unsigned int x = r.xMin; x < r.xMax; x++) {
            c[0] = domain.rMin + (x * pixelSize);

            pixels[y * res.w + x] = calcDistance(c, shapes);
        }
    }
}


// With caching and shape checking
uint32_t Mandelbrot::calcPixel(const double c[2], void* data) const {
    // Check shapes
    ShapeVector shapes = *(ShapeVector*)data;
    for(auto& inShape : shapes)
        if(inShape(c))
            return 0x0;

    double z[2] = {0, 0};
    double zSquared[2] = {0, 0};  // caches squares of real and imaginary part

    // Escape iteration loop
    iter_t n = 0;
    for(; n < nMax && zSquared[0] + zSquared[1] <= 4.0; n++) {
        z[1] = z[0] * z[1] * 2.0;
        z[0] = zSquared[0] - zSquared[1];

        z[0] += c[0];
        z[1] += c[1];

        zSquared[0] = z[0] * z[0];
        zSquared[1] = z[1] * z[1];
    }

    return calcColor(n);
}


// With border trace + shape checking
void Mandelbrot::calcScreen(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    const ShapeVector s = (data == nullptr ? ShapeVector() : *(ShapeVector*)data);

    const double ps = (domain.rMax - domain.rMin) / (double)res.w;  // Pixel size
    const unsigned int dX = r.xMax - r.xMin,
                       dY = r.yMax - r.yMin;

    // Set border trace struct up
    BorderTrace bt;
    bt.pixels = pixels; bt.pixelSize = ps; bt.data = (void*)&s;
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
}


void Mandelbrot::calcScreenBruteforce(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    const ShapeVector shapes = (data == nullptr ? ShapeVector() : *(ShapeVector*)data);
    const double pixelSize = (domain.rMax - domain.rMin) / (double)res.w;

    double c[2];
    for(unsigned int y = r.yMin; y < r.yMax; y++) {
        c[1] = domain.iMax - (y * pixelSize);
        
        for(unsigned int x = r.xMin; x < r.xMax; x++) {
            c[0] = domain.rMin + (x * pixelSize);

            pixels[y * res.w + x] = calcPixel(c, (void*)&shapes);
        }
    }
}


inline uint32_t Mandelbrot::calcPixelNoShape(const double c[2]) const {
    double z[2] = {0, 0};
    double zSquared[2] = {0, 0};  // caches squares of real and imaginary part

    // Escape iteration loop
    iter_t n = 0;                                               // Without caching
    for(; n < nMax && zSquared[0] + zSquared[1] <= 4.0; n++) {  // for(; n < nMax && (z[0] * z[0]) + (z[1] * z[1]) <= 4.0; n++) {
        z[1] = z[0] * z[1] * 2.0;                               //     double temp = z[0] * z[1] * 2.0;
        z[0] = zSquared[0] - zSquared[1];                       //     z[0] = (z[0] * z[0]) - (z[1] * z[1]);
                                                                //     z[1] = temp;
        z[0] += c[0];                                           //
        z[1] += c[1];                                           //     z[0] += c[0];
                                                                //     z[1] += c[1];
        zSquared[0] = z[0] * z[0];                              // }
        zSquared[1] = z[1] * z[1];
    }

    return calcColor(n);
}

void Mandelbrot::calcScreenBruteforceNoShape(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    const double pixelSize = (domain.rMax - domain.rMin) / (double)res.w;

    double c[2];
    for(unsigned int y = r.yMin; y < r.yMax; y++) {
        c[1] = domain.iMax - (y * pixelSize);
        
        for(unsigned int x = r.xMin; x < r.xMax; x++) {
            c[0] = domain.rMin + (x * pixelSize);

            pixels[y * res.w + x] = calcPixelNoShape(c);
        }
    }

    return;

    // Prevent error
    data = (double*)&data;
}


uint32_t Mandelbrot::calcPixelShapeWrong(const double c[2], void* data) const {
    const ShapeVector shapes = (data == nullptr ? ShapeVector() : *(ShapeVector*)data);

    double z[2] = {0, 0};
    double zSquared[2] = {0, 0};  // caches squares of real and imaginary part

    // Escape iteration loop
    iter_t n = 0;
    for(; n < nMax && zSquared[0] + zSquared[1] <= 4.0; n++) {

        z[1] = z[0] * z[1] * 2.0;
        z[0] = zSquared[0] - zSquared[1];

        z[0] += c[0];
        z[1] += c[1];

        zSquared[0] = z[0] * z[0];
        zSquared[1] = z[1] * z[1];

        // Check shapes
        for(auto& inShape : shapes)
            if(inShape(z))
                return 0x0;
    }

    return calcColor(n);
}

void Mandelbrot::calcScreenShapeWrong(const Domain& domain, const Resolution& res, const Range& r, void* data, uint32_t* pixels) const {
    const ShapeVector shapes = (data == nullptr ? ShapeVector() : *(ShapeVector*)data);
    const double pixelSize = (domain.rMax - domain.rMin) / (double)res.w;

    double c[2];
    for(unsigned int y = r.yMin; y < r.yMax; y++) {
        c[1] = domain.iMax - (y * pixelSize);
        
        for(unsigned int x = r.xMin; x < r.xMax; x++) {
            c[0] = domain.rMin + (x * pixelSize);

            pixels[y * res.w + x] = calcPixelShapeWrong(c, (void*)&shapes);
        }
    }
}


void Mandelbrot::calcOrbit(const double c[2], Orbit& points) const {
    points.push_back({0, 0});
    double z[2] = {0, 0};
    double zSquared[2] = {0, 0};  // caches squares of real and imaginary part

    // Escape iteration loop
    iter_t n = 0;
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
