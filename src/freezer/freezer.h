#ifndef FREEZER_FREEZER_H_
#define FREEZER_FREEZER_H_

#include <cmath>

#include <Eigen/Core>
#include "rtff/abstract_filter.h"

// Allow building an unary expression from a vector / matrix
//   auto expression = my_object.unaryExpr<ArgOperation>();
struct ArgOperation {
  float operator()(const std::complex<float>& value) const {
    return std::arg(value);
  }
};
struct Modulo2PIOperation {
  float operator()(const float& value) const {
    return std::fmod(value, 2 * M_PI);
  }
};
struct ToComplexImgOperation {
  std::complex<float> operator()(const float& value) const {
    return std::complex<float>(0, value);
  }
};

class Freezer : public rtff::AbstractFilter {
 public:
  Freezer();
  void set_is_on(bool value);
  bool is_on() const;
  void ProcessBlock(rtff::AudioBuffer* buffer) override;
  uint32_t FrameLatency() const override;

 private:
  void PrepareToPlay() override;
  void ProcessTransformedBlock(std::vector<std::complex<float>*> data,
                               uint32_t size) override;

  std::shared_ptr<rtff::AudioBuffer> processing_buffer_;
  Eigen::VectorXcf fourier_transform_, previous_fourier_transform_;
  bool is_on_, just_on_;
  Eigen::VectorXf dphi_, total_dphi_, freeze_ft_magnitude_;
};

#endif  // FREEZER_FREEZER_H_
