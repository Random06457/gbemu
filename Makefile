include prettyprint.mk

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
TIME_TRACE ?= 0

WARN := -Wall -Wextra -Werror \
	-Wno-unused-parameter \
	-Wno-unused-private-field \
	-Wno-unused-variable \
	-Wno-unused-function

CXXFLAGS := -std=$(STD) -g$(DEBUG) $(OPT) $(WARN)
CPPFLAGS := -MMD
LDFLAGS :=
LIBS := \
	GL \
	glfw \
	SDL2

ifneq ($(ASAN),0)
	CXXFLAGS += -fsanitize=address
	LDFLAGS += -fsanitize=address
endif
ifneq ($(LTO),0)
	CXXFLAGS += -flto
	CFLAGS += -flto
	LDFLAGS += -flto
endif
ifneq ($(TIME_TRACE),0)
	CXXFLAGS += -ftime-trace
	CFLAGS += -ftime-trace
endif

INCDIRS := \
	src \
	3rd-party

CXXFILES_EMU := \
	src/main.cpp \
	src/common/arg_parser.cpp \
	src/common/fs.cpp \
	src/common/logging.cpp \
	src/core/mbc/rom.cpp \
	src/core/mbc/mbc1.cpp \
	src/core/mbc/mbc3.cpp \
	src/core/apu.cpp \
	src/core/cart.cpp \
	src/core/cpu.cpp \
	src/core/disas.cpp \
	src/core/gameboy.cpp \
	src/core/int_controller.cpp \
	src/core/joypad.cpp \
	src/core/serial.cpp \
	src/core/memory.cpp \
	src/core/timer.cpp \
	src/core/ppu.cpp \
	src/gui/audio_player.cpp \
	src/gui/gui_main.cpp \
	src/gui/joypad_process.cpp \
	src/gui/ppu_render.cpp

CXXFILES_TEST := \
	src/common/arg_parser.cpp \
	src/common/logging.cpp \
	src/core/mbc/rom.cpp \
	src/core/mbc/mbc1.cpp \
	src/core/mbc/mbc3.cpp \
	src/core/cart.cpp \
	src/core/cpu.cpp \
	src/core/disas.cpp \
	src/core/int_controller.cpp \
	src/core/memory.cpp \
	src/core/timer.cpp \
	test/test_arg_parser.cpp \
	test/test_cpu.cpp \
	test/test_memory.cpp

# fmtlib
CXXFILES_FMTLIB := \
	3rd-party/fmt/src/format.cc \
	3rd-party/fmt/src/os.cc
INCDIRS += 3rd-party/fmt/include

# imgui
CXXFILES_IMGUI := \
	3rd-party/imgui/imgui_impl_opengl3.cpp \
	3rd-party/imgui/imgui_impl_glfw.cpp \
	3rd-party/imgui/imgui_draw.cpp \
	3rd-party/imgui/imgui_tables.cpp \
	3rd-party/imgui/imgui_widgets.cpp \
	3rd-party/imgui/imgui.cpp

CXXFILES_EMU += $(CXXFILES_FMTLIB) $(CXXFILES_IMGUI)
CXXFILES_TEST += $(CXXFILES_FMTLIB)

OFILES_EMU := $(CXXFILES_EMU:%.cpp=$(BUILD)/%.o)
OFILES_EMU := $(OFILES_EMU:%.cc=$(BUILD)/%.o)

OFILES_TEST := $(CXXFILES_TEST:%.cpp=$(BUILD)/%.o)
OFILES_TEST := $(OFILES_TEST:%.cc=$(BUILD)/%.o)

DFILES := \
	$(OFILES_EMU:%.o=%.d) \
	$(OFILES_TEST:%.o=%.d)

SRCDIRS := $(shell find . -type d -not -path "*$(BUILD)*")
$(shell mkdir -p $(SRCDIRS:%=$(BUILD)/%))
$(shell mkdir -p $(OUTPUT))

all: $(TARGETS)
	$(shell cp -r src/gui/shaders $(OUTPUT))

clean:
	rm -rf build

test: build-test
	$(TARGET_TEST)

build-test: $(TARGET_TEST)


$(TARGET_EMU) : $(OFILES_EMU)

$(TARGET_TEST): $(OFILES_TEST)
$(TARGET_TEST): LIBS += gtest gtest_main

$(TARGETS):
	$(V)$(CXX) -fuse-ld=$(LD) $(LDFLAGS) $(LIBS:%=-l%) $^ -o $@
	$(call printtask,Linking,$@)

$(BUILD)/%.o : %.cpp
	$(V)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCDIRS:%=-I%) -c $< -o $@
	$(call printtask,Compiling,$@)

$(BUILD)/%.o : %.cc
	$(V)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCDIRS:%=-I%) -c $< -o $@
	$(call printtask,Compiling,$@)

.PHONY: all clean build-test test

-include $(DFILES)
