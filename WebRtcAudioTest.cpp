// WebRtcAudioTest.cpp : �������̨Ӧ�ó������ڵ㡣
//

// #include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <Windows.h>

#include <memory>

#include "./WebRtcMoudle/signal_processing_library.h"
#include "./WebRtcMoudle/noise_suppression_x.h"
#include "./WebRtcMoudle/noise_suppression.h"
#include "./WebRtcMoudle/gain_control.h"
#include "./WebRtcMoudle/echo_cancellation.h"
// All methods return 0 on success and -1 on failure.
class Resampler {
 public:
  Resampler();

  Resampler(size_t inFreq, size_t outFreq, size_t num_channels);

  ~Resampler();

  // Reset all states
  int Reset(size_t inFreq, size_t outFreq, size_t num_channels);

  // Reset all states if any parameter has changed
  int ResetIfNeeded(size_t inFreq, size_t outFreq, size_t num_channels);

  // Resample samplesIn to samplesOut.
  int Push(const int16_t* samplesIn,
           size_t lengthIn,
           int16_t* samplesOut,
           size_t maxLen,
           size_t& outLen);  // NOLINT: to avoid changing APIs

 private:
  enum ResamplerMode {
    kResamplerMode1To1,
    kResamplerMode1To2,
    kResamplerMode1To3,
    kResamplerMode1To4,
    kResamplerMode1To6,
    kResamplerMode1To12,
    kResamplerMode2To3,
    kResamplerMode2To11,
    kResamplerMode4To11,
    kResamplerMode8To11,
    kResamplerMode11To16,
    kResamplerMode11To32,
    kResamplerMode2To1,
    kResamplerMode3To1,
    kResamplerMode4To1,
    kResamplerMode6To1,
    kResamplerMode12To1,
    kResamplerMode3To2,
    kResamplerMode11To2,
    kResamplerMode11To4,
    kResamplerMode11To8
  };

  // Computes the resampler mode for a given sampling frequency pair.
  // Returns -1 for unsupported frequency pairs.
  static int ComputeResamplerMode(int in_freq_hz, int out_freq_hz, ResamplerMode* mode);

  // Generic pointers since we don't know what states we'll need
  void* state1_;
  void* state2_;
  void* state3_;

  // Storage if needed
  int16_t* in_buffer_;
  int16_t* out_buffer_;
  size_t in_buffer_size_;
  size_t out_buffer_size_;
  size_t in_buffer_size_max_;
  size_t out_buffer_size_max_;

  size_t my_in_frequency_khz_;
  size_t my_out_frequency_khz_;
  ResamplerMode my_mode_;
  size_t num_channels_;

  // Extra instance for stereo
  Resampler* slave_left_;
  Resampler* slave_right_;
};



struct ProcessOption {
  int sampleRate = 32000;  // OUT
  int sampleBits = 16;  // OUT
  int channels = 2;  // OUT
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
class ReSamplerProcess : public IPreProcesss {
  ProcessOption _option;
  Resampler _rs;
  const int HZ = 8000;
  const int CHANNELS = 2;

