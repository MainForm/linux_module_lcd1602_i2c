KERNEL_DIR := /lib/modules/$(shell uname -r)/build
SRC_DIR    := $(CURDIR)/src
BUILD_DIR  := $(CURDIR)/build

.PHONY: all clean

all:
	@echo "Building the LCD1602 module..."
	@mkdir -p $(BUILD_DIR)
	@cp $(SRC_DIR)/*.[ch] $(BUILD_DIR)
	@cp $(SRC_DIR)/Kbuild $(BUILD_DIR)
	$(MAKE) -C $(KERNEL_DIR) M=$(BUILD_DIR) modules

clean:
	$(RM) -r $(BUILD_DIR)