#include <gtest/gtest.h>

#include <cmath>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "rtff/abstract_filter.h"
#include "wave/file.h"


const std::string gResourcePath(TEST_RESOURCES_PATH);

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

class FreezerFilter : public rtff::AbstractFilter {
 public:
  FreezerFilter() : rtff::AbstractFilter(), is_on_(false), just_on_(false) {}
  void set_is_on(bool value) {
    is_on_ = value;
    just_on_ = true;
  }
  
  bool is_on() const { return is_on_; }
  
  void ProcessBlock(rtff::AudioBuffer* buffer) override {
    // copy data to temp buffer
    for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
      std::copy(buffer->data(channel_idx),
                buffer->data(channel_idx) + buffer->frame_count(),
                processing_buffer_->data(channel_idx));
    }
    
    // process temp buffer
    rtff::AbstractFilter::ProcessBlock(processing_buffer_.get());
    
    // add the buffer output to original buffer
    for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
      Eigen::Map<Eigen::VectorXf> processed_data(
          processing_buffer_->data(channel_idx),
          processing_buffer_->frame_count());
      Eigen::Map<Eigen::VectorXf> original_data(
          buffer->data(channel_idx),
          buffer->frame_count());
      original_data += processed_data;
    }
  }
  
  uint32_t FrameLatency() const override {
    return 0;
  }

 private:
  void PrepareToPlay() override {
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

  void ProcessTransformedBlock(std::vector<std::complex<float>*> data,
                               uint32_t size) override {
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

  std::shared_ptr<rtff::AudioBuffer> processing_buffer_;
  Eigen::VectorXcf fourier_transform_, previous_fourier_transform_;
  bool is_on_, just_on_;
  Eigen::VectorXf dphi_, total_dphi_, freeze_ft_magnitude_;
};

TEST(Freezer, basic) {
  // Read input file content
  wave::File file;
  file.Open(gResourcePath + "/test_sample.wav", wave::OpenMode::kIn);
  std::vector<float> content(file.frame_number() * file.channel_number());
  ASSERT_EQ(file.Read(&content), wave::Error::kNoError);

  // Initialize filter
  auto block_size = 2048;
  auto channel_number = file.channel_number();

  FreezerFilter filter;
  std::error_code err;
  filter.Init(channel_number, 2048, 1024, err);
  ASSERT_FALSE(err);
  filter.set_block_size(block_size);

  rtff::AudioBuffer buffer(block_size, channel_number);

  // For debug. From this point, the application shouldn't allocate any memory.
  Eigen::internal::set_is_malloc_allowed(false);

  // Extract each frames (add latency)
  auto multichannel_buffer_size = block_size * channel_number;

  for (uint32_t sample_idx = 0;
       sample_idx < content.size() - multichannel_buffer_size;
       sample_idx += multichannel_buffer_size) {
    // process the input buffer
    float* sample_ptr = content.data() + sample_idx;

    buffer.fromInterleaved(sample_ptr);
    filter.ProcessBlock(&buffer);
    buffer.toInterleaved(sample_ptr);

    // enable after 1 sec
    if (!filter.is_on() && sample_idx >= 1 * file.sample_rate() * file.channel_number()) {
      filter.set_is_on(true);
    }
    // disable after 4 sec
    if (filter.is_on() && sample_idx >= 4 * file.sample_rate() * file.channel_number()) {
      filter.set_is_on(false);
    }

    memcpy(content.data() + sample_idx, sample_ptr,
           block_size * channel_number * sizeof(float));
    
    // display the current status
    std::cout << round(double(sample_idx * 100) /
                       (file.frame_number() * file.channel_number()))
    << "%" << std::endl;
  }

  // For debug. From this point, the application can allocate memory
  Eigen::internal::set_is_malloc_allowed(true);

  // Write the output file content
  wave::File output;
  output.Open("/tmp/rtff_res.wav", wave::OpenMode::kOut);
  output.set_sample_rate(file.sample_rate());
  output.set_channel_number(file.channel_number());
  output.set_bits_per_sample(file.bits_per_sample());
  output.Write(content);
}
