
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

//extern int errno;
struct comm_service_handle {
	int inited;
	int release;
	pthread_t comm_service_thread_tid;
	int comm_service_thread_exit;
	struct usb_comm_protocol_f ucpf;
	struct usb_comm_protocol_f_ext ucpf_ext;
	int call_back_freq ;
	void (*face_detect_callback)(int event);
	void (*body_detect_callback)(int event);
};

struct comm_service_handle comm_handle;

extern int algo_device_request(void *req, int req_size, void *reply, int replay_size);
extern int algo_device_release();
extern int algo_device_init();

unsigned long get_file_size(const char *path)  
{
    unsigned long filesize = -1;      
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
}

int comm_device_isIDReg(int id){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_isIDReg\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_IS_REG;
	ucp_reply.param[1] = 1;
	ucp_reply.param[2] = id;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

int comm_device_sendPhotoFile(char *PhotoPath){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;
	int totalsize = 0;
	int num = 0;
	unsigned long filesize = 0;

	LOG("comm_device_sendPhotoFile:%s\n", PhotoPath);
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));

	if (PhotoPath == NULL){
        LOG("ota path is null\n");
		return -1;
	}
//	if (access(PhotoPath, R_OK) == 0){
	int fd = open(PhotoPath, O_RDONLY);
	if (fd < 0) {
        LOG("open Photofile %s fail ret:%d\n", PhotoPath, fd);
		return -2;
	} else {
	    lseek(fd, 0, SEEK_SET);
		filesize = get_file_size(PhotoPath);
		while(1){
		    ret = read(fd, ucp_reply.buffer, DEVICE_COMM_SERVICE_CHAR_LEN);
			totalsize += ret;
			//LOG("filesize:%d, send size:%d, ret:%d\n", filesize, totalsize, ret);
            ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
			ucp_reply.param[0] = DEVICE_SET_PHOTO_FILE;
			ucp_reply.param[1] = 3;//para num
			ucp_reply.param[2] = totalsize - ret;//file offset
			ucp_reply.param[3] = ret; //buffer size
			if (totalsize == filesize){
				ucp_reply.param[4] = 1;//file end
			} else {
				ucp_reply.param[4] = 0;
			}
retry:
			ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
			if (ret < 0) {
				num++;
				LOG("send photo file fail,usb transfer ret:%d, retry num:%d\n", ret, num);
				if (num > 5){
					close(fd);
					fd = 0;
					return -1;
				}
				goto retry;
			}
			num = 0;
			ret = ucp_reply.param[2];//result
			if (ret < 0){
                LOG("send photo file fail ret:%d\n", ret);
				close(fd);
				return -1;
			}
			if (totalsize == filesize)
                break;
		}
		close(fd);
        LOG("send photo file success\n");
	}

	return 0;
}

int comm_device_sendOtaFile(char *OtaPath){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;
	int totalsize = 0;
	int num = 0;
	unsigned long filesize = 0;
	
	LOG("comm_device_sendOtaFile:%s\n", OtaPath);
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	
	if (OtaPath == NULL){
        LOG("ota path is null\n");
		return -1;
	}
//	if (access(OtaPath, R_OK) == 0){
	int fd = open(OtaPath, O_RDONLY);
	if (fd < 0) {
        LOG("open otafile %s fail ret:%d\n", OtaPath, fd);
		return -2;
	} else {
	    lseek(fd, 0, SEEK_SET);	
		filesize = get_file_size(OtaPath);
		while(1){
		    ret = read(fd, ucp_reply.buffer, DEVICE_COMM_SERVICE_CHAR_LEN);
			totalsize += ret;
			LOG("filesize:%d, send size:%d, ret:%d\n", filesize, totalsize, ret);
            ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
			ucp_reply.param[0] = DEVICE_OTA_FILE;
			ucp_reply.param[1] = 3;//para num
			ucp_reply.param[2] = totalsize - ret;//file offset
			ucp_reply.param[3] = ret; //buffer size
			if (totalsize == filesize){
				ucp_reply.param[4] = 1;//file end
			} else {
				ucp_reply.param[4] = 0;
			}
retry:
			ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
			if (ret < 0) {
				num++;
				LOG("send ota file fail,usb transfer ret:%d, retry num:%d\n", ret, num);
				if (num > 5){
					close(fd);
					fd = 0;
					return -1;
				}
				goto retry;
			}
			num = 0;
			ret = ucp_reply.param[2];//result
			if (ret < 0){
                LOG("send ota file fail ret:%d\n", ret);
				close(fd);
				return -1;
			}
			if (totalsize == filesize)
                break;
		}
		close(fd);
        LOG("send ota file success\n");
	}

	return 0;
}

