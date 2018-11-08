#ifndef FREEZER_FREEZER_H_
#define FREEZER_FREEZER_H_

#include "rtff/abstract_filter.h"

class Freezer : public rtff::AbstractFilter {
 public:
  Freezer();
  void ProcessBlock(rtff::AudioBuffer* buffer) override;
  uint32_t FrameLatency() const override;

  // parameters
  void set_is_on(bool value);
  bool is_on() const;
  void set_gain(float gain);
  float gain() const;
  void set_dry_gain(float gain);
  float dry_gain() const;
  void set_fade_in_duration(uint32_t sample_count);
  uint32_t fade_in_duration() const;
  void set_fade_out_duration(uint32_t sample_count);
  uint32_t fade_out_duration() const;

 private:
  void ApplyGain(float gain, rtff::AudioBuffer* buffer);
  void PrepareToPlay() override;
  void ProcessTransformedBlock(std::vector<std::complex<float>*> data,
                               uint32_t size) override;

  std::shared_ptr<rtff::AudioBuffer> processing_buffer_;
  bool is_on_, just_on_;
  float gain_;
  float dry_gain_;
  uint32_t fade_in_duration_;
  uint32_t fade_out_duration_;

  class Impl;
  std::shared_ptr<Impl> impl_;
};

#endif  // FREEZER_FREEZER_H_
