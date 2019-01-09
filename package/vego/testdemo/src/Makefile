Target = testdemo
## set the include path for compile flags.
SourceIncludePath := -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner -I../../liballwinner/LIBRARY/libcedarc/include
CompileFlags = $(CFLAGS) $(SourceIncludePath)

LoadFlags += -lpthread -ldl -lstdc++ -lrt -lutils -lcutils -lmediaserver -lwifiswitch -lfaceid -lcdc_vdecoder -lcdc_memory

$(Target): testdemo.c CameraHardware.c iio_utils.c water_mark.c
	$(CC) -o $@ $^ $(CompileFlags) $(LDFLAGS) $(LoadFlags)
