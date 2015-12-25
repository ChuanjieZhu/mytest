
TARGET = client33310

CC = gcc	
CXX = g++

LIBRARY_FILE += -lpthread
LIBRARY_FILE += -lrt

CSRCS = $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)

OBJS = $(patsubst %.c, %.o, $(wildcard *.c))
OBJS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))

INCLUDE = -I/xxx
FLAGS = -Wall -O3 -s -Wcast-align -Wpadded -Wpacked 

$(TARGET) : $(OBJS)	
	$(CXX) $^ -o $@ $(FLAGS) $(LIBRARY_FILE)	

%.o: %.c 	
	$(CC) $(FLAGS) -c -o $@ $<	

%.o: %.cpp
	$(CXX) $(FLAGS) -c -o $@ $<

#%.o:%.c
#	@$(CC) -c $(FLAGS) $< -o $@ 1>/dev/null

#%.o:%.cpp#	@$(CC) -c $(FLAGS) $< -o $@ 1>/dev/null

#all:$(OBJS)
#	@$(AR) -r $(TARGET) $(OBJS) 1>/dev/null
#	@$(CC) -shared -Wall -fPIC -s $(OBJS) -o $(TARGET)
#	@cp $(TARGET) $(LIBRARY) 1>/dev/null

clean:	
	rm *.o $(TARGET)

.PHONY:clean

