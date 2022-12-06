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

void resampler32NSX(char* szFileIn, char* szFileOut, int nSample, int nMode) {
  std::ifstream in_stream;
  in_stream.open(szFileIn, std::ios::in | std::ios::binary);
  std::ofstream out_stream;
  out_stream.open(szFileOut, std::ios::out | std::ios::binary);

  int in_size = 640;
  unsigned char* in_buffer = (unsigned char*)malloc(in_size);

  unsigned int ans_size = in_size;
  unsigned char* ans_buffer = (unsigned char*)malloc(ans_size);
 
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
  auto ans = createANSProcess(option2);
  
  int index = 0;
  int gcount_size=0;
  size_t res = 0;
  do {
    in_stream.read((char*)in_buffer, in_size);
    unsigned char *input_ptr=in_buffer;
    unsigned char *output_ptr=ans_buffer;
    gcount_size = in_stream.gcount();

    if (gcount_size < 640) { memset(input_ptr + gcount_size, 0, in_size - gcount_size); }

    std::cout << "process index: " << index++ << " insize: " << in_size << " outsize:" << ans_size << " res: " << res
              << std::endl;

    res = ans->process(input_ptr, in_size, output_ptr, in_size);

    out_stream.write((char*)output_ptr, gcount_size);
  } while (in_stream && !in_stream.eof());

  if (in_buffer) free(in_buffer);
  if (ans_buffer) free(ans_buffer);
}
int main(int argc, char* argv[]) {
  resampler32NSX("/home/fengmao/cowa/webRTCtest/capture.pcm",
                 "/home/fengmao/cowa/webRTCtest/build/capture_NS_no_sample_640.pcm", 32000, 2);

  printf("�������棬�������...\n");
  return 0;
}
