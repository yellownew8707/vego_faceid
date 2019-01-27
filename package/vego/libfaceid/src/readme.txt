
void face_detect_callback(int event) {

	//detect faces
	if (event == 1){
		int ret = dev_getFaceNum();
		int *rect = NULL;
		num =ret;
		printf("num:%d\n", ret);
		for(int i = 0; i < num; i++){
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
			if (dev_getFaceID(i) =< 0) {
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
int main(){

	dev_init(face_detect_callback, NULL,0, 0, 0, NULL, 0, 0);
	//设置回调频率，30FPS
	dev_setCallBackFreq(0, 30);
	printf("version:%s\n", dev_getSysVersion());
	
	dev_release();
}