 public:
  ReSamplerProcess(ProcessOption const& option) : _option(option) { _rs.Reset(HZ, _option.sampleRate, CHANNELS); }
  virtual ~ReSamplerProcess() override {}
  virtual ProcessOption const& option() const override { return _option; };
  size_t process(void const* input, size_t in_len, void* output, size_t out_len) override {
    // samplerate = 24KHz channels = 1  for IN
    if (input == 0 || output == 0) { return 0; }

    size_t batch_in = HZ / 100;  // 240
    size_t batch_out = _option.sampleRate / 100;  // 480
    size_t samples_in = in_len / 2;
    size_t count_in = (samples_in / batch_in);
    size_t remained = samples_in - (batch_in * count_in);
    size_t expect_len = (size_t)(samples_in * 2);  // double samplerate and double channels.
    int16_t* samplesIn = (int16_t*)input;
    int16_t* samplesOut = (int16_t*)output;

    // double samplerate.
    size_t len = 0;
    for (int i = 0; i < count_in; i++) {
      _rs.Push(samplesIn, batch_in, samplesOut, batch_out, len);
      samplesIn += batch_in;
      samplesOut += len;
    }
    if (remained != 0) {
      const int max_samples = 1920;
      int16_t samplePatchIn[max_samples] = {0};
      int16_t samplePatchOut[max_samples] = {0};
      memcpy(samplePatchIn, samplesIn, remained * sizeof(int16_t));
      _rs.Push(samplePatchIn, batch_in, samplePatchOut, batch_out, len);
      memcpy(samplesOut, samplePatchOut, (remained * 2) * sizeof(int16_t));
    }

    // double channels
    samplesOut = (int16_t*)output;
    for (int j = expect_len - 1; j > 0; j--) {
      samplesOut[2 * j] = samplesOut[j];
      samplesOut[2 * j - 1] = samplesOut[j];
    }
    // 10
    // 9 18 <- 9 17 <- 9
    // 8 16 <- 8 15 <- 8
    // ...
    // 1 2 <- 1 1 <- 1

    return expect_len;
  }
};

Resampler::Resampler()
    : state1_(nullptr), state2_(nullptr), state3_(nullptr), in_buffer_(nullptr), out_buffer_(nullptr),
      in_buffer_size_(0), out_buffer_size_(0), in_buffer_size_max_(0), out_buffer_size_max_(0), my_in_frequency_khz_(0),
      my_out_frequency_khz_(0), my_mode_(kResamplerMode1To1), num_channels_(0), slave_left_(nullptr),
      slave_right_(nullptr) {}

Resampler::Resampler(size_t inFreq, size_t outFreq, size_t num_channels) : Resampler() {
  Reset(inFreq, outFreq, num_channels);
}

Resampler::~Resampler() {
  if (state1_) { free(state1_); }
  if (state2_) { free(state2_); }
  if (state3_) { free(state3_); }
  if (in_buffer_) { free(in_buffer_); }
  if (out_buffer_) { free(out_buffer_); }

  delete slave_left_;
  delete slave_right_;
}

int Resampler::ResetIfNeeded(size_t inFreq, size_t outFreq, size_t num_channels) {
  size_t tmpInFreq_kHz = inFreq / 1000;
  size_t tmpOutFreq_kHz = outFreq / 1000;

  if ((tmpInFreq_kHz != my_in_frequency_khz_) || (tmpOutFreq_kHz != my_out_frequency_khz_) ||
      (num_channels != num_channels_)) {
    return Reset(inFreq, outFreq, num_channels);
  } else {
    return 0;
  }
}

int Resampler::Reset(size_t inFreq, size_t outFreq, size_t num_channels) {
  if (num_channels != 1 && num_channels != 2) {
    printf("Reset() called with unsupported channel count, num_channels = %d .\n", num_channels);
    return -1;
  }
  ResamplerMode mode;
  if (ComputeResamplerMode(inFreq, outFreq, &mode) != 0) {
    printf("Reset() called with unsupported sample rates, inFreq = %d , outFreq = %d .\n", inFreq, outFreq);
    return -1;
  }
  // Reinitialize internal state for the frequencies and sample rates.
  num_channels_ = num_channels;
  my_mode_ = mode;

  if (state1_) {
    free(state1_);
    state1_ = nullptr;
  }
  if (state2_) {
    free(state2_);
    state2_ = nullptr;
  }
  if (state3_) {
    free(state3_);
    state3_ = nullptr;
  }
  if (in_buffer_) {
    free(in_buffer_);
    in_buffer_ = nullptr;
  }
  if (out_buffer_) {
    free(out_buffer_);
    out_buffer_ = nullptr;
  }
  if (slave_left_) {
    delete slave_left_;
    slave_left_ = nullptr;
  }
  if (slave_right_) {
    delete slave_right_;
    slave_right_ = nullptr;
  }

  in_buffer_size_ = 0;
  out_buffer_size_ = 0;
  in_buffer_size_max_ = 0;
  out_buffer_size_max_ = 0;

  // We need to track what domain we're in.
  my_in_frequency_khz_ = inFreq / 1000;
  my_out_frequency_khz_ = outFreq / 1000;

  if (num_channels_ == 2) {
    // Create two mono resamplers.
    slave_left_ = new Resampler(inFreq, outFreq, 1);
    slave_right_ = new Resampler(inFreq, outFreq, 1);
  }

  // Now create the states we need.
  switch (my_mode_) {
  case kResamplerMode1To1:
    // No state needed;
    break;
  case kResamplerMode1To2:
    state1_ = malloc(8 * sizeof(int32_t));
    memset(state1_, 0, 8 * sizeof(int32_t));
    break;
  case kResamplerMode1To3:
    state1_ = malloc(sizeof(WebRtcSpl_State16khzTo48khz));
    WebRtcSpl_ResetResample16khzTo48khz(static_cast<WebRtcSpl_State16khzTo48khz*>(state1_));
    break;
  case kResamplerMode1To4:
    // 1:2
    state1_ = malloc(8 * sizeof(int32_t));
    memset(state1_, 0, 8 * sizeof(int32_t));
    // 2:4
    state2_ = malloc(8 * sizeof(int32_t));
    memset(state2_, 0, 8 * sizeof(int32_t));
    break;
  case kResamplerMode1To6:
    // 1:2
    state1_ = malloc(8 * sizeof(int32_t));
    memset(state1_, 0, 8 * sizeof(int32_t));
    // 2:6
    state2_ = malloc(sizeof(WebRtcSpl_State16khzTo48khz));
    WebRtcSpl_ResetResample16khzTo48khz(static_cast<WebRtcSpl_State16khzTo48khz*>(state2_));
    break;
  case kResamplerMode1To12:
    // 1:2
    state1_ = malloc(8 * sizeof(int32_t));
    memset(state1_, 0, 8 * sizeof(int32_t));
    // 2:4
    state2_ = malloc(8 * sizeof(int32_t));
    memset(state2_, 0, 8 * sizeof(int32_t));
    // 4:12
    state3_ = malloc(sizeof(WebRtcSpl_State16khzTo48khz));
    WebRtcSpl_ResetResample16khzTo48khz(static_cast<WebRtcSpl_State16khzTo48khz*>(state3_));
    break;
  case kResamplerMode2To3:
    // 2:6
    state1_ = malloc(sizeof(WebRtcSpl_State16khzTo48khz));
    WebRtcSpl_ResetResample16khzTo48khz(static_cast<WebRtcSpl_State16khzTo48khz*>(state1_));
    // 6:3
    state2_ = malloc(8 * sizeof(int32_t));
    memset(state2_, 0, 8 * sizeof(int32_t));
    break;
  case kResamplerMode2To11:
    state1_ = malloc(8 * sizeof(int32_t));
    memset(state1_, 0, 8 * sizeof(int32_t));

    state2_ = malloc(sizeof(WebRtcSpl_State8khzTo22khz));
    WebRtcSpl_ResetResample8khzTo22khz(static_cast<WebRtcSpl_State8khzTo22khz*>(state2_));
    break;
  case kResamplerMode4To11:
    state1_ = malloc(sizeof(WebRtcSpl_State8khzTo22khz));
    WebRtcSpl_ResetResample8khzTo22khz(static_cast<WebRtcSpl_State8khzTo22khz*>(state1_));
    break;
  case kResamplerMode8To11:
    state1_ = malloc(sizeof(WebRtcSpl_State16khzTo22khz));
    WebRtcSpl_ResetResample16khzTo22khz(static_cast<WebRtcSpl_State16khzTo22khz*>(state1_));
    break;
  case kResamplerMode11To16:
    state1_ = malloc(8 * sizeof(int32_t));
    memset(state1_, 0, 8 * sizeof(int32_t));

    state2_ = malloc(sizeof(WebRtcSpl_State22khzTo16khz));
    WebRtcSpl_ResetResample22khzTo16khz(static_cast<WebRtcSpl_State22khzTo16khz*>(state2_));
    break;
  case kResamplerMode11To32:
    // 11 -> 22
    state1_ = malloc(8 * sizeof(int32_t));
    memset(state1_, 0, 8 * sizeof(int32_t));

    // 22 -> 16
    state2_ = malloc(sizeof(WebRtcSpl_State22khzTo16khz));
    WebRtcSpl_ResetResample22khzTo16khz(static_cast<WebRtcSpl_State22khzTo16khz*>(state2_));

    // 16 -> 32
    state3_ = malloc(8 * sizeof(int32_t));
    memset(state3_, 0, 8 * sizeof(int32_t));

    break;
  case kResamplerMode2To1:
    state1_ = malloc(8 * sizeof(int32_t));
    memset(state1_, 0, 8 * sizeof(int32_t));
    break;
  case kResamplerMode3To1:
    state1_ = malloc(sizeof(WebRtcSpl_State48khzTo16khz));
    WebRtcSpl_ResetResample48khzTo16khz(static_cast<WebRtcSpl_State48khzTo16khz*>(state1_));
    break;
  case kResamplerMode4To1:
    // 4:2
    state1_ = malloc(8 * sizeof(int32_t));
    memset(state1_, 0, 8 * sizeof(int32_t));
    // 2:1
    state2_ = malloc(8 * sizeof(int32_t));
    memset(state2_, 0, 8 * sizeof(int32_t));
    break;
  case kResamplerMode6To1:
    // 6:2
    state1_ = malloc(sizeof(WebRtcSpl_State48khzTo16khz));
    WebRtcSpl_ResetResample48khzTo16khz(static_cast<WebRtcSpl_State48khzTo16khz*>(state1_));
    // 2:1
    state2_ = malloc(8 * sizeof(int32_t));
    memset(state2_, 0, 8 * sizeof(int32_t));
    break;
  case kResamplerMode12To1:
    // 12:4
    state1_ = malloc(sizeof(WebRtcSpl_State48khzTo16khz));
    WebRtcSpl_ResetResample48khzTo16khz(static_cast<WebRtcSpl_State48khzTo16khz*>(state1_));
    // 4:2
    state2_ = malloc(8 * sizeof(int32_t));
    memset(state2_, 0, 8 * sizeof(int32_t));
    // 2:1
    state3_ = malloc(8 * sizeof(int32_t));
    memset(state3_, 0, 8 * sizeof(int32_t));
    break;
  case kResamplerMode3To2:
    // 3:6
    state1_ = malloc(8 * sizeof(int32_t));
    memset(state1_, 0, 8 * sizeof(int32_t));
    // 6:2
    state2_ = malloc(sizeof(WebRtcSpl_State48khzTo16khz));
    WebRtcSpl_ResetResample48khzTo16khz(static_cast<WebRtcSpl_State48khzTo16khz*>(state2_));
    break;
  case kResamplerMode11To2:
    state1_ = malloc(sizeof(WebRtcSpl_State22khzTo8khz));
    WebRtcSpl_ResetResample22khzTo8khz(static_cast<WebRtcSpl_State22khzTo8khz*>(state1_));

    state2_ = malloc(8 * sizeof(int32_t));
    memset(state2_, 0, 8 * sizeof(int32_t));

    break;
  case kResamplerMode11To4:
    state1_ = malloc(sizeof(WebRtcSpl_State22khzTo8khz));
    WebRtcSpl_ResetResample22khzTo8khz(static_cast<WebRtcSpl_State22khzTo8khz*>(state1_));
    break;
  case kResamplerMode11To8:
    state1_ = malloc(sizeof(WebRtcSpl_State22khzTo16khz));
    WebRtcSpl_ResetResample22khzTo16khz(static_cast<WebRtcSpl_State22khzTo16khz*>(state1_));
    break;
  }

  return 0;
}

int Resampler::ComputeResamplerMode(int in_freq_hz, int out_freq_hz, ResamplerMode* mode) {
  // Start with a math exercise, Euclid's algorithm to find the gcd:
  int a = in_freq_hz;
  int b = out_freq_hz;
  int c = a % b;
  while (c != 0) {
    a = b;
    b = c;
    c = a % b;
  }
  // b is now the gcd;

  // Scale with GCD
  const int reduced_in_freq = in_freq_hz / b;
  const int reduced_out_freq = out_freq_hz / b;

  if (reduced_in_freq == reduced_out_freq) {
    *mode = kResamplerMode1To1;
  } else if (reduced_in_freq == 1) {
    switch (reduced_out_freq) {
    case 2: *mode = kResamplerMode1To2; break;
    case 3: *mode = kResamplerMode1To3; break;
    case 4: *mode = kResamplerMode1To4; break;
    case 6: *mode = kResamplerMode1To6; break;
    case 12: *mode = kResamplerMode1To12; break;
    default: return -1;
    }
  } else if (reduced_out_freq == 1) {
    switch (reduced_in_freq) {
    case 2: *mode = kResamplerMode2To1; break;
    case 3: *mode = kResamplerMode3To1; break;
    case 4: *mode = kResamplerMode4To1; break;
    case 6: *mode = kResamplerMode6To1; break;
    case 12: *mode = kResamplerMode12To1; break;
    default: return -1;
    }
  } else if ((reduced_in_freq == 2) && (reduced_out_freq == 3)) {
    *mode = kResamplerMode2To3;
  } else if ((reduced_in_freq == 2) && (reduced_out_freq == 11)) {
    *mode = kResamplerMode2To11;
  } else if ((reduced_in_freq == 4) && (reduced_out_freq == 11)) {
    *mode = kResamplerMode4To11;
  } else if ((reduced_in_freq == 8) && (reduced_out_freq == 11)) {
    *mode = kResamplerMode8To11;
  } else if ((reduced_in_freq == 3) && (reduced_out_freq == 2)) {
    *mode = kResamplerMode3To2;
  } else if ((reduced_in_freq == 11) && (reduced_out_freq == 2)) {
    *mode = kResamplerMode11To2;
  } else if ((reduced_in_freq == 11) && (reduced_out_freq == 4)) {
    *mode = kResamplerMode11To4;
  } else if ((reduced_in_freq == 11) && (reduced_out_freq == 16)) {
    *mode = kResamplerMode11To16;
  } else if ((reduced_in_freq == 11) && (reduced_out_freq == 32)) {
    *mode = kResamplerMode11To32;
  } else if ((reduced_in_freq == 11) && (reduced_out_freq == 8)) {
    *mode = kResamplerMode11To8;
  } else {
    return -1;
  }
  return 0;
}

// Synchronous resampling, all output samples are written to samplesOut
int Resampler::Push(const int16_t* samplesIn, size_t lengthIn, int16_t* samplesOut, size_t maxLen, size_t& outLen) {
  if (num_channels_ == 2) {
    // Split up the signal and call the slave object for each channel
    int16_t* left = static_cast<int16_t*>(malloc(lengthIn * sizeof(int16_t) / 2));
    int16_t* right = static_cast<int16_t*>(malloc(lengthIn * sizeof(int16_t) / 2));
    int16_t* out_left = static_cast<int16_t*>(malloc(maxLen / 2 * sizeof(int16_t)));
    int16_t* out_right = static_cast<int16_t*>(malloc(maxLen / 2 * sizeof(int16_t)));
    if ((left == nullptr) || (right == nullptr) || (out_left == nullptr) || (out_right == nullptr)) {
      if (left) free(left);
      if (right) free(right);
      if (out_left) free(out_left);
      if (out_right) free(out_right);
      return -1;
    }
    int res = 0;
    for (size_t i = 0; i < lengthIn; i += 2) {
      left[i >> 1] = samplesIn[i];
      right[i >> 1] = samplesIn[i + 1];
    }

    // It's OK to overwrite the local parameter, since it's just a copy
    lengthIn = lengthIn / 2;

    size_t actualOutLen_left = 0;
    size_t actualOutLen_right = 0;
    // Do resampling for right channel
    res |= slave_left_->Push(left, lengthIn, out_left, maxLen / 2, actualOutLen_left);
    res |= slave_right_->Push(right, lengthIn, out_right, maxLen / 2, actualOutLen_right);
    if (res || (actualOutLen_left != actualOutLen_right)) {
      free(left);
      free(right);
      free(out_left);
      free(out_right);
      return -1;
    }

    // Reassemble the signal
    for (size_t i = 0; i < actualOutLen_left; i++) {
      samplesOut[i * 2] = out_left[i];
      samplesOut[i * 2 + 1] = out_right[i];
    }
    outLen = 2 * actualOutLen_left;

    free(left);
    free(right);
    free(out_left);
    free(out_right);

    return 0;
  }

  // Containers for temp samples
  int16_t* tmp;
  int16_t* tmp_2;
  // tmp data for resampling routines
  int32_t* tmp_mem;

  switch (my_mode_) {
  case kResamplerMode1To1:
    memcpy(samplesOut, samplesIn, lengthIn * sizeof(int16_t));
    outLen = lengthIn;
    break;
  case kResamplerMode1To2:
    if (maxLen < (lengthIn * 2)) { return -1; }
    WebRtcSpl_UpsampleBy2(samplesIn, lengthIn, samplesOut, static_cast<int32_t*>(state1_));
    outLen = lengthIn * 2;
    return 0;
  case kResamplerMode1To3:

    // We can only handle blocks of 160 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 160) != 0) { return -1; }
    if (maxLen < (lengthIn * 3)) { return -1; }
    tmp_mem = static_cast<int32_t*>(malloc(336 * sizeof(int32_t)));
    if (tmp_mem == nullptr) { return -1; }
    for (size_t i = 0; i < lengthIn; i += 160) {
      WebRtcSpl_Resample16khzTo48khz(samplesIn + i, samplesOut + i * 3,
                                     static_cast<WebRtcSpl_State16khzTo48khz*>(state1_), tmp_mem);
    }
    outLen = lengthIn * 3;
    free(tmp_mem);
    return 0;
  case kResamplerMode1To4:
    if (maxLen < (lengthIn * 4)) { return -1; }

    tmp = static_cast<int16_t*>(malloc(sizeof(int16_t) * 2 * lengthIn));
    // 1:2
    WebRtcSpl_UpsampleBy2(samplesIn, lengthIn, tmp, static_cast<int32_t*>(state1_));
    // 2:4
    WebRtcSpl_UpsampleBy2(tmp, lengthIn * 2, samplesOut, static_cast<int32_t*>(state2_));
    outLen = lengthIn * 4;
    free(tmp);
    return 0;
  case kResamplerMode1To6:
    // We can only handle blocks of 80 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 80) != 0) { return -1; }
    if (maxLen < (lengthIn * 6)) { return -1; }

    // 1:2

    tmp_mem = static_cast<int32_t*>(malloc(336 * sizeof(int32_t)));
    tmp = static_cast<int16_t*>(malloc(sizeof(int16_t) * 2 * lengthIn));
    if (tmp_mem == nullptr || tmp == nullptr) {
      if (tmp_mem) free(tmp_mem);
      if (tmp) free(tmp);
      return -1;
    }
    WebRtcSpl_UpsampleBy2(samplesIn, lengthIn, tmp, static_cast<int32_t*>(state1_));
    outLen = lengthIn * 2;

    for (size_t i = 0; i < outLen; i += 160) {
      WebRtcSpl_Resample16khzTo48khz(tmp + i, samplesOut + i * 3, static_cast<WebRtcSpl_State16khzTo48khz*>(state2_),
                                     tmp_mem);
    }
    outLen = outLen * 3;
    free(tmp_mem);
    free(tmp);

    return 0;
  case kResamplerMode1To12:
    // We can only handle blocks of 40 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 40) != 0) { return -1; }
    if (maxLen < (lengthIn * 12)) { return -1; }

    tmp_mem = static_cast<int32_t*>(malloc(336 * sizeof(int32_t)));
    tmp = static_cast<int16_t*>(malloc(sizeof(int16_t) * 4 * lengthIn));
    if (tmp_mem == nullptr || tmp == nullptr) {
      if (tmp_mem) free(tmp_mem);
      if (tmp) free(tmp);
      return -1;
    }  // 1:2
    WebRtcSpl_UpsampleBy2(samplesIn, lengthIn, samplesOut, static_cast<int32_t*>(state1_));
    outLen = lengthIn * 2;
    // 2:4
    WebRtcSpl_UpsampleBy2(samplesOut, outLen, tmp, static_cast<int32_t*>(state2_));
    outLen = outLen * 2;
    // 4:12
    for (size_t i = 0; i < outLen; i += 160) {
      // WebRtcSpl_Resample16khzTo48khz() takes a block of 160 samples
      // as input and outputs a resampled block of 480 samples. The
      // data is now actually in 32 kHz sampling rate, despite the
      // function name, and with a resampling factor of three becomes
      // 96 kHz.
      WebRtcSpl_Resample16khzTo48khz(tmp + i, samplesOut + i * 3, static_cast<WebRtcSpl_State16khzTo48khz*>(state3_),
                                     tmp_mem);
    }
    outLen = outLen * 3;
    free(tmp_mem);
    free(tmp);

    return 0;
  case kResamplerMode2To3:
    if (maxLen < (lengthIn * 3 / 2)) { return -1; }
    // 2:6
    // We can only handle blocks of 160 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 160) != 0) { return -1; }
    tmp = static_cast<int16_t*>(malloc(sizeof(int16_t) * lengthIn * 3));
    tmp_mem = static_cast<int32_t*>(malloc(336 * sizeof(int32_t)));
    if (tmp_mem == nullptr || tmp == nullptr) {
      if (tmp_mem) free(tmp_mem);
      if (tmp) free(tmp);
      return -1;
    }
    for (size_t i = 0; i < lengthIn; i += 160) {
      WebRtcSpl_Resample16khzTo48khz(samplesIn + i, tmp + i * 3, static_cast<WebRtcSpl_State16khzTo48khz*>(state1_),
                                     tmp_mem);
    }
    lengthIn = lengthIn * 3;
    // 6:3
    WebRtcSpl_DownsampleBy2(tmp, lengthIn, samplesOut, static_cast<int32_t*>(state2_));
    outLen = lengthIn / 2;
    free(tmp);
    free(tmp_mem);
    return 0;
  case kResamplerMode2To11:

    // We can only handle blocks of 80 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 80) != 0) { return -1; }
    if (maxLen < ((lengthIn * 11) / 2)) { return -1; }
    tmp = static_cast<int16_t*>(malloc(sizeof(int16_t) * 2 * lengthIn));
    tmp_mem = static_cast<int32_t*>(malloc(98 * sizeof(int32_t)));
    if (tmp_mem == nullptr || tmp == nullptr) {
      if (tmp_mem) free(tmp_mem);
      if (tmp) free(tmp);
      return -1;
    }
    // 1:2
    WebRtcSpl_UpsampleBy2(samplesIn, lengthIn, tmp, static_cast<int32_t*>(state1_));
    lengthIn *= 2;
    for (size_t i = 0; i < lengthIn; i += 80) {
      WebRtcSpl_Resample8khzTo22khz(tmp + i, samplesOut + (i * 11) / 4,
                                    static_cast<WebRtcSpl_State8khzTo22khz*>(state2_), tmp_mem);
    }
    outLen = (lengthIn * 11) / 4;
    free(tmp_mem);
    free(tmp);
    return 0;
  case kResamplerMode4To11:

    // We can only handle blocks of 80 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 80) != 0) { return -1; }
    if (maxLen < ((lengthIn * 11) / 4)) { return -1; }
    tmp_mem = static_cast<int32_t*>(malloc(98 * sizeof(int32_t)));
    if (tmp_mem == nullptr) { return -1; }
    for (size_t i = 0; i < lengthIn; i += 80) {
      WebRtcSpl_Resample8khzTo22khz(samplesIn + i, samplesOut + (i * 11) / 4,
                                    static_cast<WebRtcSpl_State8khzTo22khz*>(state1_), tmp_mem);
    }
    outLen = (lengthIn * 11) / 4;
    free(tmp_mem);
    return 0;
  case kResamplerMode8To11:
    // We can only handle blocks of 160 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 160) != 0) { return -1; }
    if (maxLen < ((lengthIn * 11) / 8)) { return -1; }
    tmp_mem = static_cast<int32_t*>(malloc(88 * sizeof(int32_t)));
    if (tmp_mem == nullptr) { return -1; }
    for (size_t i = 0; i < lengthIn; i += 160) {
      WebRtcSpl_Resample16khzTo22khz(samplesIn + i, samplesOut + (i * 11) / 8,
                                     static_cast<WebRtcSpl_State16khzTo22khz*>(state1_), tmp_mem);
    }
    outLen = (lengthIn * 11) / 8;
    free(tmp_mem);
    return 0;

  case kResamplerMode11To16:
    // We can only handle blocks of 110 samples
    if ((lengthIn % 110) != 0) { return -1; }
    if (maxLen < ((lengthIn * 16) / 11)) { return -1; }

    tmp_mem = static_cast<int32_t*>(malloc(104 * sizeof(int32_t)));
    tmp = static_cast<int16_t*>(malloc((sizeof(int16_t) * lengthIn * 2)));
    if (tmp_mem == nullptr || tmp == nullptr) {
      if (tmp_mem) free(tmp_mem);
      if (tmp) free(tmp);
      return -1;
    }
    WebRtcSpl_UpsampleBy2(samplesIn, lengthIn, tmp, static_cast<int32_t*>(state1_));

    for (size_t i = 0; i < (lengthIn * 2); i += 220) {
      WebRtcSpl_Resample22khzTo16khz(tmp + i, samplesOut + (i / 220) * 160,
                                     static_cast<WebRtcSpl_State22khzTo16khz*>(state2_), tmp_mem);
    }

    outLen = (lengthIn * 16) / 11;

    free(tmp_mem);
    free(tmp);
    return 0;

  case kResamplerMode11To32:

    // We can only handle blocks of 110 samples
    if ((lengthIn % 110) != 0) { return -1; }
    if (maxLen < ((lengthIn * 32) / 11)) { return -1; }

    tmp_mem = static_cast<int32_t*>(malloc(104 * sizeof(int32_t)));
    tmp = static_cast<int16_t*>(malloc((sizeof(int16_t) * lengthIn * 2)));
    if (tmp_mem == nullptr || tmp == nullptr) {
      if (tmp_mem) free(tmp_mem);
      if (tmp) free(tmp);
      return -1;
    }
    // 11 -> 22 kHz in samplesOut
    WebRtcSpl_UpsampleBy2(samplesIn, lengthIn, samplesOut, static_cast<int32_t*>(state1_));

    // 22 -> 16 in tmp
    for (size_t i = 0; i < (lengthIn * 2); i += 220) {
      WebRtcSpl_Resample22khzTo16khz(samplesOut + i, tmp + (i / 220) * 160,
                                     static_cast<WebRtcSpl_State22khzTo16khz*>(state2_), tmp_mem);
    }

    // 16 -> 32 in samplesOut
    WebRtcSpl_UpsampleBy2(tmp, (lengthIn * 16) / 11, samplesOut, static_cast<int32_t*>(state3_));

    outLen = (lengthIn * 32) / 11;

    free(tmp_mem);
    free(tmp);
    return 0;

  case kResamplerMode2To1:
    if (maxLen < (lengthIn / 2)) { return -1; }
    WebRtcSpl_DownsampleBy2(samplesIn, lengthIn, samplesOut, static_cast<int32_t*>(state1_));
    outLen = lengthIn / 2;
    return 0;
  case kResamplerMode3To1:
    // We can only handle blocks of 480 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 480) != 0) { return -1; }
    if (maxLen < (lengthIn / 3)) { return -1; }
    tmp_mem = static_cast<int32_t*>(malloc(496 * sizeof(int32_t)));
    if (tmp_mem == nullptr) { return -1; }
    for (size_t i = 0; i < lengthIn; i += 480) {
      WebRtcSpl_Resample48khzTo16khz(samplesIn + i, samplesOut + i / 3,
                                     static_cast<WebRtcSpl_State48khzTo16khz*>(state1_), tmp_mem);
    }
    outLen = lengthIn / 3;
    free(tmp_mem);
    return 0;
  case kResamplerMode4To1:
    if (maxLen < (lengthIn / 4)) { return -1; }
    tmp = static_cast<int16_t*>(malloc(sizeof(int16_t) * lengthIn / 2));
    if (tmp == nullptr) { return -1; }  // 4:2
    WebRtcSpl_DownsampleBy2(samplesIn, lengthIn, tmp, static_cast<int32_t*>(state1_));
    // 2:1
    WebRtcSpl_DownsampleBy2(tmp, lengthIn / 2, samplesOut, static_cast<int32_t*>(state2_));
    outLen = lengthIn / 4;
    free(tmp);
    return 0;

  case kResamplerMode6To1:
    // We can only handle blocks of 480 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 480) != 0) { return -1; }
    if (maxLen < (lengthIn / 6)) { return -1; }

    tmp_mem = static_cast<int32_t*>(malloc(496 * sizeof(int32_t)));
    tmp = static_cast<int16_t*>(malloc((sizeof(int16_t) * lengthIn) / 3));
    if (tmp_mem == nullptr || tmp == nullptr) {
      if (tmp_mem) free(tmp_mem);
      if (tmp) free(tmp);
      return -1;
    }
    for (size_t i = 0; i < lengthIn; i += 480) {
      WebRtcSpl_Resample48khzTo16khz(samplesIn + i, tmp + i / 3, static_cast<WebRtcSpl_State48khzTo16khz*>(state1_),
                                     tmp_mem);
    }
    outLen = lengthIn / 3;
    free(tmp_mem);
    WebRtcSpl_DownsampleBy2(tmp, outLen, samplesOut, static_cast<int32_t*>(state2_));
    free(tmp);
    outLen = outLen / 2;
    return 0;
  case kResamplerMode12To1:
    // We can only handle blocks of 480 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 480) != 0) { return -1; }
    if (maxLen < (lengthIn / 12)) { return -1; }

    tmp_mem = static_cast<int32_t*>(malloc(496 * sizeof(int32_t)));
    tmp = static_cast<int16_t*>(malloc((sizeof(int16_t) * lengthIn) / 3));
    tmp_2 = static_cast<int16_t*>(malloc((sizeof(int16_t) * lengthIn) / 6));
    if (tmp_mem == nullptr || tmp_2 == nullptr || tmp == nullptr) {
      if (tmp_mem) free(tmp_mem);
      if (tmp) free(tmp);
      if (tmp_2) free(tmp_2);
      return -1;
    }  // 12:4
    for (size_t i = 0; i < lengthIn; i += 480) {
      // WebRtcSpl_Resample48khzTo16khz() takes a block of 480 samples
      // as input and outputs a resampled block of 160 samples. The
      // data is now actually in 96 kHz sampling rate, despite the
      // function name, and with a resampling factor of 1/3 becomes
      // 32 kHz.
      WebRtcSpl_Resample48khzTo16khz(samplesIn + i, tmp + i / 3, static_cast<WebRtcSpl_State48khzTo16khz*>(state1_),
                                     tmp_mem);
    }
    outLen = lengthIn / 3;
    free(tmp_mem);
    // 4:2
    WebRtcSpl_DownsampleBy2(tmp, outLen, tmp_2, static_cast<int32_t*>(state2_));
    outLen = outLen / 2;
    free(tmp);
    // 2:1
    WebRtcSpl_DownsampleBy2(tmp_2, outLen, samplesOut, static_cast<int32_t*>(state3_));
    free(tmp_2);
    outLen = outLen / 2;
    return 0;
  case kResamplerMode3To2:
    if (maxLen < (lengthIn * 2 / 3)) { return -1; }
    // 3:6
    tmp = static_cast<int16_t*>(malloc(sizeof(int16_t) * lengthIn * 2));
    if (tmp == nullptr) { return -1; }
    WebRtcSpl_UpsampleBy2(samplesIn, lengthIn, tmp, static_cast<int32_t*>(state1_));
    lengthIn *= 2;
    // 6:2
    // We can only handle blocks of 480 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 480) != 0) {
      free(tmp);
      return -1;
    }
    tmp_mem = static_cast<int32_t*>(malloc(496 * sizeof(int32_t)));
    if (tmp_mem == nullptr) { return -1; }
    for (size_t i = 0; i < lengthIn; i += 480) {
      WebRtcSpl_Resample48khzTo16khz(tmp + i, samplesOut + i / 3, static_cast<WebRtcSpl_State48khzTo16khz*>(state2_),
                                     tmp_mem);
    }
    outLen = lengthIn / 3;
    free(tmp);
    free(tmp_mem);
    return 0;
  case kResamplerMode11To2:
    // We can only handle blocks of 220 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 220) != 0) { return -1; }
    if (maxLen < ((lengthIn * 2) / 11)) { return -1; }
    tmp_mem = static_cast<int32_t*>(malloc(126 * sizeof(int32_t)));
    tmp = static_cast<int16_t*>(malloc((lengthIn * 4) / 11 * sizeof(int16_t)));
    if (tmp_mem == nullptr || tmp == nullptr) {
      if (tmp) free(tmp);
      if (tmp_mem) free(tmp_mem);
      return -1;
    }
    for (size_t i = 0; i < lengthIn; i += 220) {
      WebRtcSpl_Resample22khzTo8khz(samplesIn + i, tmp + (i * 4) / 11,
                                    static_cast<WebRtcSpl_State22khzTo8khz*>(state1_), tmp_mem);
    }
    lengthIn = (lengthIn * 4) / 11;

    WebRtcSpl_DownsampleBy2(tmp, lengthIn, samplesOut, static_cast<int32_t*>(state2_));
    outLen = lengthIn / 2;

    free(tmp_mem);
    free(tmp);
    return 0;
  case kResamplerMode11To4:
    // We can only handle blocks of 220 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 220) != 0) { return -1; }
    if (maxLen < ((lengthIn * 4) / 11)) { return -1; }
    tmp_mem = static_cast<int32_t*>(malloc(126 * sizeof(int32_t)));
    if (tmp_mem == nullptr) { return -1; }
    for (size_t i = 0; i < lengthIn; i += 220) {
      WebRtcSpl_Resample22khzTo8khz(samplesIn + i, samplesOut + (i * 4) / 11,
                                    static_cast<WebRtcSpl_State22khzTo8khz*>(state1_), tmp_mem);
    }
    outLen = (lengthIn * 4) / 11;
    free(tmp_mem);
    return 0;
  case kResamplerMode11To8:
    // We can only handle blocks of 160 samples
    // Can be fixed, but I don't think it's needed
    if ((lengthIn % 220) != 0) { return -1; }
    if (maxLen < ((lengthIn * 8) / 11)) { return -1; }
    tmp_mem = static_cast<int32_t*>(malloc(104 * sizeof(int32_t)));
    if (tmp_mem == nullptr) { return -1; }
    for (size_t i = 0; i < lengthIn; i += 220) {
      WebRtcSpl_Resample22khzTo16khz(samplesIn + i, samplesOut + (i * 8) / 11,
                                     static_cast<WebRtcSpl_State22khzTo16khz*>(state1_), tmp_mem);
    }
    outLen = (lengthIn * 8) / 11;
    free(tmp_mem);
    return 0;
    break;
  }
  return 0;
}





