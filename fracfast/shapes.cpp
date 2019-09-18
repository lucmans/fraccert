
#include "shapes.h"


bool inCardioid(const double z[2]) {
    const double q = ((z[0] - 0.25) * (z[0] - 0.25)) + (z[1] * z[1]);
    return q * (q + (z[0] - 0.25)) < z[1] * z[1] * 0.25;
}


bool in2Bulb(const double z[2]) {
    return (z[0] * z[0]) + (2 * z[0]) + 1 + (z[1] * z[1]) < 0.0625;
}


// bool inCardioid(const double z[2], const double zSquared[2]) {
//     const double q = ((z[0] - 0.25) * (z[0] - 0.25)) + zSquared[1];
//     return q * (q + (z[0] - 0.25)) <= zSquared[1] * 0.25;
// }


// bool in2Bulb(const double z[2], const double zSquared[2]) {
//     return zSquared[0] + (2 * z[0]) + 1 + zSquared[1] <= 0.0625;
// }


// if ((Complex.Abs(1.0 - Complex.Sqrt(Complex.One - (4 * item))) < 1.0)) continue;
// if (((Complex.Abs(item - new Complex(-1, 0))) < 0.25)) continue;
// if ((((item.Real + 1.309) * (item.Real + 1.309)) + item.Imaginary * item.Imaginary) < 0.00345) continue;
// if ((((item.Real + 0.125) * (item.Real + 0.125)) + (item.Imaginary - 0.744) * (item.Imaginary - 0.744)) < 0.0088) continue;
// if ((((item.Real + 0.125) * (item.Real + 0.125)) + (item.Imaginary + 0.744) * (item.Imaginary + 0.744)) < 0.0088) continue;
