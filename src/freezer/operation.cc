#include "freezer/operation.h"

#include <cmath>

float ArgOperation::operator()(const std::complex<float>& value) const {
  return std::arg(value);
}
float Modulo2PIOperation::operator()(const float& value) const {
  return std::fmod(value, 2 * M_PI);
}
std::complex<float> ToComplexImgOperation::operator()(const float& value) const {
  return std::complex<float>(0, value);
}