std::unique_ptr<IPreProcesss> createReSamplerProcess(ProcessOption const& option);
std::unique_ptr<IPreProcesss> createReSamplerProcess(ProcessOption const& option) {
  return std::make_unique<ReSamplerProcess>(option);
}


void NoiseSuppression32(char *szFileIn,char *szFileOut,int nSample,int nMode)
{
	int nRet = 0;
	NsHandle *pNS_inst = NULL;

	FILE *fpIn = NULL;
	FILE *fpOut = NULL;

	char *pInBuffer =NULL;
	char *pOutBuffer = NULL;

	do
	{
		int i = 0;
		int nFileSize = 0;
		int nTime = 0;
		if (0 != WebRtcNs_Create(&pNS_inst))
		{
			printf("Noise_Suppression WebRtcNs_Create err! \n");
			break;
		}

		if (0 !=  WebRtcNs_Init(pNS_inst,nSample))
		{
			printf("Noise_Suppression WebRtcNs_Init err! \n");
			break;
		}

		if (0 !=  WebRtcNs_set_policy(pNS_inst,nMode))
		{
			printf("Noise_Suppression WebRtcNs_set_policy err! \n");
			break;
		}

		fpIn = fopen(szFileIn, "rb");
		if (NULL == fpIn)
		{
			printf("open src file err \n");
			break;
		}
		fseek(fpIn,0,SEEK_END);
		nFileSize = ftell(fpIn); 
		printf("nFileSize:% \n",nFileSize);

		ProcessOption option;
  		int sampleRate = 32000;  // OUT
		size_t res = 0;
		auto resampler = createReSamplerProcess(option);
  		// res = resampler->process(in_buffer, in_size, out_buffer, out_size);

		fseek(fpIn,0,SEEK_SET); 

		pInBuffer = (char*)malloc(nFileSize);
		memset(pInBuffer,0,nFileSize);
		fread(pInBuffer, sizeof(char), nFileSize, fpIn);

		pOutBuffer = (char*)malloc(nFileSize/48*32);
		memset(pOutBuffer,0,nFileSize/48*32);

		int  filter_state1[6],filter_state12[6];
		int  Synthesis_state1[6],Synthesis_state12[6];

		memset(filter_state1,0,sizeof(filter_state1));
		memset(filter_state12,0,sizeof(filter_state12));
		memset(Synthesis_state1,0,sizeof(Synthesis_state1));
		memset(Synthesis_state12,0,sizeof(Synthesis_state12));

		// nTime = GetTickCount();
		nTime = 0;
		for (i = 0;i < nFileSize;i+=960)
		{
			if (nFileSize - i >= 960)
			{
				short shBufferIn[480] = {0};
				short shBufferInReSample[320] = {0};
				short shInL[160],shInH[160];
				short shOutL[160] = {0},shOutH[160] = {0};

				memcpy(shBufferIn,(char*)(pInBuffer+i),480*sizeof(short));

				res = resampler->process(shBufferIn, 480, shBufferInReSample, 320);
				
				//������Ҫʹ���˲���������Ƶ���ݷָߵ�Ƶ���Ը�Ƶ�͵�Ƶ�ķ�ʽ���뽵�뺯���ڲ�
				WebRtcSpl_AnalysisQMF(shBufferIn,320,shInL,shInH,filter_state1,filter_state12);

				//����Ҫ����������Ը�Ƶ�͵�Ƶ�����Ӧ�ӿڣ�ͬʱ��Ҫע�ⷵ������Ҳ�Ƿָ�Ƶ�͵�Ƶ
				if (0 == WebRtcNs_Process(pNS_inst ,shInL  ,shInH ,shOutL , shOutH))
				{
					short shBufferOut[320];
					//�������ɹ�������ݽ�����Ƶ�͵�Ƶ���ݴ����˲��ӿڣ�Ȼ���ý����ص�����д���ļ�
					WebRtcSpl_SynthesisQMF(shOutL,shOutH,160,shBufferOut,Synthesis_state1,Synthesis_state12);
					memcpy(pOutBuffer+i/48*32,shBufferOut,320*sizeof(short));
				}
			}	
		}

		// nTime = GetTickCount() - nTime;
		nTime = 0;
		printf("n_s user time=%dms\n",nTime);
		fpOut = fopen(szFileOut, "wb");
		if (NULL == fpOut)
		{
			printf("open out file err! \n");
			break;
		}
		fwrite(pOutBuffer, sizeof(char), nFileSize/48*32, fpOut);
	} while (0);

	WebRtcNs_Free(pNS_inst);
	fclose(fpIn);
	fclose(fpOut);
	free(pInBuffer);
	free(pOutBuffer);
}

