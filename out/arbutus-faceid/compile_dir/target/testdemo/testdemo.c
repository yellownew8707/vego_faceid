#include <utils/Timers.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <sys/dir.h>
#include <getopt.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdbool.h>
#include <rtsp_server_live.h>
#include <tv_sdk_api.h>
#include "CameraHardware.h"
#include "water_mark.h"
#include "debug.h"

static int capturing = 1;
static int Vclose = 0;
static int Isopen = 0;
void stream_close(int sig)
{
    signal(sig, SIG_IGN);
    Vclose = 1;
    printf("==========stream_close==============\n");
}

int iio_fp = -1;
char *data = NULL;

static int _init_decoder(VideoDecoder **pVideo)
{
	VConfig vConfig;
	//init the vConfig
	struct ScMemOpsS* memops = MemAdapterGetOpsS();
	if(memops == NULL)
	{
		return -1;
	}
	CdcMemOpen(memops);
	AddVDPlugin();

	db_error("pVideo is %p  111\n",*pVideo);

	memset(&vConfig, 0x00, sizeof(VConfig));
	vConfig.bDisable3D = 0;
	vConfig.bDispErrorFrame = 0;
	vConfig.bNoBFrames = 0;
	vConfig.bRotationEn = 0;
//	vConfig.nRotateDegree = 3;
	vConfig.nAlignStride = 16;
	vConfig.bScaleDownEn = 0;
	vConfig.nHorizonScaleDownRatio = 0;
	vConfig.nVerticalScaleDownRatio = 0;
	vConfig.eOutputPixelFormat =PIXEL_FORMAT_NV21;/*decoder to */
	//vConfig.eOutputPixelFormat =PIXEL_FORMAT_YUV_PLANER_420;
	//vConfig.eOutputPixelFormat =PIXEL_FORMAT_YV12;
	vConfig.nDeInterlaceHoldingFrameBufferNum = 0;
	vConfig.nDisplayHoldingFrameBufferNum = 0;
	vConfig.nRotateHoldingFrameBufferNum = 3;
	vConfig.nDecodeSmoothFrameBufferNum = 1;
	vConfig.nVbvBufferSize = 2*1024*1024;
	vConfig.bThumbnailMode = 0;
	vConfig.memops = memops;
	//create videodecoder
	VideoStreamInfo videoInfo;
	memset(&videoInfo, 0x00, sizeof(VideoStreamInfo));
	videoInfo.eCodecFormat = VIDEO_CODEC_FORMAT_MJPEG;/*MJPEG or H264 to decoder*/
	videoInfo.nWidth = 640;
	videoInfo.nHeight = 360;
	*pVideo = CreateVideoDecoder();
	if(!*pVideo)
	{
		db_error("create video decoder failed\n");
		return -1;
	}
	db_error("pVideo is %p  222\n",*pVideo);
	db_error("create video decoder ok\n");

	if ((InitializeVideoDecoder(*pVideo, &videoInfo, &vConfig)) != 0)
	{
		db_error("open dev failed,  decode error !\n");
		DestroyVideoDecoder(*pVideo);
		return -1;
	}
	db_error("Initialize video decoder ok\n");
	db_error("pVideo is %p  333\n",*pVideo);

	return 0;
}

void sigint_handler(int sig)
{
    capturing = 0;
    Vclose = 1;
    printf("========sigint_handle===========\n");
}

void face_detect_callback(int event) {
	int i = 0, num = 0;
	//detect faces
	if (event == 1){
		int ret = dev_getFaceNum();
		int *rect = NULL;
		num = ret;
		printf("num==:%d, i:%d\n", ret, i);
		for(i = 0; i < num; i++){
			printf("%s,l:%d\n", __FUNCTION__, __LINE__);
			rect = dev_getFaceRect(i);
			printf("rect(%d,%d,%d,%d) \n", rect[0],rect[1],rect[2],rect[3]);
			printf("info: id:%d , gender:%d, age:%d, emotion:%d, attention:%d\n",
					dev_getFaceID(i),
					dev_faceGetGender(i),
					dev_faceGetAge(i),
					dev_faceGetEmotion(i),
					dev_faceGetAttention(i)
				);
			//如果没有注册过，id小于等于0
			if (dev_getFaceID(i) <= 0) {
				dev_enterRegisterMode();
				int id = dev_registerFaceID(i);
				if (id > 0) {
				    //todo map id to name;
				}
				dev_exitRegisterMode();
			}
		}
		
	}
	return 0;
}

void *faceIDDataDet(void *param) {
	int ret = 0;
	printf("faceID test start\n");
	while(1) {
		if (access("/dev/video0", R_OK)==0){
			printf("camera ok\n");
			break;
		} else {
			printf("camera FAIL\n");
			usleep(1000*1000);
		}
	}
	dev_init(face_detect_callback, NULL,0, 0, 0, NULL, 0, 0);
	printf("%s,l:%d\n", __FUNCTION__, __LINE__);
	dev_setCallBackFreq(0, 30);
	printf("version:%s\n", dev_getSysVersion());
	while(1) {
		if (capturing == 0)
			break;
		usleep(30*1000);
	};
	dev_release();
	printf("faceID test end\n");
	
}

