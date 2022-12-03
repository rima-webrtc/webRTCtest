#ifndef __AUDIO_PREPROCESS_H__
#define __AUDIO_PREPROCESS_H__

#include <memory>

// TODO: just support aac format<AAC-LC 48kHz 16LE 2channels 1024period>
struct ProcessOption {
  int sampleRate_in = 48000;  
  int sampleBits_in = 16;  
  int channels_in = 2; 
  int sampleRate_out = 32000;  
  int sampleBits_out = 16;  
  int channels_out = 2;
};

class IPreProcesss {
 public:
  virtual ProcessOption const& option() const = 0;
  // input   -- for raw pcm data
  // in_len  -- for length of raw pcm data.  len = 1024*2*2  return max to maxBufferSize.
  // output  -- for processed pcm data.
  // out_len -- for length of processed pcm data. equal to in_len default.
  virtual size_t process(void const* input, size_t in_len, void* output, size_t out_len) = 0;
  virtual ~IPreProcesss(){};
};

// default for AAC-LC
// AGC
std::unique_ptr<IPreProcesss> createAGCProcess(ProcessOption const& option);
// ANS
std::unique_ptr<IPreProcesss> createANSProcess(ProcessOption const& option);
// ANSX
std::unique_ptr<IPreProcesss> createANSProcess(ProcessOption const& option);
// reSampler
std::unique_ptr<IPreProcesss> createReSamplerProcess(ProcessOption const& option);

#endif