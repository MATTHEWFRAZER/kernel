obj-m += periodic_work_queue_module.o
periodic_work_queue_module-y := periodic_work_queue.o periodic_work_queue_implementation.o periodic_work_queue_ioctl.o
modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
