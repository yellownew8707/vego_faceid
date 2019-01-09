#ifndef CAMERA_I_H
#define CAMERA_I_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <utils/Log.h>
#include "videodev.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <vdecoder.h>
#include "memoryAdapter.h"

//#define EN_WATERMARK 0
#define NB_BUFFER 8
#define ALIGN_16B(x) (((x) + (15)) & ~(15))
#define ALIGN_4K(x) (((x) + (4095)) & ~(4095))
#define OK 0

typedef struct v4l2_mem_map_t {
	void *mem[NB_BUFFER];
	void *bufVirAddr[NB_BUFFER]; // for usb camera
	void *thumbAddrPhyY[NB_BUFFER]; //for double stream
	void *thumbAddrVirY[NB_BUFFER]; //for double stream
	int  thumbIsAvailable[NB_BUFFER];//for double stream
	int thumbWidth;//for double stream
	int thumbHeight;//for double stream
	int length;
} v4l2_mem_map_t;

typedef struct V4L2BUF_t
{
	unsigned int	addrPhyY;		// physical Y address of this frame
	unsigned int	addrPhyC;		// physical C address of this frame
	
	unsigned int	addrVirY;		// virtual Y address of this frame
	unsigned int	addrVirC;		// virtual C address of this frame

	unsigned int	width;
	unsigned int	height;
	int 			index;			// DQUE id number
	long long		timeStamp;		// time stamp of this frame
	int			isInvalide;
	int				freeInNeed;
}V4L2BUF_t;

typedef struct vstream {
	int handle;
	int format;
	int fps;
	int bufferCnt;
	int width;
	int height;

	pthread_mutex_t vMutex;
	char path[256];

	VideoDecoder *mjpegVideo;
	v4l2_mem_map_t mMapMem;

#ifdef EN_WATERMARK
	WaterMark *mWaterMark;
#endif

}VStream;

int __StartCam(VStream *stream);
int __StopCam(VStream *stream);
int __StartStream(VStream *stream);
void __ReleaseOneFrame(VStream *stream, int index);
V4L2BUF_t __GetOneFrame(VStream *stream);
int __setLED(VStream *stream, int value);

#endif // CAMERA_I_H