CC = gcc
CXX = g++
LDFLAGS = -L ~/.local/lib/ -lopencv_core -lopencv_videoio -lopencv_imgcodecs -lopencv_highgui -lopencv_objdetect
CFLAGS = -O2 -Wall -pedantic -Wextra -Wl,--copy-dt-needed-entries
INC = -I ~/.local/include/opencv4/

TARGETS = main
# Directory to store the built targets
BUILD_DIR = build

# The paths to the built targets
TARGET_PATHS = $(addprefix $(BUILD_DIR)/,$(TARGETS))

all: $(TARGET_PATHS)

$(BUILD_DIR)/%: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(INC) $(CFLAGS) -o $@ $< $(LDFLAGS)

.PHONY: clean
clean:
	rm  $(TARGET_PATHS)
