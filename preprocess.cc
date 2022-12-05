// Copyright (c) 2022 caobing@cowarobot
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "preprocess.h"
#include "resampler.h"
#include <sstream>
#include <string.h>
#include <vector>
#include <iostream>
#include "./WebRtcMoudle/echo_cancellation.h"
#include "./WebRtcMoudle/gain_control.h"
#include "./WebRtcMoudle/noise_suppression.h"
#include "./WebRtcMoudle/noise_suppression_x.h"
#include "./WebRtcMoudle/signal_processing_library.h"
// 4 types, the second is the best
// enum nsLevel
// {
//     kLow,
//     kModerate,
//     kHigh,
//     kVeryHigh
// };

#ifndef MIN
#  define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

class ANSProcess : public IPreProcesss {
  ProcessOption _option;
  NsxHandle* pNS_inst = NULL;
  int filter_state1[6], filter_state12[6];
  int Synthesis_state1[6], Synthesis_state12[6];

 public:
  ANSProcess(ProcessOption const& option) : _option(option) {
    if (0 != WebRtcNsx_Create(&pNS_inst)) { printf("Noise_Suppression WebRtcNs_Create err! \n"); }

    if (0 != WebRtcNsx_Init(pNS_inst, _option.sampleRate_in)) { printf("Noise_Suppression WebRtcNs_Init err! \n"); }

    if (0 != WebRtcNsx_set_policy(pNS_inst, 2)) { printf("Noise_Suppression WebRtcNs_set_policy err! \n"); }

    memset(filter_state1, 0, sizeof(filter_state1));
    memset(filter_state12, 0, sizeof(filter_state12));
    memset(Synthesis_state1, 0, sizeof(Synthesis_state1));
    memset(Synthesis_state12, 0, sizeof(Synthesis_state12));
  }
  virtual ~ANSProcess() override { WebRtcNsx_Free(pNS_inst); }
  virtual ProcessOption const& option() const override { return _option; };

  /// @brief
  /// @param input every input frame  pointer
  /// @param in_len every input frame length
  /// @param output every output frame pointer
  /// @param out_len every output frame length
  /// @return
  size_t process(void const* input, size_t in_len, void* output, size_t out_len) override {
       std::cout <<"processstart\n";
    // 32hz will be cut to 2 16000 to handle.
    if (_option.sampleRate_in == 32000) {
    int16_t* ansIn = (int16_t*)input;
    int16_t* ansOut = (int16_t*)output;
      short shBufferIn[320] = {0};
      short shInL[160] = {0}, shInH[160] = {0};
      short shOutL[160] = {0}, shOutH[160] = {0};
      short shBufferOut[320] = {0};
      memcpy(shBufferIn, ansIn, in_len);

      WebRtcSpl_AnalysisQMF(shBufferIn, 320, shInL, shInH, filter_state1, filter_state12);

      if (0 == WebRtcNsx_Process(pNS_inst, shInL, shInH, shOutL, shOutH)) {
        WebRtcSpl_SynthesisQMF(shOutL, shOutH, 160, shBufferOut, Synthesis_state1, Synthesis_state12);
      }
      memcpy(ansOut, shBufferOut, out_len);
std::cout <<"11111\n";
    } else {
      std::cout << "need to be finished. ns,nsx only support 8hz,16hz,32hz.every time do with 10ms.it is one frame "
                << std::endl;
    }
    return 0;
  }
  };

// class AGCProcess : public IPreProcesss {
//   ProcessOption _option;

//  public:
//   AGCProcess(ProcessOption const& option) {}
//   virtual ~AGCProcess() override {}
//   virtual ProcessOption const& option() const override { return _option; };
//   size_t process(void const* input, size_t in_len, void* output, size_t out_len) override {
//     // size_t samples = MIN(160, _option.sampleRate / 100);
//     // int16_t *raw = (int16_t*)input; //raw pcm

//     return 0;
//   }
// };

class ReSamplerProcess : public IPreProcesss {
  ProcessOption _option;
  Resampler _rs;
  // int HZ = 48000;
  // const int CHANNELS = 2;

 public:
  ReSamplerProcess(ProcessOption const& option) : _option(option) { _rs.Reset(_option.sampleRate_in, _option.sampleRate_out, _option.channels_in ); }
  virtual ~ReSamplerProcess() override {}
  virtual ProcessOption const& option() const override { return _option; };
  size_t process(void const* input, size_t in_len, void* output, size_t out_len) override {
    // samplerate = 24KHz channels = 1  for IN
    if (input == 0 || output == 0) { return 0; }

    size_t batch_in =_option.sampleRate_in / 100;  // 240
    size_t batch_out = _option.sampleRate_out / 100;  // 480
    size_t samples_in = in_len / 2;
    size_t count_in = (samples_in / batch_in);
    size_t remained = samples_in - (batch_in * count_in);
    size_t expect_len = (size_t)(samples_in * 32 / 48);  // double samplerate and double channels.
    int16_t* samplesIn = (int16_t*)input;
    int16_t* samplesOut = (int16_t*)output;

    // double samplerate.
    size_t len = 0;
    for (int i = 0; i < count_in; i++) {
      _rs.Push(samplesIn, batch_in, samplesOut, batch_out, len);
      samplesIn += batch_in;
      samplesOut += len;
    }
    //if (remained != 0) {
    //   const int max_samples = 1920;
    //   int16_t samplePatchIn[max_samples] = {0};
    //   int16_t samplePatchOut[max_samples] = {0};
    //   memcpy(samplePatchIn, samplesIn, remained * sizeof(int16_t));
    //   _rs.Push(samplePatchIn, batch_in, samplePatchOut, batch_out, len);
    //   memcpy(samplesOut, samplePatchOut, (remained * 2) * sizeof(int16_t));

    // }

    return expect_len;
  }
};

// std::unique_ptr<IPreProcesss> createAGCProcess(ProcessOption const& option) {
//   return std::make_unique<AGCProcess>(option);
// }
std::unique_ptr<IPreProcesss> createANSProcess(ProcessOption const& option) {
  return std::make_unique<ANSProcess>(option);
}

std::unique_ptr<IPreProcesss> createReSamplerProcess(ProcessOption const& option) {
  return std::make_unique<ReSamplerProcess>(option);
}
