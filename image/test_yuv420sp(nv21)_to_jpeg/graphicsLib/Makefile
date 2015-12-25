TARGET = libgraphics.a

LIBRARY = ../lib/

INCLUDE_DIR = ../include
INCLUDE_INSIDE_DIR = ../include/inside
INCLUDE_VENC_DIR = ../include/venc

#LIBRARY_DIR += -L../lib/ffmpeg/

ifdef PRINT_DEBUG
	FLAGS += -DPRINT_DEBUG
endif

ifdef GDB_DEBUG
FLAGS = -g -Wall -gstabs+ -I. -I$(INCLUDE_DIR) -I$(INCLUDE_INSIDE_DIR)
else
FLAGS = -Wall -O3 -s -I. -I$(INCLUDE_DIR) -I$(INCLUDE_INSIDE_DIR) -I$(INCLUDE_VENC_DIR)
endif


#替换所有的.c文件名为.o文件名，并保存到OBJS中
OBJS = $(patsubst %.c, %.o, $(wildcard *.c))
OBJS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))

#编译所有的.c文件成.o
%.o:%.c
	@$(CC) -c $(FLAGS) $< -o $@ 1>/dev/null

%.o:%.cpp
	@$(CX) -c $(FLAGS) $< -o $@ 1>/dev/null

#制作成.a文件
all:$(OBJS)
	@$(AR) -r $(TARGET) $(OBJS) 1>/dev/null
#	@$(CC) -shared -Wall -fPIC -s $(OBJS) -o $(TARGET)
	@cp $(TARGET) $(LIBRARY) 1>/dev/null

clean:
	rm $(TARGET) *.o -f
