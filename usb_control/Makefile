CONFIG_MODULE_SIG=n

obj-m += usb_control.o
usb_control_module-y := usb_control.o

modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
