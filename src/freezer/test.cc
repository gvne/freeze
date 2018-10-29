#include <gtest/gtest.h>

#include <Eigen/Core>

#include "rtff/abstract_filter.h"
#include "wave/file.h"

const std::string gResourcePath(TEST_RESOURCES_PATH);

class FreezerFilter : public rtff::AbstractFilter {
 private:
  void PrepareToPlay(std::error_code& err) override {
    // TODO: in members and ctor parameters
    const auto mem_size = 7;
    rtff::AbstractFilter::PrepareToPlay(err);
    auto trame_size = window_size() / 2 + 1;
    auto multichannel_trame_size = trame_size * channel_count();
    sliding_window_ = Eigen::MatrixXcf::Zero(multichannel_trame_size, mem_size);
  }
  
  void ProcessTransformedBlock(std::vector<std::complex<float>*> data,
                               uint32_t size) override {
    // store the stft trame in a sliding window
    // -- rotate the array by one
    for (auto idx = sliding_window_.cols() - 1; idx > 0; idx--) {
      sliding_window_.col(idx) = sliding_window_.col(idx - 1);
    }
    // -- make trame from data
    for (auto channel_idx = 0; channel_idx < channel_count(); channel_idx++) {
      auto channel_block = sliding_window_.block(channel_idx * size, 0, size, 1);
      channel_block = Eigen::Map<Eigen::MatrixXcf>(data[channel_idx], size, 1);
    }
  }
  
  Eigen::MatrixXcf sliding_window_;
};

TEST(Freezer, basic) {
  // Read input file content
  wave::File file;
  file.Open(gResourcePath + "/Untitled3.wav", wave::OpenMode::kIn);
  std::vector<float> content(file.frame_number() * file.channel_number());
  ASSERT_EQ(file.Read(&content), wave::Error::kNoError);
  
  // Initialize filter
  auto block_size = 2048;
  auto channel_number = file.channel_number();
  
  FreezerFilter filter;
  std::error_code err;
  filter.Init(channel_number, 512, 256, err);
  ASSERT_FALSE(err);
  filter.set_block_size(block_size);
  
  rtff::AudioBuffer buffer(block_size, channel_number);
  
  // For debug. From this point, the application shouldn't allocate any memory.
//  Eigen::internal::set_is_malloc_allowed(false);
  
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
    
    // to write, we compensate the latency
    int output_sample_idx =
    sample_idx - (filter.FrameLatency() * channel_number);
    if (output_sample_idx < 0) {
      // begining of the file. As we create latency, the first few samples will
      // be zeros. To compensate, we just remove them
      float* output_sample_ptr = content.data();
      float* processed_sample_ptr = sample_ptr + abs(output_sample_idx);
      auto size_to_copy = block_size - filter.FrameLatency();
      memcpy(output_sample_ptr, processed_sample_ptr,
             size_to_copy * channel_number * sizeof(float));
    } else {
      // after the first few buffers, we are on general case. We just have the
      // write taking the latency into consideration
      float* output_sample_ptr = content.data() + output_sample_idx;
      memcpy(output_sample_ptr, sample_ptr,
             block_size * channel_number * sizeof(float));
    }
    
    // display the current status
    std::cout << round(double(sample_idx * 100) /
                       (file.frame_number() * file.channel_number()))
    << "%" << std::endl;
  }
  
  // For debug. From this point, the application can allocate memory
//  Eigen::internal::set_is_malloc_allowed(true);
  
  // Write the output file content
  wave::File output;
  output.Open("/tmp/rtff_res.wav", wave::OpenMode::kOut);
  output.set_sample_rate(file.sample_rate());
  output.set_channel_number(file.channel_number());
  output.set_bits_per_sample(file.bits_per_sample());
  output.Write(content);
}
