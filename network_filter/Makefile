CONFIG_MODULE_SIG=n

obj-m += network_filter.o
network_filter_module-y := network_filter.o
modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
