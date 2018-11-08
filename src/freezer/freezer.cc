#include "freezer/freezer.h"

#include <Eigen/Core>

#include "freezer/operation.h"

class Freezer::Impl {
 public:
  Eigen::VectorXcf fourier_transform, previous_fourier_transform;
  Eigen::VectorXf dphi, total_dphi, freeze_ft_magnitude;
};

Freezer::Freezer()
    : rtff::AbstractFilter(),
      processing_buffer_(nullptr),
      is_on_(false),
      just_on_(false),
      gain_(1),
      dry_gain_(1),
      fade_in_duration_(0),
      fade_out_duration_(0),
      impl_(std::make_shared<Impl>()) {}

void Freezer::set_is_on(bool value) {
  is_on_ = value;
  just_on_ = true;
}

bool Freezer::is_on() const { return is_on_; }

void Freezer::ProcessBlock(rtff::AudioBuffer* buffer) {
  // copy data to temp buffer
  for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
    std::copy(buffer->data(channel_idx),
              buffer->data(channel_idx) + buffer->frame_count(),
              processing_buffer_->data(channel_idx));
  }

  // process temp buffer
  rtff::AbstractFilter::ProcessBlock(processing_buffer_.get());
  ApplyGain(gain_, processing_buffer_.get());

  // add the temp buffer output to original buffer
  for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
    Eigen::Map<Eigen::VectorXf> processed_data(
        processing_buffer_->data(channel_idx),
        processing_buffer_->frame_count());
    Eigen::Map<Eigen::VectorXf> original_data(buffer->data(channel_idx),
                                              buffer->frame_count());
    original_data += processed_data;
  }

  ApplyGain(dry_gain_, buffer);
}

void Freezer::ApplyGain(float gain, rtff::AudioBuffer* buffer) {
  for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
    Eigen::Map<Eigen::VectorXf> data(buffer->data(channel_idx),
                                     buffer->frame_count());
    data.array() *= gain;
  }
}

uint32_t Freezer::FrameLatency() const { return 0; }

void Freezer::PrepareToPlay() {
  rtff::AbstractFilter::PrepareToPlay();
  // Initialize processing buffer
  processing_buffer_ =
      std::make_shared<rtff::AudioBuffer>(block_size(), channel_count());

  // Initialize storage to avoid allocating memory at runtime
  auto trame_size = window_size() / 2 + 1;
  auto multichannel_trame_size = trame_size * channel_count();

  // members
  impl_->fourier_transform = Eigen::VectorXcf::Zero(multichannel_trame_size);
  impl_->previous_fourier_transform.noalias() = impl_->fourier_transform;
  impl_->dphi = Eigen::VectorXf::Zero(multichannel_trame_size);
  impl_->total_dphi = Eigen::VectorXf::Zero(multichannel_trame_size);
  impl_->freeze_ft_magnitude = Eigen::VectorXf::Zero(multichannel_trame_size);
}

void Freezer::ProcessTransformedBlock(std::vector<std::complex<float>*> data,
                                      uint32_t size) {
  // Fill data into the fourier transform and set it to zeros
  for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
    Eigen::Map<Eigen::VectorXcf> channel_data(data[channel_idx], size);
    impl_->fourier_transform.segment(channel_idx * size, size) = channel_data;
    channel_data.noalias() = Eigen::VectorXcf::Zero(size);
  }

  if (just_on_) {
    impl_->freeze_ft_magnitude.noalias() = impl_->fourier_transform.cwiseAbs();
    impl_->total_dphi.noalias() =
        impl_->fourier_transform.unaryExpr<ArgOperation>();
    auto previous_angle =
        impl_->previous_fourier_transform.unaryExpr<ArgOperation>();
    impl_->dphi.noalias() = impl_->total_dphi - previous_angle;
    just_on_ = false;
  }

  if (is_on()) {
    impl_->total_dphi += impl_->dphi;
    impl_->total_dphi.noalias() =
        impl_->total_dphi.unaryExpr<Modulo2PIOperation>();

    // in python: self.freeze_ft_magnitude * np.exp(1j * self.total_dphi)
    // 1j * self.total_dphi => convert total_dphi imaginary part of a complex
    auto expr =
        impl_->freeze_ft_magnitude.array() *
        impl_->total_dphi.unaryExpr<ToComplexImgOperation>().array().exp();

    // set it to the input data
    for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
      Eigen::Map<Eigen::VectorXcf> channel_data(data[channel_idx], size);
      channel_data.array() = expr.segment(channel_idx * size, size);
    }
  }

  impl_->previous_fourier_transform.noalias() = impl_->fourier_transform;
}

void Freezer::set_gain(float gain) { gain_ = std::pow(10, gain / 20.0); }
float Freezer::gain() const { return 20.0 * std::log10(gain_); }
void Freezer::set_dry_gain(float gain) {
  dry_gain_ = std::pow(10, gain / 20.0);
}
float Freezer::dry_gain() const { return 20.0 * std::log10(dry_gain_); }
void Freezer::set_fade_in_duration(uint32_t sample_count) {
  fade_in_duration_ = sample_count;
}
uint32_t Freezer::fade_in_duration() const { return fade_in_duration_; }
void Freezer::set_fade_out_duration(uint32_t sample_count) {
  fade_out_duration_ = sample_count;
}
uint32_t Freezer::fade_out_duration() const { return fade_out_duration_; }
