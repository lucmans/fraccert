
#ifndef SHAPES_H
#define SHAPES_H


#include <vector>


typedef std::vector<bool (*)(const double[2])> ShapeVector;


// Shapes
bool inCardioid(const double z[2]);
bool in2Bulb(const double z[2]);


#endif  // SHAPES_H
