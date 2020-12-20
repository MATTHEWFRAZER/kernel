#pragma once

#define PWQ_IOC_MAGIC 'p'
#define PWQ_IOC_MAXNR 2

#define PWQ_START_IOCTL _IO(PWQ_IOC_MAGIC, 1)
#define PWQ_SET_DELAY_IOCTL _IOW(PWQ_IOC_MAGIC, 2, int*)