void NoiseSuppressionX32(char *szFileIn,char *szFileOut,int nSample,int nMode)
{
	int nRet = 0;
	NsxHandle *pNS_inst = NULL;

	FILE *fpIn = NULL;
	FILE *fpOut = NULL;

	char *pInBuffer =NULL;
	char *pOutBuffer = NULL;

	do
	{
		int i = 0;
		int nFileSize = 0;
		int nTime = 0;
		if (0 != WebRtcNsx_Create(&pNS_inst))
		{
			printf("Noise_Suppression WebRtcNs_Create err! \n");
			break;
		}

		if (0 !=  WebRtcNsx_Init(pNS_inst,nSample))
		{
			printf("Noise_Suppression WebRtcNs_Init err! \n");
			break;
		}

		if (0 !=  WebRtcNsx_set_policy(pNS_inst,nMode))
		{
			printf("Noise_Suppression WebRtcNs_set_policy err! \n");
			break;
		}

		fpIn = fopen(szFileIn, "rb");
		if (NULL == fpIn)
		{
			printf("open src file err \n");
			break;
		}
		fseek(fpIn,0,SEEK_END);
		nFileSize = ftell(fpIn); 
		printf("nFileSize:%d \n",nFileSize);
		fseek(fpIn,0,SEEK_SET); 

		pInBuffer = (char*)malloc(nFileSize);
		memset(pInBuffer,0,nFileSize);
		fread(pInBuffer, sizeof(char), nFileSize, fpIn);

		pOutBuffer = (char*)malloc(nFileSize);
		memset(pOutBuffer,0,nFileSize);

		int  filter_state1[6],filter_state12[6];
		int  Synthesis_state1[6],Synthesis_state12[6];

		memset(filter_state1,0,sizeof(filter_state1));
		memset(filter_state12,0,sizeof(filter_state12));
		memset(Synthesis_state1,0,sizeof(Synthesis_state1));
		memset(Synthesis_state12,0,sizeof(Synthesis_state12));

		// nTime = GetTickCount();
		nTime = 0;
		for (i = 0;i < nFileSize;i+=640)
		{
			if (nFileSize - i >= 640)
			{
				short shBufferIn[320] = {0};

				short shInL[160],shInH[160];
				short shOutL[160] = {0},shOutH[160] = {0};

				memcpy(shBufferIn,(char*)(pInBuffer+i),320*sizeof(short));
				//������Ҫʹ���˲���������Ƶ���ݷָߵ�Ƶ���Ը�Ƶ�͵�Ƶ�ķ�ʽ���뽵�뺯���ڲ�
				WebRtcSpl_AnalysisQMF(shBufferIn,320,shInL,shInH,filter_state1,filter_state12);

				//����Ҫ����������Ը�Ƶ�͵�Ƶ�����Ӧ�ӿڣ�ͬʱ��Ҫע�ⷵ������Ҳ�Ƿָ�Ƶ�͵�Ƶ
				if (0 == WebRtcNsx_Process(pNS_inst ,shInL  ,shInH ,shOutL , shOutH))
				{
					short shBufferOut[320];
					//�������ɹ�������ݽ�����Ƶ�͵�Ƶ���ݴ����˲��ӿڣ�Ȼ���ý����ص�����д���ļ�
					WebRtcSpl_SynthesisQMF(shOutL,shOutH,160,shBufferOut,Synthesis_state1,Synthesis_state12);
					memcpy(pOutBuffer+i,shBufferOut,320*sizeof(short));
				}
			}	
			//丢弃最后的frame不足640
		}

		// nTime = GetTickCount() - nTime;
		nTime = 0;
		printf("n_s user time=%dms\n",nTime);
		fpOut = fopen(szFileOut, "wb");
		if (NULL == fpOut)
		{
			printf("open out file err! \n");
			break;
		}
		fwrite(pOutBuffer, sizeof(char), nFileSize, fpOut);
	} while (0);

	WebRtcNsx_Free(pNS_inst);
	fclose(fpIn);
	fclose(fpOut);
	free(pInBuffer);
	free(pOutBuffer);
}

