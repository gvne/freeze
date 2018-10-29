#include <gtest/gtest.h>

#include "freezer/freezer.h"
#include "wave/file.h"


const std::string gResourcePath(TEST_RESOURCES_PATH);


TEST(Freezer, basic) {
  // Read input file content
  wave::File file;
  file.Open(gResourcePath + "/test_sample.wav", wave::OpenMode::kIn);
  std::vector<float> content(file.frame_number() * file.channel_number());
  ASSERT_EQ(file.Read(&content), wave::Error::kNoError);

  // Initialize filter
  auto block_size = 2048;
  auto channel_number = file.channel_number();

  Freezer filter;
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
