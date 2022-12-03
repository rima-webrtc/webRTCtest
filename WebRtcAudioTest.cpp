// WebRtcAudioTest.cpp : �������̨Ӧ�ó������ڵ㡣
//

// #include "stdafx.h"
// #include <stdio.h>
// #include <stdlib.h>
#include <string.h>
// #include <Windows.h>
#include <iostream>

#include "./WebRtcMoudle/echo_cancellation.h"
#include "./WebRtcMoudle/gain_control.h"
#include "./WebRtcMoudle/noise_suppression.h"
#include "./WebRtcMoudle/noise_suppression_x.h"
#include "./WebRtcMoudle/signal_processing_library.h"

#include "preprocess.h"
#include "resampler.h"

#include <fstream>

void resampler32NSX(char* szFileIn, char* szFileOut, char* szFileOut2, int nSample, int nMode) {
  std::ifstream in_stream;
  in_stream.open(szFileIn, std::ios::in | std::ios::binary);
  std::ofstream out_stream;
  out_stream.open(szFileOut, std::ios::out | std::ios::binary);
  std::ofstream out_stream2;
  out_stream2.open(szFileOut2, std::ios::out | std::ios::binary);

  int in_size = 960;
  unsigned char* in_buffer = (unsigned char*)malloc(in_size);
  unsigned char* out_buffer = (unsigned char*)malloc(in_size);
  unsigned int ans_size = in_size * 32 / 48;
  unsigned char* ans_buffer = (unsigned char*)malloc(ans_size);
  unsigned char* resample_buffer = (unsigned char*)malloc(ans_size);
  ProcessOption option;
  // wav or pcm
  in_stream.read((char*)in_buffer, 44);
  if (in_buffer[0] == 'R' && in_buffer[1] == 'I' && in_buffer[2] == 'F' && in_buffer[3] == 'F') {
    // wav
    in_stream.seekg(44, std::ios_base::beg);
  } else {
    in_stream.seekg(0, std::ios_base::beg);
  }

  int filter_state1[6], filter_state12[6];
  int Synthesis_state1[6], Synthesis_state12[6];


  ProcessOption option2;
  option2.sampleRate_in = 32000;

  ProcessOption option3;
  option2.sampleRate_in = 32000;
  option2.sampleRate_out = 48000;
  auto resampler_in= createReSamplerProcess(option);
  auto resampler_out = createReSamplerProcess(option3);
  auto ans = createANSProcess(option2);
  
  int index = 0;
  int gcount_size=0;
  size_t res = 0;
  do {
    in_stream.read((char*)in_buffer, in_size);

    gcount_size = in_stream.gcount();

    if (gcount_size < 960) { memset(in_buffer + gcount_size, 0, 960 - gcount_size); }

    res = resampler_in->process(in_buffer, in_size, resample_buffer, ans_size);
    //std::cout << "process index: " << index++ << " insize: " << in_size << " outsize:" << ans_size << " res: " << res
     //         << std::endl;

    res = ans->process(resample_buffer, ans_size, ans_buffer, ans_size);

    res = resampler_out->process(out_buffer, ans_size, out_buffer, in_size);

    //std::cout << "process index: " << index++ << " insize: " << ans_size << " outsize:" << in_size << " res: " << res
     //         << std::endl;

    out_stream2.write((char*)out_buffer, gcount_size);
  } while (in_stream && !in_stream.eof());

  if (in_buffer) free(in_buffer);
  if (out_buffer) free(out_buffer);
  if (ans_buffer) free(ans_buffer);
}
int main(int argc, char* argv[]) {
  resampler32NSX("/home/fengmao/cowa/webRTCtest/build/capture.pcm",
                 "/home/fengmao/cowa/webRTCtest/build/capture_32k.pcm",
                 "/home/fengmao/cowa/webRTCtest/build/capture_32k_NS.pcm", 32000, 2);

  printf("�������棬�������...\n");
  return 0;
}