void WebRtcAgcTest(char *filename, char *outfilename,int fs)
{
	FILE *infp      = NULL;
	FILE *outfp     = NULL;

	short *pData    = NULL;
	short *pOutData = NULL;
	void *agcHandle = NULL;	

	do 
	{
		WebRtcAgc_Create(&agcHandle);

		int minLevel = 0;
		int maxLevel = 255;
		int agcMode  = kAgcModeFixedDigital;
		WebRtcAgc_Init(agcHandle, minLevel, maxLevel, agcMode, fs);

		WebRtcAgc_config_t agcConfig;
		agcConfig.compressionGaindB = 20;
		agcConfig.limiterEnable     = 1;
		agcConfig.targetLevelDbfs   = 3;
		WebRtcAgc_set_config(agcHandle, agcConfig);

		infp = fopen(filename,"rb");
		int frameSize = 80;
		pData    = (short*)malloc(frameSize*sizeof(short));
		pOutData = (short*)malloc(frameSize*sizeof(short));

		outfp = fopen(outfilename,"wb");
		int len = frameSize*sizeof(short);
		int micLevelIn = 0;
		int micLevelOut = 0;
		while(1)
		{
			memset(pData, 0, len);
			len = fread(pData, 1, len, infp);
			if (len > 0)
			{
				int inMicLevel  = micLevelOut;
				int outMicLevel = 0;
				uint8_t saturationWarning;
				int nAgcRet = WebRtcAgc_Process(agcHandle, pData, NULL, frameSize, pOutData,NULL, inMicLevel, &outMicLevel, 0, &saturationWarning);
				if (nAgcRet != 0)
				{
					printf("failed in WebRtcAgc_Process\n");
					break;
				}
				micLevelIn = outMicLevel;
				fwrite(pOutData, 1, len, outfp);
			}
			else
			{
				break;
			}
		}
	} while (0);

	fclose(infp);
	fclose(outfp);
	free(pData);
	free(pOutData);
	WebRtcAgc_Free(agcHandle);
}

