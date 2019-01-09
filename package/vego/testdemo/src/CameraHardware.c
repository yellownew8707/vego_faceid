
#include "CameraHardware.h"
#include "water_mark.h"

//#define SAVE_YUV_DATA_PREV
#ifdef SAVE_YUV_DATA_PREV
char *pFile_PREV = NULL;
char prev_path[128] = { 0 };
char prev_buff[1088*1080*3/2] = "\0";
#endif

WaterMark* mWaterMark = NULL;

void __ReleaseOneFrame(VStream *stream, int index) {

	int ret = -1;
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = index;
	ret = ioctl(stream->handle, VIDIOC_QBUF, &buf);
	if (ret != 0) {
		printf("VIDIOC_QBUF failed");
	}
}
int __setExposure(VStream *stream, int exp) {
	int ret = -1;
	struct v4l2_control ctrl;
	int mV4l2Handle = stream->handle;
    memset(&ctrl, 0, sizeof(struct v4l2_control));
	ctrl.id = V4L2_CID_AUTO_EXPOSURE_BIAS;
	ctrl.value = exp;
	ret = ioctl(mV4l2Handle, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		printf("setExposure failed!\n");
	else
		printf("setExposure ok\n");
	return ret;
}

int __setLED(VStream *stream, int onoff) {
	int ret = -1;
	struct v4l2_control ctrl;
	int mV4l2Handle = stream->handle;
    memset(&ctrl, 0, sizeof(struct v4l2_control));
	ctrl.id = V4L2_CID_FLASH_LED_MODE;
	ctrl.value = onoff;
	ret = ioctl(mV4l2Handle, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		printf("__setLED failed:%d!\n", ret);
	else
		printf("__setLED ok\n");
	return ret;
}

static int v4l2Init(VStream *stream) {

    int index = 0;
	int ret = -1;
	struct v4l2_capability cap;
	struct v4l2_streamparm params;
	struct v4l2_format format;
	struct v4l2_requestbuffers rb;
	struct v4l2_buffer buf;
	int mVideoFormat = stream->format;
	int mVideoWidth = stream->width;
	int mVideoHeight = stream->height;
	int mV4l2Handle = stream->handle;
	unsigned int mBufferCnt = stream->bufferCnt;
	
	// 查询兼容的模式
	printf("handle, %d\n", mV4l2Handle);
	ret = ioctl (mV4l2Handle, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		printf("Error opening device: unable to query device.\n");
		return -1;
	}

	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		printf("Error opening device: video capture not supported.\n");
		return -1;
	}

	if ((cap.capabilities & V4L2_CAP_STREAMING) == 0) {
		printf("Capture device does not support streaming i/o\n");
		return -1;
	}

    if (-1 == ioctl(mV4l2Handle, VIDIOC_S_INPUT, &index)) {
        printf("VIDIOC_S_INPUT error errno: %d!\n", errno);
        return -1;
    }

	// 设置流模式
	params.parm.capture.timeperframe.numerator = 1;
	params.parm.capture.timeperframe.denominator = stream->fps;
    params.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	params.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	params.parm.capture.capturemode = V4L2_MODE_VIDEO;

    // set v4l2 device CaptureParams
	//CHECK_ERROR(v4l2setCaptureParams(&params));

	ret = ioctl(mV4l2Handle, VIDIOC_S_PARM, &params);
	if (ret < 0) {
		printf("v4l2setCaptureParams failed\n");
		return -1;
	} else {
		printf("v4l2setCaptureParams ok\n");
	}
	ret = ioctl(mV4l2Handle, VIDIOC_G_PARM, &params);

	printf("=====joey===== VIDIOC_G_PARM fps is %d\n",params.parm.capture.timeperframe.denominator);
	/*
	// set v4l2 device parameters
	//CHECK_ERROR(v4l2SetVideoParams());
	//获取支持的格式
	//ret = tryFmtSize(&mVideoWidth, &mVideoHeight);

	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = mVideoWidth;
	fmt.fmt.pix.height = mVideoHeight;
	fmt.fmt.pix.pixelformat = stream->format;//V4L2_PIX_FMT_NV12;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	ret = ioctl(mV4l2Handle, VIDIOC_TRY_FMT, &fmt);
	if (ret < 0) {
		printf("VIDIOC_TRY_FMT Failed: %s", strerror(errno));
		return ret;
	}

	// driver surpport this size
	//mVideoWidth = fmt.fmt.pix.width;
	//mVideoHeight = fmt.fmt.pix.height;
	
	*/
	memset(&format, 0, sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = mVideoWidth;
	format.fmt.pix.height = mVideoHeight;
	format.fmt.pix.pixelformat = mVideoFormat;
	format.fmt.pix.field = V4L2_FIELD_NONE;

	ret = ioctl(mV4l2Handle, VIDIOC_S_FMT, &format);
	if (ret < 0) {
		printf("VIDIOC_S_FMT Failed: %s", strerror(errno));
		return ret;
	}
	
	//__setExposure(stream, 3);
	printf("VIDIOC_S_FMT:  width = %d,  height = %d \n", format.fmt.pix.width, format.fmt.pix.height);
	// v4l2 request buffers
	//CHECK_ERROR(v4l2ReqBufs());

	//申请buffer
	//LOGD("Device[%d] TO VIDIOC_REQBUFS count: %d", mCameraId, mBufferCnt);
	memset(&rb, 0, sizeof(rb));
	rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rb.memory = V4L2_MEMORY_MMAP;
	rb.count = mBufferCnt;

	ret = ioctl(mV4l2Handle, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		printf("Init: VIDIOC_REQBUFS failed: %s\n", strerror(errno));
		return ret;
	}

	if (mBufferCnt != rb.count) {
		mBufferCnt = rb.count;
		printf("VIDIOC_REQBUFS count: %d\n", mBufferCnt);
	}

	// v4l2 query buffers
	//CHECK_ERROR(v4l2QueryBuf());
	//对申请到的buffer 做一次释放动作
	for (unsigned int i = 0; i < mBufferCnt; i++) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		ret = ioctl (mV4l2Handle, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			printf("Unable to query buffer (%s)\n", strerror(errno));
			return ret;
		}
		stream->mMapMem.mem[i] = mmap(0, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, mV4l2Handle, buf.m.offset);
		stream->mMapMem.length = buf.length;
		//LOGI("mV4l2Handle :%d ,index: %d, mem: %x, len: %x, offset: %x", mV4l2Handle, i, (int )mMapMem.mem[i], buf.length, buf.m.offset);
		if (stream->mMapMem.mem[i] == MAP_FAILED) {
			printf("Unable to map buffer (%s)\n", strerror(errno));
			return -1;
		}
		// start with all buffers in queue
		ret = ioctl(mV4l2Handle, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			printf("VIDIOC_QBUF Failed\n");
			return ret;
		}
	}

#ifdef EN_WATERMARK
	if (mV4l2Handle > 0) {
		mWaterMark = (WaterMark *) malloc(sizeof(WaterMark));
		memset(mWaterMark, 0x0, sizeof(WaterMark));
		mWaterMark->bgInfo.width = mVideoWidth;
		mWaterMark->bgInfo.height = mVideoHeight;
        if (mVideoWidth == 1280) {
            mWaterMark->srcPathPrefix = "/tmp/watermark/water_mark_720_";
        } else if (mVideoWidth == 1920){
            mWaterMark->srcPathPrefix = "/tmp/watermark/water_mark_1080_";
        } else {
            mWaterMark->srcPathPrefix = "/tmp/watermark/water_mark_480_";
        }
		mWaterMark->srcNum = 13;
		waterMarkInit(mWaterMark);
	} else {
		mWaterMark = NULL;
	}
#endif
	printf("start camera ok\n");
	return OK;

}

int v4l2DeInit(VStream *stream) {
	int ret = 0;
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(stream->handle, VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		printf("StopStreaming: Unable to stop capture: mV4l2Handle:%d  %s", stream->handle, strerror(errno));
		return ret;
	}

    for (int i = 0; i < stream->bufferCnt; i++) {
		ret = munmap(stream->mMapMem.mem[i], stream->mMapMem.length);
		if (ret < 0) {
			printf("v4l2CloseBuf Unmap failed ,mem[%d] ,mem:0x%p,length:%d", i, stream->mMapMem.mem[i], stream->mMapMem.length);

		}
       stream->mMapMem.mem[i] = NULL;
	}

	
#ifdef	EN_WATERMARK
	if (mWaterMark != NULL) {
		waterMarkExit(mWaterMark);
		free(mWaterMark);
		mWaterMark = NULL;
	}
#endif
	return ret;
}

static void YUYVToNV21(const void* yuyv, void *nv21, int width, int height)
{
	uint8_t* Y	= (uint8_t*)nv21;
	uint8_t* VU = (uint8_t*)Y + width * height;

	for(int i = 0; i < height; i += 2)
	{
		for (int j = 0; j < width; j++)
		{
			*(uint8_t*)((uint8_t*)Y + i * width + j) = *(uint8_t*)((uint8_t*)yuyv + i * width * 2 + j * 2);
			*(uint8_t*)((uint8_t*)Y + (i + 1) * width + j) = *(uint8_t*)((uint8_t*)yuyv + (i + 1) * width * 2 + j * 2);

			if (j % 2)
			{
				if (j < width - 1)
				{
					*(uint8_t*)((uint8_t*)VU + ((i * width) >> 1) + j) = *(uint8_t*)((uint8_t*)yuyv + i * width * 2 + (j + 1) * 2 + 1);
				}
			}
			else
			{
				if (j > 1)
				{
					*(uint8_t*)((uint8_t*)VU + ((i * width) >> 1) + j) = *(uint8_t*)((uint8_t*)yuyv + i * width * 2 + (j - 1) * 2 + 1);
				}
			}
		}
	}
}
static int i_hx = 0;

int Decoder_Thread(VideoDecoder *pVideo, void *p, int64_t currenttimeus, unsigned int *addrVirY, unsigned int *addrVirC, VStream *stream)
{
	int ret;
	char prev_path[128] = { 0 };
	FILE *pFile_PREV = NULL;
	char *Data = NULL;
	int dataLen =0;
	struct v4l2_buffer *param = (struct v4l2_buffer *)p;
	VideoPicture *videoPicture = NULL;
	//get data to decoder
	dataLen = param->bytesused;
	Data = stream->mMapMem.mem[param->index];
	//Copy Video Stream Data to buf and ringbuf
	char *buf, *ringBuf;
	int buflen, ringBufLen;
	if(RequestVideoStreamBuffer(pVideo, dataLen, (char**)&buf, &buflen, (char**)&ringBuf, &ringBufLen, 0)) {
		printf("#####Error: RequestVideoStreamBuffer failed\n");
		return -1;
	}
	//printf("Request Video Stream Buffer ok\n");

	if(buflen + ringBufLen < dataLen) {
		printf("#####Error: request buffer failed, buffer is not enough\n");
		return -1;
	}

	if(buflen >= dataLen) {
		memcpy(buf,Data,dataLen);
	} else {
		memcpy(buf,Data,buflen);
		memcpy(ringBuf,Data+buflen,dataLen-buflen);
	}
	//Submit Video Stream Data
	VideoStreamDataInfo DataInfo;
	memset(&DataInfo, 0, sizeof(DataInfo));
	DataInfo.pData = buf;
	DataInfo.nLength = dataLen;
	DataInfo.bIsFirstPart = 1;
	DataInfo.bIsLastPart = 1;
	if (SubmitVideoStreamData(pVideo, &DataInfo, 0)) {
		printf("#####Error: Submit Video Stream Data failed!\n");
		return -1;
	}
	// step : decode stream now
	int endofstream = 0;
	int dropBFrameifdelay = 0;
	int decodekeyframeonly = 0;

	ret = DecodeVideoStream(pVideo, endofstream, decodekeyframeonly,dropBFrameifdelay, currenttimeus);
	//printf("decoder ret is %d\n",ret);
	//printf("decode ret = %d",ret);

	switch (ret)
	{
		case VDECODE_RESULT_KEYFRAME_DECODED:
		case VDECODE_RESULT_FRAME_DECODED:
		case VDECODE_RESULT_NO_FRAME_BUFFER:
		{
			ret = ValidPictureNum(pVideo, 0);
			//printf("ValidPictureNum ret is %d\n",ret);
			if (ret >= 0) {
				videoPicture = RequestPicture(pVideo, 0);
				//printf("RequestPicture FINISH !!\n");
				if (videoPicture == NULL) {
					printf("**********************************************************************decoder fail\n");


					//fail_decoder = 1;
					return -1;
				}
				//printf("decoder one pic...\n");
				//fail_decoder = 0;
				//printf("pic nWidth is %d,nHeight is %d\n",videoPicture->nWidth,videoPicture->nHeight);
				//get decoder data
				if(videoPicture->pData0 != NULL) {
					*addrVirY = videoPicture->pData0;
				}
				if(videoPicture->pData1 != NULL) {
					*addrVirC = videoPicture->pData1;
				}
				/*
				static int count = 0;
				if(count < 5){
					FILE *pFile_YUV = NULL;
					char path[128] = { 0 };

					snprintf(path, sizeof(path),"/mnt/UDISK/222test%d.yuv",count);
					pFile_YUV = fopen(path, "wb");
					if (pFile_YUV == NULL)
						{
							printf("fopen %s fail !", path);
						}
					char *tmp_buf0 = (char*)malloc(640* 480 *3/2);
					printf("videoPicture->pData0: %p ��videoPicture->pData1 %p ,videoPicture->ePixelFormat = %d",
						videoPicture->pData0,videoPicture->pData1,videoPicture->ePixelFormat);




					memcpy(tmp_buf0, (char *)videoPicture->pData0, videoPicture->nWidth* videoPicture->nHeight);
					memcpy(tmp_buf0 + videoPicture->nWidth* videoPicture->nHeight, (char *)videoPicture->pData1, videoPicture->nWidth* videoPicture->nHeight/2);
					//memcpy(tmp_buf0 + videoPicture->nWidth* videoPicture->nHeight*5/4, (char *)videoPicture->pData2, videoPicture->nWidth* videoPicture->nHeight/4);
					fwrite(tmp_buf0, videoPicture->nWidth* videoPicture->nHeight *3/2 , 1, pFile_YUV);

					free(tmp_buf0);
					fflush(pFile_YUV);
					fclose(pFile_YUV);


					count++;


				}
				*/
				//printf("Decoder FINISH !!\n");
				ReturnPicture(pVideo, videoPicture);
//#if SAVE_DECODER_FRAME
#if 0
	char image_name2[30];
	sprintf(image_name2,"%s/%s%d%s",PATH,"IMAGE",np+1,".yuv");
	save_frame_to_file(image_name2,videoPicture->pData0,videoPicture->nWidth*videoPicture->nHeight);
	save_frame_to_file(image_name2,videoPicture->pData1,videoPicture->nWidth*videoPicture->nHeight/2);
#endif
				//CdcMemClose(memops);
				return NULL;
			} else {
				printf("no ValidPictureNum ret is %d",ret);
			}
			break;
		}
		case VDECODE_RESULT_OK:
		case VDECODE_RESULT_CONTINUE:
		case VDECODE_RESULT_NO_BITSTREAM:
		case VDECODE_RESULT_RESOLUTION_CHANGE:
		case VDECODE_RESULT_UNSUPPORTED:
		default:
			printf("video decode Error: %d!\n", ret);
		return -1;
	}
	//free memops
	//CdcMemClose(memops);
	return 0;
}
char *p = NULL;
V4L2BUF_t __GetOneFrame(VStream *stream) {
	int ret = 0;
	//Mutex::Autolock autoLock(mLockGetFrame);
	void *addrVirY = NULL;
    struct timespec t;
	struct v4l2_buffer buf;
	fd_set fds;
	struct timeval tv;
	int r;
	int64_t mCurFrameTimestamp = 0;
	int mV4l2Handle = stream->handle;
	int mVideoWidth =stream->width;
	int mVideoHeight = stream->height;
	V4L2BUF_t v4l2_buf = {0};
	FD_ZERO(&fds);
	FD_SET(mV4l2Handle, &fds);

#ifdef SAVE_YUV_DATA_PREV
	static int size_prev_y = 0;
	static int size_prev_c = 0;
	static unsigned char* addrPrevVirY = NULL;
	static unsigned char* addrPrevVirC = NULL;
#endif

	/* Timeout */
	memset(&v4l2_buf, 0, sizeof(V4L2BUF_t));

	if(V4L2_PIX_FMT_MJPEG == stream->format || stream->format == V4L2_PIX_FMT_YUYV){
		tv.tv_sec = 3;
	}else{
		tv.tv_sec = 1;
	}
	//tv.tv_sec = 1;
	tv.tv_usec = 0;

	r = select(mV4l2Handle + 1, &fds, NULL, NULL, &tv);
	if (r == -1) {
		printf("select err, %s", strerror(errno));
		ret =  -1;
	} else if (r == 0) {
		printf("v412 waitcamerReady select timeout\n");
		ret =  -1;
	}

	if (ret != 0) {
		printf("#########mCameraId:%s wait time out############\n", stream->path);
        //resetCamera();
		//GlobalEventCallback(RECORDER_TRACK_EVENT_INFO, MSG_TYPE_CAMERA_WAIT_TIME_OUT, mCameraId, NULL);
		//exit(1);
		v4l2_buf.isInvalide = -2;
		return v4l2_buf;
	}
/*
	if(!mIsUsbCamera && uvcPowerSupply == false){
		//LOGE("V4L2Camera::closeUvcPower & openUvcPower");
		closeUvcPower();
		usleep(10*1000);			
		openUvcPower();
		uvcPowerSupply = true;
	}
*/
	memset(&buf, 0, sizeof(struct v4l2_buffer));
	//ret = getPreviewFrame(&buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(mV4l2Handle, VIDIOC_DQBUF, &buf);
	if (ret < 0) {
		/* notify Previewthread release PreviewFrame */
		//GlobalEventCallback(RECORDER_TRACK_EVENT_INFO, MSG_TYPE_CAMERA_QBUF_FAIL, mCameraId, NULL);
		//mListener->postDataTimestamp(mCameraId, 0, NULL, NULL, false);
        //wait app to reset
		printf("VIDIOC_DQBUF failed-------------------\n");
		usleep(3*1000);
		return v4l2_buf;
	}

    clock_gettime(CLOCK_MONOTONIC, &t);
    mCurFrameTimestamp = (int64_t)((int64_t) t.tv_nsec/1000 + (((int64_t) t.tv_sec) * 1000000));//微秒


	v4l2_buf.width = mVideoWidth;
	v4l2_buf.height = mVideoHeight;
	v4l2_buf.index = buf.index;
	v4l2_buf.timeStamp = mCurFrameTimestamp;

	if (stream->format == V4L2_PIX_FMT_NV12 || stream->format == V4L2_PIX_FMT_NV21 || stream->format == V4L2_PIX_FMT_YUYV) {//for csi camera
 		addrVirY = stream->mMapMem.mem[buf.index];

 		v4l2_buf.addrPhyY = buf.m.offset;
    	v4l2_buf.addrVirY = (unsigned int) addrVirY;
    	v4l2_buf.addrPhyC = buf.m.offset + mVideoWidth * mVideoHeight;
    	v4l2_buf.addrVirC = (unsigned int) addrVirY + mVideoWidth * mVideoHeight;
	}else if (stream->format == V4L2_PIX_FMT_MJPEG) {//for uvc camera
		if (stream->mjpegVideo != NULL && stream->mMapMem.mem[buf.index] != NULL) {
			ret = Decoder_Thread(stream->mjpegVideo, &buf, mCurFrameTimestamp, &v4l2_buf.addrVirY, &v4l2_buf.addrVirC, stream);
			if(0 != ret) {
				printf("Decoder_Thread error\n");
			}
		} else {
			printf("stream->mjpegVideo = NULL or stream->mMapMem = NULL; stream->mjpegVideo:%p, stream->mMapMem.mem[%d]:%p\n",\
				stream->mjpegVideo, buf.index, stream->mMapMem.mem[buf.index]);
		}
		//printf("getonfrram cam: 1, index: %d, v4l2_buf.addrVirY: 0x%x, stream->mMapMem.mem[buf.index]: 0x%x\n",buf.index, v4l2_buf.addrVirY, stream->mMapMem.mem[buf.index]);
	}

	if (stream->format == V4L2_PIX_FMT_YUYV){
		if (p == NULL){
			p = malloc(mVideoWidth*mVideoHeight*2);
		}
		if (p) {
			memcpy(p, addrVirY, mVideoWidth*mVideoHeight*2);
			YUYVToNV21(p, v4l2_buf.addrVirY, mVideoWidth, mVideoHeight);
			//free(p);
		}
	}
#ifdef SAVE_YUV_DATA_PREV
	memcpy(prev_buff, addrVirY, 1088*1080*3/2);
#endif

#ifdef	EN_WATERMARK
	//if (NULL != addrVirY && true == mBackCarStatus && mCameraId == 1) {
	//	CameraHal_LRFlipFrame(V4L2_PIX_FMT_NV12, mVideoWidth, mVideoHeight, (unsigned char *)v4l2_buf.addrVirY, (unsigned //char*)v4l2_buf.addrVirC);
	//}

    if (addrVirY != NULL && mWaterMark != NULL) {
        mWaterMark->bgInfo.y = (unsigned char*)v4l2_buf.addrVirY;
        mWaterMark->bgInfo.c = (unsigned char*)v4l2_buf.addrVirC;

        if (mVideoWidth == 1920) {
            waterMarkShowTime(mWaterMark, 48, 84, (int)mCurFrameTimestamp);
        } else {
            waterMarkShowTime(mWaterMark, 16, 30, (int)mCurFrameTimestamp);
        }
    }
#endif
   // MemAdapterFlushCache((void *)v4l2_buf.addrVirY, mVideoWidth*mVideoHeight);
   // MemAdapterFlushCache((void *)v4l2_buf.addrVirC, mVideoWidth*mVideoHeight/2);
	//printf("%s,l:%d,v4l2_buf.addrVirY:%p, v4l2_buf.addrVirC:%p\n", __FUNCTION__, __LINE__, v4l2_buf.addrVirY, v4l2_buf.addrVirC);
	if (v4l2_buf.addrVirY != 0 && v4l2_buf.addrVirC != 0) {
		//printf("__GetOneFrame success\n");
		v4l2_buf.isInvalide = 1;
//		MemAdapterFlushCache((void *)v4l2_buf.addrVirY, mVideoWidth*mVideoHeight);
//		MemAdapterFlushCache((void *)v4l2_buf.addrVirC, mVideoWidth*mVideoHeight/2);

#ifdef SAVE_YUV_DATA_PREV
	size_prev_y = 1088*1080;
	size_prev_c = 1088*1080/4;
	addrPrevVirY = (unsigned char*)prev_buff;
	addrPrevVirC = (unsigned char*)prev_buff + size_prev_y;
	if (pFile_PREV){
		fwrite(addrPrevVirY, size_prev_y, 1, (FILE*)pFile_PREV);
		fwrite(addrPrevVirC, size_prev_c * 2, 1, (FILE*)pFile_PREV);
	}
	//gettimeofday( &tv_end, NULL );
	printf("SAVE_YUV_DATA_PREV OK \n");
#endif
		
	}

	return v4l2_buf;
}


int __StartCam(VStream *stream){

	int ret = 0;
	int handle = 0;
i_hx = 0;
	if (stream->handle) {
		printf("stream alreay open, please check\n");
		return OK;
	}
	// snprintf(dev, sizeof(dev),"%s%d", "/dev/video", id);

    if (access(stream->path,F_OK) == 0) {
        handle = open(stream->path, O_RDWR | O_NONBLOCK, 0);
        if (handle <= 0) {
            printf("stream open failed: %d, please check\n", handle);
            return handle;
        }
		printf("v4l2Init 33333 access\n");
		stream->handle = handle;
    }

	ret = v4l2Init(stream);
	if (ret < 0) {
		printf("v4l2Init failed: %d, please check\n", ret);
		close(handle);
		return -1;
	}

#ifdef SAVE_YUV_DATA_PREV
	snprintf(prev_path, sizeof(prev_buff),"/mnt/SDCARD/prev.yuv");

	if(pFile_PREV == NULL){
		pFile_PREV  = (char *)fopen(prev_path, "wb");
	}else{
		printf("File exists !!!\n");
	}

	if(pFile_PREV == NULL){
		printf("fopen %s fail !", prev_path);
	}
#endif

	return OK;
}


int __StartStream(VStream *stream) {
	int mV4l2Handle = stream->handle;
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret = 0;
	// stream on the v4l2 device
	//CHECK_ERROR(v4l2StartStreaming());
	// 开始获取流

	ret = ioctl (mV4l2Handle, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		printf("StartStreaming: Unable to start capture: %s", strerror(errno));
		return ret;
	}
	printf("StartStreaming ok-----------------\n");

	return ret;
}

int __StopCam(VStream *stream) {
	if (!stream) {
		printf("stream is null----------\n");
		return OK;
	}
	if (!stream->handle) {
		printf("CameraHardware::stopTVDecoder: state already stopped\n");
		return OK;
	}
	
	v4l2DeInit(stream);

	close(stream->handle);
	stream->handle = 0;
	printf("now close came sueccess----------------\n");
	return OK;
}

