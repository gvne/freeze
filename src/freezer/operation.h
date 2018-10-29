#ifndef FREEZER_OPERATION_H_
#define FREEZER_OPERATION_H_

#include <complex>

struct ArgOperation {
  float operator()(const std::complex<float>& value) const;
};
struct Modulo2PIOperation {
  float operator()(const float& value) const;
};
struct ToComplexImgOperation {
  std::complex<float> operator()(const float& value) const;
};

#endif  // FREEZER_OPERATION_H_