int WebRtcAecTest()
{
	#define  NN 160
	short far_frame[NN];
	short near_frame[NN];
	// float near_frame[NN];
	short out_frame[NN];


	void *aecmInst = NULL;
	FILE *fp_far  = fopen("speaker.pcm", "rb");
	FILE *fp_near = fopen("micin.pcm", "rb");
	FILE *fp_out  = fopen("out.pcm", "wb");

	do 
	{
		if(!fp_far)
		{
			printf("WebRtcAecTest open speaker.pcm file err \n");
			break;
		}
		if(!fp_near)
		{
			printf("WebRtcAecTest open micin.pcm file err \n");
			break;
		}
		if(!fp_out)
		{
			printf("WebRtcAecTest open out.pcm file err \n");
			break;
		}

		WebRtcAec_Create(&aecmInst);
		WebRtcAec_Init(aecmInst, 8000, 8000);

		AecConfig config;
		config.nlpMode = kAecNlpConservative;
		WebRtcAec_set_config(aecmInst, config);

		while(1)
		{
			if (NN == fread(far_frame, sizeof(short), NN, fp_far))
			{
				fread(near_frame, sizeof(short), NN, fp_near);

				WebRtcAec_BufferFarend(aecmInst, far_frame, NN);//�Բο�����(����)�Ĵ���


				WebRtcAec_Process(aecmInst, near_frame, NULL, out_frame, NULL, NN,40,0);//��������

				fwrite(out_frame, sizeof(short), NN, fp_out);
			}
			else
			{
				break;
			}
		}
	} while (0);

	fclose(fp_far);
	fclose(fp_near);
	fclose(fp_out);
	WebRtcAec_Free(aecmInst);
	return 0;
}


