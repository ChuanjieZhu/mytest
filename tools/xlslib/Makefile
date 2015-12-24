TARGET = xlslib_test

CC = gcc
CPP = g++
LD = ld
AR = ar

CPPFLAGS = -g -Wall

OBJS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

$(TARGET) : $(OBJS)
	$(CPP) $^ -o $@ $(CPPFLAGS) -L./lib -lxls
	
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) -o $@ $<

clean:
	rm *.o $(TARGET)

.PHONY:clean


