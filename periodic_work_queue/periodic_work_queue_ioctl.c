#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>

#include "periodic_work_queue_ioctl_common.h"
#include "periodic_work_queue_implementation.h"

long pwq_handle_ioctl(struct file *pfile, unsigned int ioctl, unsigned long argument)
{
    if(_IOC_TYPE(ioctl) != PWQ_IOC_MAGIC)
    {
        return -ENOTTY;
    }

    switch(ioctl)
    {
        case PWQ_START_IOCTL:
            printk(KERN_NOTICE "%s: periodic queue start\n", __FUNCTION__);
            pwq_work_queue_init();
            break;
        case PWQ_SET_DELAY_IOCTL:
        {
            int delay;

            if(!access_ok(argument, sizeof(int)))
            {
                printk(KERN_ERR "%s: could not access delay from user\n", __FUNCTION__);
                return -EACCES;
            }

            if(copy_from_user(&delay, (int *)argument, sizeof(int)))
            {
                printk(KERN_ERR "%s: could not copy delay from user\n", __FUNCTION__);
                return -EACCES;
            }
            pwq_set_delay(delay);
            break;
        }
        default:
            printk(KERN_ERR "%s: invalid ioctl %d\n", __FUNCTION__, ioctl);
            return -EINVAL;

    }
    return 0;
}
