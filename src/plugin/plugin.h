#ifndef FREEZE_SRC_PLUGIN_PLUGIN_H_
#define FREEZE_SRC_PLUGIN_PLUGIN_H_

#include <cstdint>
#include <memory>

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>


// Plugin name
#define PLUGIN_URI "http://romain-hennequin.fr/plugins/mod-devel/Freeze"

// Plugin parameters
enum {
  IN,
  OUT,
  FREEZE,
  FREEZEGAIN,
  DRYGAIN,
  FADEINDURATION,
  FADEOUTDURATION,
  PLUGIN_PORT_COUNT
};

// class Freezer;
namespace rtff {
class AudioBuffer;
class Filter;
}  // namespace rtff

class Plugin {
 public:
  // LV2 Plugin interface
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

  // plugin specific code
  Plugin();
 private:
  uint8_t channel_count() const;

  void UpdateParameters();
  void UpdateBuffers(uint32_t block_size);
  void ProcessBlock(uint32_t block_size);
  // std::shared_ptr<rtff::Filter> filter_;
  // std::shared_ptr<Freezer> filter_;
  std::shared_ptr<rtff::AudioBuffer> buffer_;
};

#endif  // FREEZE_SRC_PLUGIN_PLUGIN_H_
