obj-m += i2c-module.o

BUILDROOT_DIR ?= /home/rgnagel/Desktop/buildroot

KERNEL_DIR ?= $(BUILDROOT_DIR)/output/build/linux-5.4.30

CROSS_COMPILE ?= $ $(BUILDROOT_DIR)/output/host/bin/arm-none-linux-gnueabihf-

all:
	make -C $(KERNEL_DIR) \
			ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) \
			M=$(PWD) modules
clean:
	make -C $(KERNEL_DIR) \
			ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) \
			SUBDIRS=$(PWD) clean

deploy:
	scp *.ko root@192.168.10.1:/lib/modules/5.4.30/