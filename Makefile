BUILD 	:= build
OUTPUT 	:= out

TARGET_EMU 			:= $(OUTPUT)/gbemu
TARGET_TEST 		:= $(OUTPUT)/test

TARGETS := \
	$(TARGET_EMU) \
	$(TARGET_TEST)

CXX := clang++
LD := lld
STD := c++20
OPT := -O0

DEBUG ?= 3
LTO ?= 0
ASAN ?= 1

WARN := -Wall -Wextra -Werror \
	-Wno-unused-parameter \
	-Wno-unused-private-field \
	-Wno-unused-variable

CXXFLAGS := -std=$(STD) -g$(DEBUG) $(OPT) $(WARN)
CPPFLAGS := -MMD
LDFLAGS :=
LIBS :=

ifneq ($(ASAN),0)
	CXXFLAGS += -fsanitize=address
	LDFLAGS += -fsanitize=address
endif
ifneq ($(LTO),0)
	LDFLAGS += -flto
endif

INCDIRS := \
	src \
	3rd-party

CXXFILES_EMU := \
	src/main.cpp \
	src/common/arg_parser.cpp \
	src/common/fs.cpp \
	src/core/cart.cpp \
	src/core/cpu.cpp \
	src/core/gameboy.cpp \
	src/core/memory.cpp \
	src/core/ppu.cpp

CXXFILES_TEST := \
	src/common/arg_parser.cpp \
	src/core/cart.cpp \
	src/core/cpu.cpp \
	src/core/memory.cpp \
	test/test_arg_parser.cpp \
	test/test_cpu.cpp \
	test/test_memory.cpp

OFILES_EMU := $(CXXFILES_EMU:%.cpp=$(BUILD)/%.o)
OFILES_TEST := $(CXXFILES_TEST:%.cpp=$(BUILD)/%.o)

DFILES := \
	$(OFILES_EMU:%.o=%.d) \
	$(OFILES_TEST:%.o=%.d)

SRCDIRS := $(shell find . -type d -not -path "*$(BUILD)*")
$(shell mkdir -p $(SRCDIRS:%=$(BUILD)/%))
$(shell mkdir -p $(OUTPUT))

all: $(TARGETS)

clean:
	rm -rf build

test: build-test
	$(TARGET_TEST)

build-test: $(TARGET_TEST)


$(TARGET_EMU) : $(OFILES_EMU)

$(TARGET_TEST): $(OFILES_TEST)
$(TARGET_TEST): LIBS += gtest gtest_main

$(TARGETS):
	$(CXX) -fuse-ld=$(LD) $(LDFLAGS) $(LIBS:%=-l%) $^ -o $@

$(BUILD)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCDIRS:%=-I%) -c $< -o $@

.PHONY: all clean build-test test

-include $(DFILES)
