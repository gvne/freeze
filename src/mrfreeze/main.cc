#include <cstdlib>
#include <iostream>

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>

#include "freezer/freezer.h"
#include "rtff/buffer/audio_buffer.h"

/**********************************************************************************************************************************************************/

#define PLUGIN_URI "http://romain-hennequin.fr/plugins/mod-devel/Freeze"
enum { IN, OUT, FREEZE, FREEZEGAIN, DRYGAIN, FADEINDURATION, FADEOUTDURATION, PLUGIN_PORT_COUNT };

/**********************************************************************************************************************************************************/

class Plugin {
 public:
  Plugin() : filter(std::make_shared<Freezer>()) {
    std::error_code err;
    filter->Init(channel_count(), err);
    // TODO: check for error...
  }
  std::shared_ptr<Freezer> filter;
  std::shared_ptr<rtff::AudioBuffer> buffer;
  uint8_t channel_count() const { return 1; }  // only runs in mono

  static LV2_Handle instantiate(const LV2_Descriptor* descriptor,
                                double samplerate, const char* bundle_path,
                                const LV2_Feature* const* features);
  static void activate(LV2_Handle instance);
  static void deactivate(LV2_Handle instance);
  static void connect_port(LV2_Handle instance, uint32_t port, void* data);
  static void run(LV2_Handle instance, uint32_t n_samples);
  static void cleanup(LV2_Handle instance);
  static const void* extension_data(const char* uri);
  float* ports[PLUGIN_PORT_COUNT];

 private:
  void UpdateParameters();
  void UpdateBuffers(uint32_t block_size);
  void ProcessBlock(uint32_t block_size);
};

/**********************************************************************************************************************************************************/

static const LV2_Descriptor descriptor = {
    PLUGIN_URI,       Plugin::instantiate,   Plugin::connect_port,
    Plugin::activate, Plugin::run,           Plugin::deactivate,
    Plugin::cleanup,  Plugin::extension_data};

/**********************************************************************************************************************************************************/

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index) {
  if (index == 0) {
    return &descriptor;
  }
  return nullptr;
}

/**********************************************************************************************************************************************************/

LV2_Handle Plugin::instantiate(const LV2_Descriptor* descriptor,
                               double samplerate, const char* bundle_path,
                               const LV2_Feature* const* features) {
  return (LV2_Handle)new Plugin();
}

/**********************************************************************************************************************************************************/

void Plugin::activate(LV2_Handle instance) {}

/**********************************************************************************************************************************************************/

void Plugin::deactivate(LV2_Handle instance) {}

/**********************************************************************************************************************************************************/

void Plugin::connect_port(LV2_Handle instance, uint32_t port, void* data) {
  auto plugin = reinterpret_cast<Plugin*>(instance);
  plugin->ports[port] = reinterpret_cast<float*>(data);
}

/**********************************************************************************************************************************************************/

void Plugin::UpdateBuffers(uint32_t block_size) {
  // TODO: initialize the block size at instanciation to avoid allocating
  // memory at runtime
  if (filter->block_size() != n_samples) {
    filter->set_block_size(n_samples);
  }
  if (!buffer || buffer->frame_count() != n_samples) {
    buffer = std::make_shared<rtff::AudioBuffer>(block_size, channel_count());
  }
}

void Plugin::UpdateParameters() {
  // set the ON / OFF state of the filter
  bool enabled  = static_cast<int>(*(ports[FREEZE])+0.5f) == 1;
  if (filter->is_on() != enabled) {
    filter->set_is_on(enabled);
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
  auto buffer_ptr = buffer->data(0);

  // copy input in buffer
  auto in = ports[IN];
  std::copy(in, in + n_samples, buffer_ptr);

  // process buffer
  filter->ProcessBlock(buffer.get());

  // copy buffer to output
  auto out = ports[OUT];
  std::copy(buffer_ptr, buffer_ptr + n_samples, out);
}

void Plugin::run(LV2_Handle instance, uint32_t n_samples) {
  auto plugin = reinterpret_cast<Plugin*>(instance);

  plugin->UpdateBuffers(n_samples);
  plugin->UpdateParameters();
  plugin->ProcesBlock(n_samples);
}

/**********************************************************************************************************************************************************/

void Plugin::cleanup(LV2_Handle instance) { delete ((Plugin*)instance); }

/**********************************************************************************************************************************************************/

const void* Plugin::extension_data(const char* uri) { return NULL; }
