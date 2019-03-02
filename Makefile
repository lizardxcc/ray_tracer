CXX = g++-8

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
DEBUG_OBJS_DIR = ./objs/debug
DEBUG_OBJS = $(SRCS:%.cpp=$(DEBUG_OBJS_DIR)/%.o)

DEPS_DIR = ./objs
RELEASE_DEPS_DIR = $(DEPS_DIR)/release
RELEASE_DEPS = $(SRCS:%.cpp=$(RELEASE_DEPS_DIR)/%.d)
DEBUG_DEPS_DIR = $(DEPS_DIR)/debug
DEBUG_DEPS = $(SRCS:%.cpp=$(DEBUG_DEPS_DIR)/%.d)


DEBUG_FLAGS = -g3 -O0
RELEASE_FLAGS = -O3

CXXFLAGS = -Wall -Wextra -std=c++14
CXXFLAGS += -MMD -MP

CXXFLAGS += -I/usr/local/Cellar/boost/1.68.0_1/include
LDFLAGS = -L/usr/local/Cellar/boost/1.68.0_1/lib
LDLIBS =
OPENMPFLAGS = -fopenmp


.PHONY: all debug release clean

all: $(RELEASE_TARGET)

run: $(RELEASE_TARGET)
	$(RELEASE_TARGET) $(ARGS)

release: $(RELEASE_TARGET)

debug: $(DEBUG_TARGET)
	
$(RELEASE_TARGET) : $(RELEASE_BIN_DIR) $(RELEASE_OBJS) Makefile
	$(CXX) $(LDLIBS) $(OPENMPFLAGS) -o $@ $(RELEASE_OBJS)

$(DEBUG_TARGET) : $(DEBUG_BIN_DIR) $(DEBUG_OBJS) Makefile
	$(CXX) $(LDLIBS) -o $@ $(DEBUG_OBJS)

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

$(RELEASE_OBJS_DIR)/%.o: %.cpp $(RELEASE_OBJS_DIR) $(RELEASE_DEPS_DIR) Makefile
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(RELEASE_FLAGS) $(OPENMPFLAGS) -c -o $@ $<

$(DEBUG_OBJS_DIR)/%.o: %.cpp $(DEBUG_OBJS_DIR) $(DEBUG_DEPS_DIR) Makefile
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEBUG_FLAGS) -c -o $@ $<

-include $(RELEASE_DEPS)
-include $(DEBUG_DEPS)


clean:
	rm -rf $(BIN_DIR) $(OBJS_DIR) $(DEPS_DIR)