void resamplerTO32(char *szFileIn,char *szFileOut)
{
	int nFileSize = 0;
	FILE *fpIn = NULL;
	FILE *fpOut = NULL;
	char *pInBuffer = NULL;
	char *pOutBuffer = NULL;
	fpIn = fopen(szFileIn, "rb");
	if (NULL == fpIn)
	{
		printf("open src file err \n");
	}
	fseek(fpIn, 0, SEEK_END);
	nFileSize = ftell(fpIn);
	printf("nFileSize:% \n", nFileSize);

	ProcessOption option;
  	int sampleRate = 3200;  // OUT
	size_t res = 0;
	auto resampler = createReSamplerProcess(option);
	// res = resampler->process(in_buffer, in_size, out_buffer, out_size);

	fseek(fpIn, 0, SEEK_SET);

	pInBuffer = (char *)malloc(nFileSize);
	memset(pInBuffer, 0, nFileSize);
	fread(pInBuffer, sizeof(char), nFileSize, fpIn);

	pOutBuffer = (char *)malloc(nFileSize / 8 * 32);
	memset(pOutBuffer, 0, nFileSize / 8 * 32);
	int i=0;
	for (i = 0;i < nFileSize;i+=160)
	{
			if (nFileSize - i >= 160)
			{
				short shBufferIn[80] = {0};
				short shBufferOut[320] = {0};
				memcpy(shBufferIn,(char*)(pInBuffer+i),80*sizeof(short));
				res = resampler->process(shBufferIn, 80, shBufferOut, 320);	
				memcpy(pOutBuffer+i/8*32,shBufferOut,320*sizeof(short));
			}	
	}

	fpOut = fopen(szFileOut, "wb");
	if (NULL == fpOut)
	{
		printf("open out file err! \n");
	}
	fwrite(pOutBuffer, sizeof(char), nFileSize / 8 * 32, fpOut);
	fclose(fpIn);
	fclose(fpOut);
	free(pInBuffer);
	free(pOutBuffer);
 }

int main(int argc, char* argv[])
{
	resamplerTO32("/home/fengmao/cowa/webRTCtest/build/byby_8K_1C_16bit.pcm.32k.pcm","/home/fengmao/cowa/webRTCtest/build/byby_8K_1C_16bit.pcm");
	// WebRtcAecTest();
	// WebRtcAgcTest("byby_8K_1C_16bit.pcm","byby_8K_1C_16bit_agc.pcm",8000);
	//NoiseSuppression32("capture.pcm","capture.pcm_ns.pcm",32000,1);
	// NoiseSuppression32("lhydd_1C_16bit_32K.pcm","lhydd_1C_16bit_32K_ns.pcm",32000,1);

	// NoiseSuppressionX32("lhydd_1C_16bit_32K.pcm","lhydd_1C_16bit_32K_nsx.pcm",32000,1);

	printf("�������棬�������...\n");
	return 0;
}

