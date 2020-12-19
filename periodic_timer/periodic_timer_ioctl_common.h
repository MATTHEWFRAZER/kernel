#pragma once

#define PT_IOC_MAGIC 'p'
#define PT_IOC_MAXNR 2

#define PT_START_IOCTL _IO(PT_IOC_MAGIC, 1)
#define PT_SET_EXPIRATION_IOCTL _IOW(PT_IOC_MAGIC, 2, int*)