//#define SAVEYUVFILE
int testdemo(char *ip, int port)
{
	int ret = 0;
	pthread_t pfaceIDDet;
    VStream *Recorder = NULL;
	int i = 5;
	int j = 0;
#ifdef SAVEYUVFILE
	char prev_path[128] = { 0 };
	FILE *pFile_PREV = NULL;
#endif
	struct timespec t;
	int64_t  mCurFrameTimestamp, now, diff;
	int badframe = 0;
	WaterMark *mWaterMark = NULL;
	/*
	* in order to ensure the testdemo can run normal. 
	* sleep 15s to ensure the system resource is OK. need to do... just for test.
	*/
	usleep(1000*1000*15);
	//system("wifi_connect_ap_test ssid password");
	system("softap_up zulukowifi open broadcast");
	usleep(1000*1000*3);
	
	if (access("/sys/class/net/wlan0", R_OK)==0){
		printf("wlan0 OK\n");
	} else {
		printf("wlan0 FAIL\n");
	}
	ret = pthread_create(&pfaceIDDet, NULL, faceIDDataDet, NULL);
	if (0 != ret){
		printf("creat usbStorageDetect thread falied!!!");
	}
#if 1
	while(!Vclose){

		if (Recorder == NULL) {
			printf("11111111111111111\n");
			Recorder = calloc(1, sizeof(VStream));
			Recorder->handle = 0;
			Recorder->format = V4L2_PIX_FMT_YUYV;
			Recorder->fps = 30;
			Recorder->bufferCnt = 8;
			Recorder->width = 640;
			Recorder->height = 360;

			memcpy(Recorder->path, "/dev/video0", strlen("/dev/video0"));
reopen:
			if (Recorder->format == V4L2_PIX_FMT_MJPEG) {
				printf("Recorder->mjpegVideo is %p  111\n",Recorder->mjpegVideo);
				ret = _init_decoder(&Recorder->mjpegVideo);
				if(0 != ret) {
					printf("init_decoder error!!!!!\n");
					exit(-1);
				} else {
					printf("init_decoder success!!!!!\n");
				}
				printf("Recorder->mjpegVideo is %p  222\n",Recorder->mjpegVideo);
			}

			ret = __StartCam(Recorder);
			if(0 != ret){
				printf("_StartCam failed, May be camera is not ready!!!\n");
			}
	
			ret = __StartStream(Recorder);
			if(0 != ret){
				printf("__StartStream failed!!!\n");
			} else {
				printf("cam ok\n");
			}
#ifdef SAVEYUVFILE
			memcpy(prev_path, "/mnt/UDISK/prev.yuv", strlen("/mnt/UDISK/prev.yuv"));
			if(pFile_PREV == NULL){
				pFile_PREV  = fopen(prev_path, "wb");
			}else{
				printf("File exists !!!\n");
			}
#endif
			init_mediaserver("192.168.5.1", 8554);
			printf("cam init ok: %s, %d\n", ip, port);
		}

		clock_gettime(CLOCK_MONOTONIC, &t);
		mCurFrameTimestamp = (int64_t)((int64_t) t.tv_nsec/1000 + (((int64_t) t.tv_sec) * 1000000));

		V4L2BUF_t buf = __GetOneFrame(Recorder);
//		printf("%s,l:%d,buf.isInvalide:%d\n", __FUNCTION__, __LINE__, buf.isInvalide);
		if (buf.isInvalide == 1){
			clock_gettime(CLOCK_MONOTONIC, &t);
			now = (int64_t)((int64_t) t.tv_nsec/1000 + (((int64_t) t.tv_sec) * 1000000));
			diff = (now - mCurFrameTimestamp)/1000;
//			printf("send yuv, %d, %d\n", buf.width, buf.height);
			sendYUV((unsigned char*)buf.addrVirY, buf.width * buf.height*3/2, 0);
			//printf("send yuv end\n");
			__ReleaseOneFrame(Recorder, buf.index);
#ifdef SAVEYUVFILE
			if (pFile_PREV){
				ret = fwrite((char *)buf.addrVirY, buf.width * buf.height, 1, (FILE*)pFile_PREV); // y
				fwrite(buf.addrVirC, buf.width * buf.height/2, 1, (FILE*)pFile_PREV);// uv
			}
#endif
		} else {
			__ReleaseOneFrame(Recorder, buf.index);
			__StopCam(Recorder);
			free(Recorder);
			printf("cam failed\n");
			
		}
	}
#endif
#ifdef SAVEYUVFILE
	if (pFile_PREV)
		fclose(pFile_PREV);
#endif
destory_mediaserver();
    printf("\n");
    printf("******************************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("******************************************************************************************\n");
    printf("\n");

	return 0;
}

int main(int argc, char** argv)
{
	int ret = 0;

	printf("0000000000000000\n");
	/* install signal handler and begin faceid thread*/
    signal(SIGINT, sigint_handler);

	printf("111111111111111111\n");
	testdemo(NULL, 8554);
	while(1) {
		//printf("%s\n", __FUNCTION__);
		usleep(30*1000);
	 	if (capturing==0)
	 		break;
	};	
	printf("2222222222222222222\n");
    exit(0);
}

//}


