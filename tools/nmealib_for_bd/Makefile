TARGET = libnmeagps.so

INCLUDE_FILE = context.h parse.h parser.h gtime.h
INCLUDE_FILE += tok.h units.h gmath.h nmea.h sentence.h info.h config.h

COMPILER_PATH = arm-linux-

CC = $(COMPILER_PATH)gcc
CXX = $(COMPILER_PATH)g++
LD = $(COMPILER_PATH)ld
AR = $(COMPILER_PATH)ar

LIBRARY = ./lib/

INCLUDE_DIR = ./include/

FLAGS = -Wall -O3 -s -I. -I$(INCLUDE_DIR)

ifdef S3C2440 
FLAGS += -DPLAT_S3C2440
else ifdef TI6446 
FLAGS += -DPLAT_TI6446
else
endif

OBJS = $(patsubst %.c, %.o, $(wildcard *.c))

%.o:%.c
	@$(CC) -c $(FLAGS) $< -o $@ 

#制作成.so文件
all:$(OBJS)
	@$(CC) -shared -Wall -fPIC -s $(OBJS) -o $(TARGET)

	@cp $(TARGET) $(LIBRARY) 1>/dev/null
	@cp $(INCLUDE_FILE) $(INCLUDE_DIR) 1>/dev/nul
	
update:
	@cp -a $(INCLUDE_FILE) $(INCLUDE_DIR) 1>/dev/null

clean:
	rm $(TARGET) *.o -f
	rm lib/* -f
	rm include/* -f
