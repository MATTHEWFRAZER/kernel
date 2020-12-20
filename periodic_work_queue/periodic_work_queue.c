#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>

#include "periodic_work_queue_ioctl.h"
#include "periodic_work_queue_implementation.h"

MODULE_DESCRIPTION("example of a periodic work queue");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Frazer");

#define FIRST_MINOR 0
#define MINOR_CNT 1

static const char g_device_name[] = "periodic_work_queue_module";
static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

static const struct file_operations g_periodic_work_queue_file_operations =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = pwq_handle_ioctl
};

static int __init pwq_init(void)
{
    int result;
    struct device *dev_ret;

    printk(KERN_NOTICE "%s: periodic work queue init\n", __FUNCTION__);

    if ((result = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "periodic_work_queue_module_ioctl")) < 0)
    {
        return result;
    }

    cdev_init(&c_dev, &g_periodic_work_queue_file_operations);

    if ((result = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
    {
        return result;
    }

    if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
    {
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, g_device_name)))
    {
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }

    return 0;
}

static void __exit pwq_exit(void)
{
    printk(KERN_NOTICE "%s: periodic work queue exit\n", __FUNCTION__);

    pwq_work_queue_exit();

    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
    return;
}

module_init(pwq_init);
module_exit(pwq_exit);
