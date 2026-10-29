CXXFLAGS ?= -std=c++20 -Wall -Wextra -O2 -g
CPPFLAGS += -Iinclude
LDFLAGS += -pthread

BUILD_DIR := build
EXAMPLE := $(BUILD_DIR)/iro_spsc_example

.PHONY: all run clean

all: $(EXAMPLE)

$(EXAMPLE): main.cpp include/iro/lockfree/spsc_queue.hpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $@

run: $(EXAMPLE)
	./$(EXAMPLE)

clean:
	rm -rf $(BUILD_DIR)
