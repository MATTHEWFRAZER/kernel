#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

MODULE_DESCRIPTION("example of a usb driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Frazer");

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define BULK_EP_OUT 0x01
#define BULK_EP_IN 0x82
#define MAX_PKT_SIZE 512
#define MAX_PKT_SIZE 512

static struct usb_device *device;
static struct usb_class_driver class;
static unsigned char bulk_buf[MAX_PKT_SIZE];

static int pen_open(struct inode *i, struct file *f)
{
    return 0;
}
static int usb_control_close(struct inode *i, struct file *f)
{
    return 0;
}
static ssize_t usb_control_read(struct file *f, char __user *buf, size_t cnt, loff_t *off)
{
    int retval;
    int read_cnt;

    /* Read the data from the bulk endpoint */
    retval = usb_bulk_msg(device, usb_rcvbulkpipe(device, BULK_EP_IN), bulk_buf, MAX_PKT_SIZE, &read_cnt, 5000);
    if (retval)
    {
        printk(KERN_ERR "Bulk message returned %d\n", retval);
        return retval;
    }
    if (copy_to_user(buf, bulk_buf, MIN(cnt, read_cnt)))
    {
        return -EFAULT;
    }

    return MIN(cnt, read_cnt);
}

/*static void usb_control_write_bulk_callback(struct urb *urb, struct pt_regs *regs)
{
    // sync/async unlink faults aren't errors
    if (urb->status && !(urb->status =  = -ENOENT || urb->status =  = -ECONNRESET || urb->status =  = -ESHUTDOWN))
    {
        printk("%s - nonzero write bulk status received: %d", _ _FUNCTION_ _, urb->status);
    }

    /* free up our allocated buffer */
    usb_buffer_free(urb->dev, urb->transfer_buffer_length, urb->transfer_buffer, urb->transfer_dma);
}*/


static ssize_t usb_control_write(struct file *f, const char __user *user_buffer, size_t cnt, loff_t *off)
{
    int retval;
    int wrote_cnt = MIN(cnt, MAX_PKT_SIZE);
    /*struct urb * urb;

    // no iso_packets
    urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!urb)
    {
        return -ENOMEM;
    }

    buf = usb_buffer_alloc(device, count, GFP_KERNEL, &urb->transfer_dma);
    if (!buf)
    {
        return -ENOMEM;
    }

    if (copy_from_user(buf, user_buffer, count))
    {
        return -EFAULT;
    }

    usb_fill_bulk_urb(urb, device, usb_sndbulkpipe(device, BULK_EP_OUT), buf, count, usb_control_write_bulk_callback, device);
    urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    retval = usb_submit_urb(urb, GFP_KERNEL);
    if (retval)
    {
        printk("%s - failed submitting write urb, error %d", _ _FUNCTION_ _, retval);
        return retval;
    }*/

    if (copy_from_user(bulk_buf, buf, MIN(cnt, MAX_PKT_SIZE)))
    {
        return -EFAULT;
    }

    // Write the data into the bulk endpoint
    retval = usb_bulk_msg(device, usb_sndbulkpipe(device, BULK_EP_OUT), bulk_buf, MIN(cnt, MAX_PKT_SIZE), &wrote_cnt, 5000);
    if (retval)
    {
        printk(KERN_ERR "Bulk message returned %d\n", retval);
        return retval;
    }

    return wrote_cnt;
}

static struct file_operations fops =
{
    .owner   = THIS_MODULE,
    .open    = usb_control_open,
    .release = usb_control_close,
    .read    = usb_control_read,
    .write   = usb_control_write,
};

static int usb_control_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    struct usb_host_interface *iface_desc;
    struct usb_endpoint_descriptor *endpoint;
    int i;

    iface_desc = interface->cur_altsetting;
    printk(KERN_INFO "Pen i/f %d now probed: (%04X:%04X)\n", iface_desc->desc.bInterfaceNumber, id->idVendor, id->idProduct);
    printk(KERN_INFO "ID->bNumEndpoints: %02X\n", iface_desc->desc.bNumEndpoints);
    printk(KERN_INFO "ID->bInterfaceClass: %02X\n", iface_desc->desc.bInterfaceClass);

    for (i = 0; i < iface_desc->desc.bNumEndpoints; i++)
    {
        endpoint = &iface_desc->endpoint[i].desc;

        printk(KERN_INFO "ED[%d]->bEndpointAddress: 0x%02X\n", i, endpoint->bEndpointAddress);
        printk(KERN_INFO "ED[%d]->bmAttributes: 0x%02X\n", i, endpoint->bmAttributes);
        printk(KERN_INFO "ED[%d]->wMaxPacketSize: 0x%04X (%d)\n", i, endpoint->wMaxPacketSize, endpoint->wMaxPacketSize);
    }

    device = interface_to_usbdev(interface);
    printk(KERN_INFO "usb drive (%04X:%04X) plugged\n", id->idVendor, id->idProduct);

    class.name = "usb/usb_control%d";
    class.fops = &fops;
    if ((retval = usb_register_dev(interface, &class)) < 0)
    {
        /* Something prevented us from registering this driver */
        printk(KERN_ERR "Not able to get a minor for this device.");
    }
    else
    {
        printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
    }

    return 0;
}

static void usb_control_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "usb drive removed\n");
    usb_deregister_dev(interface, &class);
}

static int usb_control_suspend(struct usb_interface *interface, pm_message_t message)
{
    printk(KERN_INFO "%s event %d", __FUNCTION__, message.event);
}

static int usb_control_resume(struct usb_interface *interface)
{
    printk(KERN_INFO "%s", __FUNCTION__);
}

static int usb_control_reset_resume(struct usb_interface *interface)
{
    printk(KERN_INFO "%s", __FUNCTION__);
}

static int usb_control_pre_reset(struct usb_interface *interface)
{
    printk(KERN_INFO "%s", __FUNCTION__);
}

static int usb_control_post_reset(struct usb_interface *interface)
{
    printk(KERN_INFO "%s", __FUNCTION__);
}

static struct usb_device_id usb_control_table[] =
{
    { USB_DEVICE(0x058F, 0x6387) },
    {} /* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, usb_control_table);

static struct usb_driver usb_driver =
{
    .name         = "usb_driver",
    .id_table     = usb_control_table,
    .suspend      = usb_control_suspend,
    .resume       = usb_control_resume,
    .pre_reset    = usb_control_pre_reset,
    .post_reset   = usb_control_post_reset,
    .reset_resume = usb_control_reset_resume,
    .probe        = usb_control_probe,
    .disconnect   = usb_control_disconnect,
};

static int __init usb_control_init(void)
{
    return usb_register(&usb_driver);
}

static void __exit usb_control_exit(void)
{
    usb_deregister(&usb_driver);
}

module_init(usb_control_init);
module_exit(usb_control_exit);
