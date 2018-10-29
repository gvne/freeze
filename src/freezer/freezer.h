#ifndef FREEZER_FREEZER_H_
#define FREEZER_FREEZER_H_

#include "rtff/abstract_filter.h"

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
  bool is_on_, just_on_;
  
  class Impl;
  std::shared_ptr<Impl> impl_;
};

#endif  // FREEZER_FREEZER_H_
