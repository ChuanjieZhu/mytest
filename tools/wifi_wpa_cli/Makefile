
TARGET = wifiTest

ifdef HI3518
	CC = arm-hisiv100nptl-linux-gcc
	CX = arm-hisiv100nptl-linux-g++
	AR = arm-hisiv100nptl-linux-ar
else
	CC = gcc
	CX = g++
	AR = ar
endif

FLAGS = -Wall

OBJS = $(patsubst %.c, %.o, $(wildcard *.c))
OBJS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))

$(TARGET):$(OBJS)
	$(CC) $^ -o $@ $(FLAGS)

%.o:%.c
	$(CC) -c $(FLAGS) $< -o $@

%.o:%.cpp
	$(CX) -c $(FLAGS) $< -o $@

clean:
	rm -f *.o $(TARGET)

.PHONY:clean