int comm_device_set_signature(char *sig){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_set_signature\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_SET_SIGNATURE;
	ucp_reply.param[1] = 1;
	if (sig != NULL && strlen(sig) < DEVICE_COMM_SERVICE_CHAR_LEN) {
		memcpy((void *)ucp_reply.buffer, sig, strlen(sig));
	} else {
		return -1;
	}

	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

char *comm_device_get_cpuid(){
	static struct usb_comm_protocol ucp_reply;
	char *cpuid = NULL;
	int ret = 0;
	
	LOG("comm_device_get_cpuid\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_GET_CPUID;
	ucp_reply.param[1] = 0;//para num
	ucp_reply.param[2] = 0;//para 0,1,2
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	//ret = ucp_reply.param[2];//result
	return (char *)ucp_reply.buffer;

}

int comm_device_reboot_device(){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_reboot_device\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_REBOOT_DEVICE;
	ucp_reply.param[1] = 0;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}


int comm_device_reset_facdiddb(){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_reset_facdiddb\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_RESET_ALBUM;
	ucp_reply.param[1] = 0;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

char* comm_device_system_verison() {
	static struct usb_comm_protocol ucp_reply;
	int ret = 0;
	
	LOG("comm_device_system_verison\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_GET_SYSTEM_VERSION;
	ucp_reply.param[1] = 0;//para num
	ucp_reply.param[2] = 0;//para 0,1,2
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	//ret = ucp_reply.param[2];//result
	return ucp_reply.buffer;
}

int comm_device_start_update() {
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_start_update\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_START_UPFATE;
	ucp_reply.param[1] = 0;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	
	return ret;
}

int comm_device_switch_usb_mode(int mode){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_switch_usb_mode\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_SWITCH_USB_MODE;
	ucp_reply.param[1] = 1;
	ucp_reply.param[2] = mode;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	
	return ret;
}

int comm_device_get_algo_status(int algo){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_get_algo_status\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_GET_ALGO_STATUS;
	ucp_reply.param[1] = 1;
	ucp_reply.param[2] = algo;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

int comm_device_switch_algo(int algo, int enable){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_switch_algo\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_SWITCH_ALGO;
	ucp_reply.param[1] = 2;
	ucp_reply.param[2] = algo;
	ucp_reply.param[3] = enable;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

int comm_device_set_callback_freq(int algo, int freq){

	LOG("comm_device_set_callback_freq\n");
	if (freq > 0 && freq < 100)
		comm_handle.call_back_freq = freq;

	return 0;
}

int comm_device_set_detect_fps(int algo, int fps){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_set_detect_fps\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_SET_FACE_DETECT_FPS;
	ucp_reply.param[1] = 2;
	ucp_reply.param[2] = algo;
	ucp_reply.param[3] = fps;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

int comm_device_set_record_mode(int second){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_set_record_mode\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_SET_RECORD_MODE;
	ucp_reply.param[1] = 1;
	ucp_reply.param[2] = second;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;	
}

int comm_device_stop_record(){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_stop_record\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_STOP_RECORD;
	ucp_reply.param[1] = 0;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;	
}

int comm_device_start_record(){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_start_record\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_START_RECORD;
	ucp_reply.param[1] = 0;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;	
}

int comm_device_take_picture(){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_take_picture\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_TAKE_PICTURE;
	ucp_reply.param[1] = 0;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

int comm_device_get_body_status(int index){
	LOG("comm_device_get_body_status\n");
	return 0;	
}

int* comm_device_get_body_rect(int index){
	LOG("comm_device_get_body_rect\n");
	return NULL;
}
	
int comm_device_get_body_num(){
	LOG("comm_device_get_body_num\n");
	return 0;
}

int comm_device_face_get_attention(int index){
	LOG("comm_device_face_get_attention\n");
	if (index >= comm_handle.ucpf_ext.num) {
		return -1;
	}
	return comm_handle.ucpf_ext.attention[index];
}

int comm_device_face_get_emotion(int index){
	LOG("comm_device_face_get_emotion\n");
	if (index >= comm_handle.ucpf_ext.num) {
		return -1;
	}
	return comm_handle.ucpf_ext.emotion[index];
}

int comm_device_face_get_fr_feature(int index, float *feature){
int j = 0;
	LOG("comm_device_face_get_fr_feature\n");
	if (index >= comm_handle.ucpf_ext.num) {
		return -1;
	}

//	printf("index=%d, fr: ",index);
//	for(j = 0; j< FACE_RECOGNITION_FEATURE_DIMENSION; j++){
//		printf("%.2f ", comm_handle.ucpf_ext.fr_feature[index][j]);
//	}
//	printf("\n");
	memcpy(feature, comm_handle.ucpf_ext.fr_feature[index], FACE_RECOGNITION_FEATURE_DIMENSION*sizeof(float));

	return 0;
}

int comm_device_face_get_age(int index){

	LOG("comm_device_face_get_age\n");
	if (index >= comm_handle.ucpf_ext.num) {
		return -1;
	}
	return comm_handle.ucpf_ext.age[index];
}

int comm_device_face_get_gender(int index){
	LOG("comm_device_face_get_gender\n");
	if (index >= comm_handle.ucpf_ext.num) {
		return -1;
	}
	return comm_handle.ucpf_ext.gender[index];
}

int comm_device_get_faceid(int index){
	LOG("comm_device_face_get_gender\n");
	if (index >= comm_handle.ucpf_ext.num) {
		return -1;
	}
	return comm_handle.ucpf_ext.ids[index];
}

int* comm_device_get_face_rect(int index){
	int data[4];
	LOG("comm_device_get_face_rect\n");
	if (index >= comm_handle.ucpf_ext.num) {
		return NULL;
	}
	return &comm_handle.ucpf_ext.rect[index];
}

int comm_device_get_face_num(){
	
	LOG("comm_device_get_face_num\n");
	return comm_handle.ucpf_ext.num;
}

int comm_device_unregister_faceid(int id){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_unregister_faceid\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_UNREGISTER_FACEID;
	ucp_reply.param[1] = 1;
	ucp_reply.param[2] = id;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

int comm_device_register_faceid(int index){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_register_faceid\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_REGISTER_FACEID;
	ucp_reply.param[1] = 0;
	ucp_reply.param[2] = index;

	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

int comm_device_exit_register_mode(){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_exit_register_mode\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_EXIT_REGISTER_MODE;
	ucp_reply.param[1] = 0;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

int comm_device_enter_register_mode(){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_enter_register_mode\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_ENTER_REGISTER_MODE;
	ucp_reply.param[1] = 0;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}
int comm_device_isalive(){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_isalive\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_ISALIVE;
	ucp_reply.param[1] = 0;
	
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

int comm_device_isactivate(){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_isalive\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_IS_ACTIVATE;
	ucp_reply.param[1] = 0;

	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	return ret;
}

int comm_device_release(){
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_release\n");
	
	//get face info thread exit
	comm_handle.comm_service_thread_exit = 1;
	pthread_join(comm_handle.comm_service_thread_tid, NULL);
	usleep(50*1000);
	/* some time may block and unkown err because usb disconnect now, and then device will restart, so we donot send msg
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_RELEASE;
	ucp_reply.param[1] = 0;
	
	//notify remote device release
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	ret = ucp_reply.param[2];//result
	*/

	algo_device_release();
	comm_handle.inited = 0;
	
	comm_handle.release = 1;
	return ret;
}

int comm_device_get_face_info() {
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

//	LOG("comm_device_get_face_info\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_GET_FACES_INFO;
	ucp_reply.param[1] = 0;
//	printf("%s,l:%d,sizeof(struct usb_comm_protocol_f):%x\n", __FUNCTION__, __LINE__, sizeof(struct usb_comm_protocol_f));
	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &comm_handle.ucpf, sizeof(struct usb_comm_protocol_f));
	
	if (ret == -1)
		return 0;
	
	ret = comm_handle.ucpf.num;//result

	return ret;
}

int comm_device_get_face_info_ext() {
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

//	LOG("comm_device_get_face_info\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_GET_FACES_INFO_EXT;
	ucp_reply.param[1] = 0;

	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &comm_handle.ucpf_ext, sizeof(struct usb_comm_protocol_f_ext));
//	printf("%s,l:%d,ret:%d\n", __FUNCTION__, __LINE__, ret);
	if (ret == -1)
		return 0;
	
	ret = comm_handle.ucpf_ext.num;//result

	return ret;
}

static void *comm_service_thread_handle(void *arg) {
	int ret = 0;
	int sleep_time = 0;
    int event = 0;
	memset(&comm_handle.ucpf_ext, 0, sizeof(struct usb_comm_protocol));
	
	while (!comm_handle.comm_service_thread_exit) {
		if (comm_handle.release == 0) {
            ret = comm_device_get_face_info_ext();
		    if (ret > 0) {
		        event = 1;
		    } else {
                event = 0;
            }
		    comm_handle.face_detect_callback(event);
	    }
		sleep_time = 1000*(1000/comm_handle.call_back_freq);
	    if (!comm_handle.comm_service_thread_exit)
            usleep(sleep_time);//100ms
    }
	pthread_exit(NULL);
	LOG("comm_service_thread_handle exit\n");
	return NULL;
}

int comm_device_get_body_info() {
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_get_body_info\n");
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_GET_BODYS_INFO;
	ucp_reply.param[1] = 0;
	
	//ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	//ret = ucp_reply.param[2];//result
	return 0;
}

int comm_device_init(void *face_detect_callback, void *body_detect_callback,
					int pid, int vid, int fd, char *serial, int busnum, int devaddr) {
	struct usb_comm_protocol ucp_reply;
	int ret = 0;

	LOG("comm_device_init:%d,comm_handle.release:%d\n",comm_handle.inited, comm_handle.release);

	if (comm_handle.inited){
		comm_handle.release = 0;
		LOG("comm_device_init already:%d", comm_handle.release);
		return 0;
	}

	memset(&comm_handle, 0, sizeof(struct comm_service_handle));
	comm_handle.face_detect_callback = face_detect_callback;
	comm_handle.body_detect_callback = body_detect_callback;
	comm_handle.call_back_freq = 30;
	comm_handle.comm_service_thread_exit =0;
#ifdef LIBUSB_OPEN_WITH_FD
	ret = algo_device_init_with_fd(pid, vid, fd, serial, busnum, devaddr);
#else
	ret = algo_device_init();//do device open
#endif

if (ret == -1) {
		LOG("algo_device_init fail, maybe device not exsit\n");
		return ret;
	}
	
	//now start comm
	memset(&ucp_reply, 0, sizeof(struct usb_comm_protocol));
	ucp_reply.magic = DEVICE_COMM_SERVICE_MAGIC;
	ucp_reply.param[0] = DEVICE_INIT;
	ucp_reply.param[1] = 0;

	ret = algo_device_request(&ucp_reply, sizeof(struct usb_comm_protocol), &ucp_reply, sizeof(struct usb_comm_protocol));
	if (ret == -1)
		return ret;

	ret = ucp_reply.param[2];//result
	ret = pthread_create(&comm_handle.comm_service_thread_tid, NULL, comm_service_thread_handle, NULL);
	comm_handle.inited = 1;

	comm_handle.release = 0;
	return ret;
}


#ifdef __cplusplus
}
#endif
