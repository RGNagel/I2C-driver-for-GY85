obj-m += GY-85.o

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
	scp GY-85.ko root@192.168.10.1:/lib/modules/5.4.30/kernel/drivers/i2c/

start:
	ssh root@192.168.10.1 "\
	insmod /lib/modules/5.4.30/kernel/drivers/input/input-polldev.ko;\
	mount -t debugfs none /sys/kernel/debug;\
	insmod /lib/modules/5.4.30/kernel/drivers/i2c/GY-85.ko;\
	"
