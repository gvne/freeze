#include "freezer/freezer.h"

#include "freezer/operation.h"

Freezer::Freezer()
    : rtff::AbstractFilter(),
      processing_buffer_(nullptr),
      is_on_(false),
      just_on_(false) {}

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

  // add the temp buffer output to original buffer
  for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
    Eigen::Map<Eigen::VectorXf> processed_data(
        processing_buffer_->data(channel_idx),
        processing_buffer_->frame_count());
    Eigen::Map<Eigen::VectorXf> original_data(buffer->data(channel_idx),
                                              buffer->frame_count());
    original_data += processed_data;
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
  fourier_transform_ = Eigen::VectorXcf::Zero(multichannel_trame_size);
  previous_fourier_transform_.noalias() = fourier_transform_;
  dphi_ = Eigen::VectorXf::Zero(multichannel_trame_size);
  total_dphi_ = Eigen::VectorXf::Zero(multichannel_trame_size);
  freeze_ft_magnitude_ = Eigen::VectorXf::Zero(multichannel_trame_size);
}

void Freezer::ProcessTransformedBlock(std::vector<std::complex<float>*> data,
                                      uint32_t size) {
  // Fill data into the fourier transform and set it to zeros
  for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
    Eigen::Map<Eigen::VectorXcf> channel_data(data[channel_idx], size);
    fourier_transform_.segment(channel_idx * size, size) = channel_data;
    channel_data.noalias() = Eigen::VectorXcf::Zero(size);
  }

  if (just_on_) {
    freeze_ft_magnitude_.noalias() = fourier_transform_.cwiseAbs();
    total_dphi_.noalias() = fourier_transform_.unaryExpr<ArgOperation>();
    auto previous_angle = previous_fourier_transform_.unaryExpr<ArgOperation>();
    dphi_.noalias() = total_dphi_ - previous_angle;
    just_on_ = false;
  }

  if (is_on()) {
    total_dphi_ += dphi_;
    total_dphi_.noalias() = total_dphi_.unaryExpr<Modulo2PIOperation>();

    // in python: self.freeze_ft_magnitude * np.exp(1j * self.total_dphi)
    // 1j * self.total_dphi => convert total_dphi imaginary part of a complex
    auto expr = freeze_ft_magnitude_.array() *
                total_dphi_.unaryExpr<ToComplexImgOperation>().array().exp();

    // set it to the input data
    for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
      Eigen::Map<Eigen::VectorXcf> channel_data(data[channel_idx], size);
      channel_data.array() = expr.segment(channel_idx * size, size);
    }
  }

  previous_fourier_transform_.noalias() = fourier_transform_;
}
