TARGET = libjpg.a

LIBRARY = ../lib/

INCLUDE_DIR = ../include/

#LIBRARY_DIR += -L../lib/ffmpeg/

CFLAGS = -I. -I$(INCLUDE_DIR) -O3 -Wall

ifdef HI3518
	FLAGS += -DPLAT_HI3518
	CC = arm-hisiv100nptl-linux-gcc
endif

#替换所有的.c文件名为.o文件名，并保存到OBJS中
OBJS = jpeglib.o jcapimin.o jcapistd.o jccoefct.o jccolor.o jcdctmgr.o jchuff.o jcinit.o jcmainct.o jcmarker.o jcmaster.o jcomapi.o jcparam.o jcphuff.o jcprepct.o jcsample.o jctrans.o jdapimin.o jdapistd.o jdatadst.o jdatasrc.o jdcoefct.o jdcolor.o jddctmgr.o jdhuff.o jdinput.o jdmainct.o jdmarker.o jdmaster.o jdmerge.o jdphuff.o jdpostct.o jdsample.o jdtrans.o jerror.o jfdctflt.o jfdctfst.o jfdctint.o jidctflt.o jidctfst.o jidctint.o jidctred.o jmemmgr.o jmemnobs.o jpegtran.o jquant1.o jquant2.o jutils.o


#制作成.a文件
all: $(OBJS)
	#cp ../jdatabuf.o .
#	ar rv $(TARGET) $^ ../jdatabuf.o
#	@$(AR) rv $(TARGET) $^ ../jdatabuf.o
	@$(AR) rv $(TARGET) $^
	@cp $(TARGET) $(LIBRARY) 1>/dev/null
	
#编译所有的.c文件成.o
%.o:%.c
	$(CC) -c $(CFLAGS) $(@:.o=.c) 1>/dev/null
	
clean:
	rm $(TARGET) *.o -f
