target =libfaceid.so

## set the include path for compile flags.
INCLUDES := -I./ \


LIBS += -lpthread -ldl -lutils -lcutils -lusb-1.0

SRCS += tv_sdk_api.c \
		tv_comm_service.c \
		libusbwrapper.c


install:
	echo "here is src install"
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	
	@cp ./tv_sdk_api.h $(CONFIG_PREFIX)/usr/include


$(target): $(SRCS)
	$(CC) -fPIC -shared $(INCLUDES) $(LDFLAGS) $^ -o $@ $(LIBS)
	echo "here is src install"
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)  
clean:
	rm -rf $(target)

