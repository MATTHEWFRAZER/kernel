obj-m += periodic_timer_module.o
periodic_timer_module-y := periodic_timer.o periodic_timer_implementation.o periodic_timer_ioctl.o
modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
