CC = gcc
CXX = clang++
LDFLAGS = -lfmt
CXXFLAGS = -O2 -Wall -pedantic -Wextra -std=c++20
INC = 

RM = rm

SRC_FILES := $(wildcard *.cpp)
TARGETS := $(basename $(notdir $(SRC_FILES)))
BUILD_DIR = build
TARGET_PATHS = $(addprefix $(BUILD_DIR)/,$(TARGETS))

all: prebuild $(TARGET_PATHS)

prebuild:
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%: %.cpp
	$(CXX) $(INC) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all prebuild clean