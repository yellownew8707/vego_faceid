#ifndef __COMMON_H__
#define __COMMON_H__

#define LIBUSB_OPEN_WITH_FD

#define FW_DEFAULT_MOUNT_FULLNAME   "/mnt/sdcard/Firmware.img"
#define DEVICE_FACE_DB_ITEM_SIZE 	512

#define RV1108_TV_PID	0x0036
#define RV1108_TV_VID	0x2207


#define LOG printf
#define DEVICE_COMM_SERVICE_MAGIC 	0x0801

#define DEVICE_INIT 				1
#define DEVICE_RELEASE 				2
#define DEVICE_ISALIVE 				3
#define DEVICE_ENTER_REGISTER_MODE	4
#define DEVICE_EXIT_REGISTER_MODE	5
#define DEVICE_REGISTER_FACEID		6
#define DEVICE_UNREGISTER_FACEID	7
#define DEVICE_GET_FACES_INFO		8
#define DEVICE_GET_BODYS_INFO		9
#define DEVICE_TAKE_PICTURE			10
#define DEVICE_START_RECORD			11
#define DEVICE_STOP_RECORD			12
#define DEVICE_SET_RECORD_MODE		13
#define DEVICE_SET_FACE_DETECT_FPS	14
#define DEVICE_SWITCH_ALGO			15
#define DEVICE_GET_ALGO_STATUS		16
#define DEVICE_SWITCH_USB_MODE		17
#define DEVICE_START_UPFATE			18
#define DEVICE_GET_SYSTEM_VERSION	19
#define DEVICE_RESET_ALBUM			20
#define DEVICE_REBOOT_DEVICE		21

#define DEVICE_GET_CPUID			22
#define DEVICE_SET_SIGNATURE		23
#define DEVICE_OTA_FILE				24
#define DEVICE_IS_REG				25
#define DEVICE_IS_ACTIVATE 			26
#define DEVICE_GET_FACES_INFO_EXT 	27
#define DEVICE_SET_PHOTO_FILE		28

#define DEVICE_COMM_SERVICE_INT_LEN 	10
#define DEVICE_COMM_SERVICE_CHAR_LEN	256
#define RSFT_MAX_FACE_NUM 10
#define FACE_RECOGNITION_FEATURE_DIMENSION 512

struct usb_comm_protocol {
	int magic;
	int param[DEVICE_COMM_SERVICE_INT_LEN];
	char buffer[DEVICE_COMM_SERVICE_CHAR_LEN];//','隔开
};

struct face_rect {
	int left;
	int top;
	int right;
	int bottom;
};

struct usb_comm_protocol_f {
	int magic;
	int	num;
	int ids[RSFT_MAX_FACE_NUM];
	struct face_rect rect[RSFT_MAX_FACE_NUM];
	int age[RSFT_MAX_FACE_NUM];
	int attention[RSFT_MAX_FACE_NUM];
	int emotion[RSFT_MAX_FACE_NUM];
	int gender[RSFT_MAX_FACE_NUM];
};

struct usb_comm_protocol_f_ext {
	int magic;
	int	num;
	int ids[RSFT_MAX_FACE_NUM];
	struct face_rect rect[RSFT_MAX_FACE_NUM];
	int age[RSFT_MAX_FACE_NUM];
	int attention[RSFT_MAX_FACE_NUM];
	int emotion[RSFT_MAX_FACE_NUM];
	int gender[RSFT_MAX_FACE_NUM];
	float fr_feature[RSFT_MAX_FACE_NUM][FACE_RECOGNITION_FEATURE_DIMENSION];
};

#endif
