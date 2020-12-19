#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>

#include "periodic_timer_ioctl_common.h"
#include "periodic_timer_implementation.h"

long pt_handle_ioctl(struct file *pfile, unsigned int ioctl, unsigned long argument)
{
    if(_IOC_TYPE(ioctl) != PT_IOC_MAGIC)
    {
        return -ENOTTY;
    }

    switch(ioctl)
    {
        case PT_START_IOCTL:
            printk(KERN_NOTICE "%s: periodic timer start\n", __FUNCTION__);
            pt_timer_init();
            break;
        case PT_SET_EXPIRATION_IOCTL:
        {
            int expiration_time;

            if(!access_ok(argument, sizeof(int)))
            {
                printk(KERN_ERR "%s: could not access expiration from user\n", __FUNCTION__);
                return -EACCES;
            }

            if(copy_from_user(&expiration_time, (int *)argument, sizeof(int)))
            {
                printk(KERN_ERR "%s: could not copy expiration from user\n", __FUNCTION__);
                return -EACCES;
            }
            pt_set_timer(expiration_time);
            break;
        }
        default:
            printk(KERN_ERR "%s: invalid ioctl %d\n", __FUNCTION__, ioctl);
            return -EINVAL;

    }
    return 0;
}
