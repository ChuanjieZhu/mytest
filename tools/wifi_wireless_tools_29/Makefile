
TARGET = testWifi

ifdef HI3518 
	export CC = arm-hisiv100nptl-linux-gcc
	export CX = arm-hisiv100nptl-linux-g++
	export AR = arm-hisiv100nptl-linux-ar
else
	export CC = arm_v5t_le-gcc 
	export CX = arm_v5t_le-g++
	export AR = arm_v5t_le-ar
#else
#	export CC = gcc
#	export CX = g++
#	export AR = ar 
endif

CSRCS = $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)

OBJS = $(CSRCS:.c = .o)
OBJS += $(CPPSRCS:.cpp = .o)

INCLUDE = -I/xxx
CPPFLAGS = -g -Wall -lpthread

LIBS += -lrt -lm


$(TARGET) : $(OBJS)
	$(CC) $^ -o $@ $(CPPFLAGS) -L. $(LIBS)

%.o: %.c 
	$(CC) -c $(CPPFLAGS) -o $@ $< 

%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) -o $@ $<

clean:
	rm *.o $(TARGET)

.PHONY:clean
