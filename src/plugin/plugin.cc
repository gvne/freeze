#include "plugin/plugin.h"

#include <cstdlib>
#include <iostream>


#include "freezer/freezer.h"
#include "rtff/buffer/audio_buffer.h"

// LV2 Plugin Interface
static const LV2_Descriptor descriptor = {
  PLUGIN_URI,
  Plugin::instantiate,
  Plugin::connect_port,
  Plugin::activate,
  Plugin::run,
  Plugin::deactivate,
  Plugin::cleanup,
  Plugin::extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index) {
  if (index == 0) {
    return &descriptor;
  }
  return nullptr;
}

LV2_Handle Plugin::instantiate(const LV2_Descriptor* descriptor,
                               double samplerate, const char* bundle_path,
                               const LV2_Feature* const* features) {
  return static_cast<LV2_Handle>(new Plugin());
}

// ------ Plugin Definition
// LV2 Interface
void Plugin::activate(LV2_Handle instance) {}
void Plugin::deactivate(LV2_Handle instance) {}
void Plugin::connect_port(LV2_Handle instance, uint32_t port, void* data) {
  auto plugin = reinterpret_cast<Plugin*>(instance);
  plugin->ports[port] = reinterpret_cast<float*>(data);
}
void Plugin::run(LV2_Handle instance, uint32_t n_samples) {
  auto plugin = reinterpret_cast<Plugin*>(instance);
  plugin->UpdateBuffers(n_samples);
  plugin->UpdateParameters();
  plugin->ProcessBlock(n_samples);
}
void Plugin::cleanup(LV2_Handle instance) {
  auto plugin = reinterpret_cast<Plugin*>(instance);
  delete plugin;
}
const void* Plugin::extension_data(const char* uri) { return nullptr; }

// Plugin specific code
Plugin::Plugin() : filter_(std::make_shared<Freezer>()) {
  std::error_code err;
  filter_->Init(channel_count(), err);
  // TODO: check for error...
}

void Plugin::UpdateBuffers(uint32_t block_size) {
  // TODO: initialize the block size at instanciation to avoid allocating
  // memory at runtime
  if (filter_->block_size() != block_size) {
    filter_->set_block_size(block_size);
  }
  if (!buffer_ || buffer_->frame_count() != block_size) {
    buffer_ = std::make_shared<rtff::AudioBuffer>(block_size, channel_count());
  }
}

void Plugin::UpdateParameters() {
  // set the ON / OFF state of the filter
  bool enabled  = static_cast<int>(*(ports[FREEZE])+0.5f) == 1;
  if (filter_->is_on() != enabled) {
    filter_->set_is_on(enabled);
  }

  // TODO: deal with gain
  // auto gain_db = static_cast<float>(*(plugin->ports[FREEZEGAIN]));
  // auto gain = std::pow(10,gain_db/20.0);
  //
  // auto dry_gain_db = static_cast<float>(*(plugin->ports[DRYGAIN]));

  // TODO:
  // float fade_in_duration = (float)(*(plugin->ports[FADEINDURATION]));
  // float fade_out_duration = (float)(*(plugin->ports[FADEOUTDURATION]));
}

void Plugin::ProcessBlock(uint32_t block_size) {
  // Run the process
  auto buffer_ptr = buffer_->data(0);

  // copy input in buffer
  auto in = ports[IN];
  std::copy(in, in + block_size, buffer_ptr);

  // process buffer
  filter_->ProcessBlock(buffer_.get());

  // copy buffer to output
  auto out = ports[OUT];
  std::copy(buffer_ptr, buffer_ptr + block_size, out);
}
