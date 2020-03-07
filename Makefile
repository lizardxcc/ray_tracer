#CXX = g++-9
CXX = g++

TARGET = renderer
SRCS = $(wildcard *.cpp)
TEST_SRCS = $(wildcard test/*.cpp)

BIN_DIR = ./bin
RELEASE_BIN_DIR = ./bin/release
RELEASE_TARGET = $(RELEASE_BIN_DIR)/$(TARGET)
CLI_BIN_DIR = ./bin/cli
CLI_TARGET = $(CLI_BIN_DIR)/$(TARGET)
DEBUG_BIN_DIR = ./bin/debug
DEBUG_TARGET = $(DEBUG_BIN_DIR)/$(TARGET)

TEST_RELEASE_TARGET = $(RELEASE_BIN_DIR)/test

OBJS_DIR = ./objs
RELEASE_OBJS_DIR = ./objs/release
TEST_RELEASE_OBJS_DIR = ./objs/release/test
RELEASE_OBJS = $(SRCS:%.cpp=$(RELEASE_OBJS_DIR)/%.o)
RELEASE_OBJS += $(RELEASE_OBJS_DIR)/gl3w.o
TEST_RELEASE_OBJS = $(filter-out $(RELEASE_OBJS_DIR)/main.o, $(RELEASE_OBJS))
TEST_RELEASE_OBJS += $(TEST_SRCS:%.cpp=$(RELEASE_OBJS_DIR)/%.o)

CLI_OBJS_DIR = ./objs/cli
CLI_OBJS = $(filter-out $(CLI_OBJS_DIR)/im%, $(SRCS:%.cpp=$(CLI_OBJS_DIR)/%.o))
DEBUG_OBJS_DIR = ./objs/debug
DEBUG_OBJS = $(SRCS:%.cpp=$(DEBUG_OBJS_DIR)/%.o)
DEBUG_OBJS += $(DEBUG_OBJS_DIR)/gl3w.o

DEPS_DIR = ./objs
RELEASE_DEPS_DIR = $(DEPS_DIR)/release
TEST_RELEASE_DEPS_DIR = $(DEPS_DIR)/release/test
RELEASE_DEPS = $(SRCS:%.cpp=$(RELEASE_DEPS_DIR)/%.d)
RELEASE_DEPS += $(RELEASE_OBJS_DIR)/gl3w.d
CLI_DEPS_DIR = $(DEPS_DIR)/cli
CLI_DEPS = $(SRCS:%.cpp=$(CLI_DEPS_DIR)/%.d)
DEBUG_DEPS_DIR = $(DEPS_DIR)/debug
DEBUG_DEPS = $(SRCS:%.cpp=$(DEBUG_DEPS_DIR)/%.d)
DEBUG_DEPS += $(DEBUG_OBJS_DIR)/gl3w.d


DEBUG_FLAGS = -g3 -O0
RELEASE_FLAGS = -O3
CLI_FLAGS = -O3 -D _CLI

INCLUDE += -I./
INCLUDE += `pkg-config glm --cflags`
GUI_INCLUDE += `pkg-config glfw3 --cflags`
GUI_INCLUDE += -I./nativefiledialog/src/include

CFLAGS = -Wall -Wextra
CFLAGS += -Wno-unused-parameter
CFLAGS += -MMD -MP

CXXFLAGS = -Wall -Wextra -std=c++14
CXXFLAGS += -Wno-unused-parameter
CXXFLAGS += -MMD -MP

#CXXFLAGS += $(INCLUDE)
#CFLAGS += $(INCLUDE)

GUI_LDFLAGS += -L./nativefiledialog/build/lib/Release/x64
GUI_LDLIBS += `pkg-config glfw3 --libs`
GUI_LDLIBS += `pkg-config glm --libs`

ifeq ($(shell uname), Darwin)
GUI_LDLIBS += -framework OpenGL -framework CoreFoundation
GUI_LDLIBS += -framework Cocoa
OPENMPFLAGS = -Xpreprocessor -fopenmp
OPENMPLDLIBS = -lomp
#LDLIBS += -lGL
INCLUDE += -I/usr/local/Cellar/boost/1.71.0/include
LDFLAGS += -L/usr/local/Cellar/boost/1.71.0/lib
#OPENMPFLAGS = -fopenmp
else
GUI_LDLIBS += -lGL
GUI_LDLIBS += -ldl
GUI_LDLIBS += -lGLdispatch
LDLIBS += -lboost_system
GUI_LDLIBS += `pkg-config gtk+-3.0 --libs`
OPENMPFLAGS = -fopenmp
OPENMPLDLIBS = -liomp5
#OPENMPLDLIBS = -lgomp
endif

GUI_LDLIBS += -lnfd
LDLIBS += -lboost_filesystem
LDLIBS += -lboost_program_options

NATIVEFILEDIALOG_LIB = ./nativefiledialog/build/lib/Release/x64/libnfd.a


.PHONY: all test debug release clean

all: $(RELEASE_TARGET)

run: $(RELEASE_TARGET)
	$(RELEASE_TARGET) $(ARGS)

runtest: $(TEST_RELEASE_TARGET)
	$(TEST_RELEASE_TARGET) $(ARGS)

test: $(TEST_RELEASE_TARGET)

release: $(RELEASE_TARGET)

cli: $(CLI_TARGET)

debug: $(DEBUG_TARGET)
	
ifeq ($(shell uname), Darwin)
$(RELEASE_TARGET) : $(RELEASE_OBJS) $(NATIVEFILEDIALOG_LIB) Makefile | $(RELEASE_BIN_DIR)
	$(CXX) $(LDFLAGS) $(LDLIBS) $(GUI_LDFLAGS) $(GUI_LDLIBS) $(OPENMPFLAGS) $(OPENMPLDLIBS) -o $@ $(RELEASE_OBJS)
$(TEST_RELEASE_TARGET) : $(TEST_RELEASE_OBJS) $(NATIVEFILEDIALOG_LIB) Makefile | $(RELEASE_BIN_DIR)
	$(CXX) $(LDFLAGS) $(LDLIBS) $(GUI_LDFLAGS) $(GUI_LDLIBS) -lboost_unit_test_framework -lboost_test_exec_monitor $(OPENMPFLAGS) $(OPENMPLDLIBS) -o $@ $(TEST_RELEASE_OBJS)
else
$(RELEASE_TARGET) : $(RELEASE_OBJS) $(NATIVEFILEDIALOG_LIB) Makefile | $(RELEASE_BIN_DIR)
	#$(CXX) -Wl,--start-group $(LDFLAGS) $(LDLIBS) $(OPENMPFLAGS) $(OPENMPLDLIBS) -o $@ $(RELEASE_OBJS) -Wl,--end-group
	$(CXX) -Wl,--start-group $(LDFLAGS) $(LDLIBS) $(GUI_LDFLAGS) $(GUI_LDLIBS) $(OPENMPFLAGS) $(OPENMPLDLIBS) -o $@ $(RELEASE_OBJS) -Wl,--end-group
	#$(CXX) $(OPENMPFLAGS) $(OPENMPLDLIBS) $(LDFLAGS) $(LDLIBS) -o $@ $(RELEASE_OBJS)
endif
ifeq ($(shell uname), Darwin)
$(CLI_TARGET) : $(CLI_OBJS) Makefile | $(CLI_BIN_DIR)
	$(CXX) $(LDFLAGS) $(LDLIBS) $(OPENMPFLAGS) $(OPENMPLDLIBS) -o $@ $(CLI_OBJS)
else
$(CLI_TARGET) : $(CLI_OBJS) Makefile | $(CLI_BIN_DIR)
	#$(CXX) -Wl,--start-group $(LDFLAGS) $(LDLIBS) $(OPENMPFLAGS) $(OPENMPLDLIBS) -o $@ $(RELEASE_OBJS) -Wl,--end-group
	$(CXX) -Wl,--start-group $(LDFLAGS) $(LDLIBS) $(OPENMPFLAGS) $(OPENMPLDLIBS) -o $@ $(CLI_OBJS) -Wl,--end-group
	#$(CXX) $(OPENMPFLAGS) $(OPENMPLDLIBS) $(LDFLAGS) $(LDLIBS) -o $@ $(RELEASE_OBJS)
endif

$(DEBUG_TARGET) : $(DEBUG_OBJS) $(NATIVEFILEDIALOG_LIB) Makefile | $(DEBUG_BIN_DIR)
	$(CXX) $(LDFLAGS) $(LDLIBS) $(GUI_LDFLAGS) $(GUI_LDLIBS) -o $@ $(DEBUG_OBJS)

$(RELEASE_BIN_DIR):
	mkdir -p $@
$(RELEASE_OBJS_DIR):
	mkdir -p $@
$(TEST_RELEASE_OBJS_DIR):
	mkdir -p $@
$(CLI_BIN_DIR):
	mkdir -p $@
$(CLI_OBJS_DIR):
	mkdir -p $@
$(DEBUG_BIN_DIR):
	mkdir -p $@
$(DEBUG_OBJS_DIR):
	mkdir -p $@
$(DEPS_DIR):
	mkdir -p $@

ifeq ($(shell uname), Darwin)
$(NATIVEFILEDIALOG_LIB):
	$(MAKE) -C ./nativefiledialog/build/gmake_macosx config=release_x64 AR="/usr/bin/ar" RANLIB="/usr/bin/ranlib"
else
$(NATIVEFILEDIALOG_LIB):
	$(MAKE) -C ./nativefiledialog/build/gmake_linux
endif



$(TEST_RELEASE_OBJS_DIR)/%.o: test/%.cpp Makefile | $(TEST_RELEASE_OBJS_DIR) $(TEST_RELEASE_DEPS_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) $(GUI_INCLUDE) $(RELEASE_FLAGS) $(OPENMPFLAGS) -c -o $@ $<
$(RELEASE_OBJS_DIR)/%.o: %.cpp Makefile | $(RELEASE_OBJS_DIR) $(RELEASE_DEPS_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) $(GUI_INCLUDE) $(RELEASE_FLAGS) $(OPENMPFLAGS) -c -o $@ $<


$(RELEASE_OBJS_DIR)/gl3w.o: GL/gl3w.c Makefile | $(RELEASE_OBJS_DIR) $(RELEASE_DEPS_DIR)
	gcc $(CFLAGS) $(CPPFLAGS) $(INCLUDE) $(GUI_INCLUDE) $(RELEASE_FLAGS) -c -o $@ $<

$(CLI_OBJS_DIR)/%.o: %.cpp Makefile | $(CLI_OBJS_DIR) $(CLI_DEPS_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) $(CLI_FLAGS) $(OPENMPFLAGS) -c -o $@ $<


$(DEBUG_OBJS_DIR)/%.o: %.cpp Makefile | $(DEBUG_OBJS_DIR) $(DEBUG_DEPS_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) $(GUI_INCLUDE) $(DEBUG_FLAGS) -c -o $@ $<

$(DEBUG_OBJS_DIR)/gl3w.o: GL/gl3w.c Makefile | $(DEBUG_OBJS_DIR) $(DEBUG_DEPS_DIR)
	gcc $(CFLAGS) $(CPPFLAGS) $(INCLUDE) $(GUI_INCLUDE) $(DEBUG_FLAGS) -c -o $@ $<

-include $(RELEASE_DEPS)
-include $(CLI_DEPS)
-include $(DEBUG_DEPS)


clean:
	rm -rf $(BIN_DIR) $(OBJS_DIR) $(DEPS_DIR)
	rm -rf ./nativefiledialog/build/lib
	rm -rf ./nativefiledialog/build/obj
