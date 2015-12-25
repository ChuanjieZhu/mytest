
TARGET = dnsclient

CC = arm_v5t_le-gcc

CXX = arm-linux-g++

CSRCS = $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)

OBJS = $(patsubst %.c, %.o, $(wildcard *.c))
OBJS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))

INCLUDE = -I/xxx
LIBS =
FLAGS = -Wall -O3 -s -Wcast-align -Wpadded -Wpacked

$(TARGET) : $(OBJS)
	$(CC) $^ -o $@ $(FLAGS) $(LIBS)
	
%.o: %.c 
	$(CC) -c $(FLAGS) -o $@ $< 

%.o: %.cxx
	$(CXX) -c $(FLAGS) -o $@ $<

#%.o:%.c
#	@$(CC) -c $(FLAGS) $< -o $@ 1>/dev/null
#%.o:%.cpp
#	@$(CC) -c $(FLAGS) $< -o $@ 1>/dev/null

#all:$(OBJS)
#	@$(AR) -r $(TARGET) $(OBJS) 1>/dev/null
#	@$(CC) -shared -Wall -fPIC -s $(OBJS) -o $(TARGET)
#	@cp $(TARGET) $(LIBRARY) 1>/dev/null


clean:
	rm *.o $(TARGET)

.PHONY:clean

