#CXX = g++-9
CXX = g++

TARGET = renderer
SRCS = $(wildcard *.cpp)

BIN_DIR = ./bin
RELEASE_BIN_DIR = ./bin/release
RELEASE_TARGET = $(RELEASE_BIN_DIR)/$(TARGET)
DEBUG_BIN_DIR = ./bin/debug
DEBUG_TARGET = $(DEBUG_BIN_DIR)/$(TARGET)

OBJS_DIR = ./objs
RELEASE_OBJS_DIR = ./objs/release
RELEASE_OBJS = $(SRCS:%.cpp=$(RELEASE_OBJS_DIR)/%.o)
RELEASE_OBJS += $(RELEASE_OBJS_DIR)/gl3w.o
DEBUG_OBJS_DIR = ./objs/debug
DEBUG_OBJS = $(SRCS:%.cpp=$(DEBUG_OBJS_DIR)/%.o)
DEBUG_OBJS += $(DEBUG_OBJS_DIR)/gl3w.o

DEPS_DIR = ./objs
RELEASE_DEPS_DIR = $(DEPS_DIR)/release
RELEASE_DEPS = $(SRCS:%.cpp=$(RELEASE_DEPS_DIR)/%.d)
RELEASE_DEPS += $(RELEASE_OBJS_DIR)/gl3w.d
DEBUG_DEPS_DIR = $(DEPS_DIR)/debug
DEBUG_DEPS = $(SRCS:%.cpp=$(DEBUG_DEPS_DIR)/%.d)
DEBUG_DEPS += $(DEBUG_OBJS_DIR)/gl3w.d


DEBUG_FLAGS = -g3 -O0
RELEASE_FLAGS = -O3

INCLUDE += -I/usr/local/Cellar/boost/1.70.0/include
INCLUDE += -I./
INCLUDE += `pkg-config glfw3 --cflags`
INCLUDE += `pkg-config glm --cflags`
INCLUDE += -I./nativefiledialog/src/include

CFLAGS = -Wall -Wextra
CFLAGS += -Wno-unused-parameter
CFLAGS += -MMD -MP

CXXFLAGS = -Wall -Wextra -std=c++14
CXXFLAGS += -Wno-unused-parameter
CXXFLAGS += -MMD -MP

CXXFLAGS += $(INCLUDE)
CFLAGS += $(INCLUDE)

LDFLAGS = -L/usr/local/Cellar/boost/1.70.0/lib
LDFLAGS += -L./nativefiledialog/build/lib/Release/x64
LDLIBS += `pkg-config glfw3 --libs`
LDLIBS += `pkg-config glm --libs`
LDLIBS += -framework OpenGL -framework CoreFoundation
LDLIBS += -framework Cocoa
LDLIBS += -lnfd
LDLIBS += -lboost_filesystem
#OPENMPFLAGS = -fopenmp
OPENMPFLAGS = -Xpreprocessor -fopenmp
OPENMPLDLIBS = -lomp

NATIVEFILEDIALOG_LIB = ./nativefiledialog/build/lib/Release/x64/libnfd.a


.PHONY: all debug release clean

all: $(RELEASE_TARGET)

run: $(RELEASE_TARGET)
	$(RELEASE_TARGET) $(ARGS)

release: $(RELEASE_TARGET)

debug: $(DEBUG_TARGET)
	
$(RELEASE_TARGET) : $(RELEASE_OBJS) $(NATIVEFILEDIALOG_LIB) Makefile | $(RELEASE_BIN_DIR)
	$(CXX) $(LDFLAGS) $(LDLIBS) $(OPENMPFLAGS) $(OPENMPLDLIBS) -o $@ $(RELEASE_OBJS)

$(DEBUG_TARGET) : $(DEBUG_OBJS) $(NATIVEFILEDIALOG_LIB) Makefile | $(DEBUG_BIN_DIR)
	$(CXX) $(LDFLAGS) $(LDLIBS) -o $@ $(DEBUG_OBJS)

$(DEBUG_BIN_DIR):
	mkdir -p $@
$(RELEASE_BIN_DIR):
	mkdir -p $@
$(DEBUG_OBJS_DIR):
	mkdir -p $@
$(RELEASE_OBJS_DIR):
	mkdir -p $@
$(DEPS_DIR):
	mkdir -p $@

$(NATIVEFILEDIALOG_LIB):
	$(MAKE) -C ./nativefiledialog/build/gmake_macosx config=release_x64 AR="/usr/bin/ar" RANLIB="/usr/bin/ranlib"


$(RELEASE_OBJS_DIR)/%.o: %.cpp Makefile | $(RELEASE_OBJS_DIR) $(RELEASE_DEPS_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(RELEASE_FLAGS) $(OPENMPFLAGS) -c -o $@ $<

$(RELEASE_OBJS_DIR)/gl3w.o: GL/gl3w.c Makefile | $(RELEASE_OBJS_DIR) $(RELEASE_DEPS_DIR)
	g++ $(CFLAGS) $(CPPFLAGS) $(RELEASE_FLAGS) -c -o $@ $<

$(DEBUG_OBJS_DIR)/%.o: %.cpp Makefile | $(DEBUG_OBJS_DIR) $(DEBUG_DEPS_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEBUG_FLAGS) -c -o $@ $<

$(DEBUG_OBJS_DIR)/gl3w.o: GL/gl3w.c Makefile | $(DEBUG_OBJS_DIR) $(DEBUG_DEPS_DIR)
	g++ $(CFLAGS) $(CPPFLAGS) $(DEBUG_FLAGS) -c -o $@ $<

-include $(RELEASE_DEPS)
-include $(DEBUG_DEPS)


clean:
	rm -rf $(BIN_DIR) $(OBJS_DIR) $(DEPS_DIR)
