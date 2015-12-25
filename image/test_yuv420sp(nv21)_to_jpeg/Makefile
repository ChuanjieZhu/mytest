
TARGET = mrj

ifdef HI3518
	export CC = arm-hisiv100nptl-linux-gcc
	export CX = arm-hisiv100nptl-linux-g++
	export AR = arm-hisiv100nptl-linux-ar
else
	export CC = gcc
	export CX = g++
	export AR = ar
endif

INCLUDE_DIR = ./include

JPGLIB 		= 	jpgLib
GRAPHICSLIB	= 	graphicsLib

LIBRARY_DIR += ./lib

LIBRARY_FILE += -lgraphics
LIBRARY_FILE += -ljpg

LIBRARY_FILE += -lpthread
LIBRARY_FILE += -lm
LIBRARY_FILE += -lrt

ifdef GDB_DEBUG
	FLAGS += -g -Wall -gstabs+
else
	FLAGS += -Wall -O3 -lm
endif

all: $(OBJS)
	@echo ""
	@echo "compile each lib, and create lib file ..."
	@make -C $(JPGLIB) all 1>/dev/null
	@make -C $(GRAPHICSLIB) all 1>/dev/null
	
	@echo ""
	@echo "create execute file ..."
	@$(CX) -o $(TARGET) $(FLAGS) -I$(INCLUDE_DIR) -L$(LIBRARY_DIR) $(LIBRARY_FILE)
	@echo ""

clean:
	@make -C $(JPGLIB) clean 1>/dev/null
	@make -C $(GRAPHICSLIB) clean 1>/dev/null
	@rm -f $(LIBRARY_DIR)/lib*.a 1>/dev/null
	@rm -f $(TARGET)
	
