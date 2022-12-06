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

// void resampler32NSX(char* szFileIn, char* szFileOut, int nSample, int nMode) {
//   std::ifstream in_stream;
//   in_stream.open(szFileIn, std::ios::in | std::ios::binary);
//   std::ofstream out_stream;
//   out_stream.open(szFileOut, std::ios::out | std::ios::binary);

//   int in_size = 1024*4;
//   unsigned char* in_buffer = (unsigned char*)malloc(in_size);

//   unsigned int ans_size = in_size;
//   unsigned char* ans_buffer = (unsigned char*)malloc(ans_size);
 
//   ProcessOption option;
//   // wav or pcm
//   in_stream.read((char*)in_buffer, 44);
//   if (in_buffer[0] == 'R' && in_buffer[1] == 'I' && in_buffer[2] == 'F' && in_buffer[3] == 'F') {
//     // wav
//     in_stream.seekg(44, std::ios_base::beg);
//   } else {
//     in_stream.seekg(0, std::ios_base::beg);
//   }

//   int filter_state1[6], filter_state12[6];
//   int Synthesis_state1[6], Synthesis_state12[6];

//   ProcessOption option2;
//   option2.sampleRate_in = 32000;
//   auto ans = createANSProcess(option2);

//   int index = 0;
//   size_t res = 0;
//   do {
//     in_stream.read((char*)in_buffer, in_size);
//     //int16_t *input_ptr=(int16_t *)in_buffer; 会导致参数错误，指针问题导致process直接进不去
//     //int16_t *output_ptr=(int16_t *)ans_buffer;
//     unsigned char *input_ptr=in_buffer;
//     unsigned char *output_ptr=ans_buffer;
//     size_t frame_size=640;
//     size_t gcount_size = in_stream.gcount();
//     size_t count_in=gcount_size/frame_size;
//     size_t remained_size=gcount_size-count_in*frame_size;

//     for (int i = 0; i < count_in;i++)
//     {
//    std::cout <<"start\n";
//       res = ans->process(input_ptr, frame_size, output_ptr, frame_size);

//       std::cout << "process index: " << index++ << " insize: " << in_size << " outsize:" << ans_size << " res: " << res
//                 << std::endl;
//         out_stream.write((char*)output_ptr, frame_size);
//         input_ptr+=frame_size;
//         output_ptr+=frame_size;
//     }
//     if (remained_size != 0) {
//       //break;
//       int16_t samplePatchIn[frame_size] = {0};
//       int16_t samplePatchOut[frame_size] = {0};
//       memcpy(samplePatchIn, input_ptr, remained_size);
//       res = ans->process(samplePatchIn, frame_size, samplePatchOut, frame_size);
//       std::cout << "process index: " << index++ << " insize: " << in_size << " outsize:" << ans_size << " res: " << res
//                 << std::endl;
//             memcpy(output_ptr, samplePatchOut, remained_size);  
//       out_stream.write((char*)output_ptr, remained_size);
//     }

//   } while (in_stream && !in_stream.eof());

//   if (in_buffer) free(in_buffer);
//   if (ans_buffer) free(ans_buffer);
// }

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

int main(int argc, char* argv[]) {
//  resampler32NSX("/home/fengmao/cowa/webRTCtest/build/capture.pcm",
//                 "/home/fengmao/cowa/webRTCtest/build/capture_NS_no_sample.pcm", 32000, 2);
 NoiseSuppressionX32("/home/fengmao/cowa/webRTCtest/capture.pcm","/home/fengmao/cowa/webRTCtest/build/capture_NS_no_sample0.pcm", 32000, 2);
  printf("�������棬�������...\n");
  return 0;
}
