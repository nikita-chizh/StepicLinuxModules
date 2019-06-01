KERNEL := $(shell uname -r)
PWD    := $(shell pwd)
ccflags-y := -std=gnu99

KERNELDIR := /lib/modules/$(KERNEL)/build

obj-m := solution.o

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
