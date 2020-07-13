MAINSOURCE := Main.cpp webclient/Httpclient.cpp
# wildcard，查找以.cpp结尾的文件
SOURCE := $(wildcard *.cpp base/*.cpp webclient/*.cpp)
# filter-out，反过滤函数
override SOURCE := $(filter-out $(MAINSOURCE),$(SOURCE))
# patsubst，替换
OBJS := $(patsubst %.cpp,%.o,$(SOURCE))

TARGET := webserver
GXX := g++
CC := $(GXX)
LIBS := -lpthread
INCLUDE := -I./usr/local/lib
CFLAGS := -std=c++11 -g -Wall -O3 -D_PTHREADS
CXXFLAGS := $(CFLAGS)

SUBTARGET := Httpclient

.PHONY : objs clean veryclean rebuild all tests debug

all : $(TARGET) $(SUBTARGET)

objs : $(OBJS)

rebuild : veryclean all

tests : $(SUBTARGET)

clean : 
	find . -name '*.o' | xargs rm -f

veryclean :
	find . -name '*.o' | xargs rm -f
	find . -name $(TARGET) | xargs rm -f
	find . -name $(SUBTARGET) | xargs rm -f

debug:
	@echo $(SOURCE)

$(TARGET) : $(OBJS) Main.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

$(SUBTARGET) : $(OBJS) webclient/Httpclient.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)