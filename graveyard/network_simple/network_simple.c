#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/kernel.h> // printk()
#include <linux/slab.h> // kmalloc

MODULE_DESCRIPTION("example of a network driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Frazer");

#define FIRST_MINOR 0
#define MINOR_CNT 1

static const char g_device_name[] = "network_simple_module";

static struct net_device *g_network_device;
static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

static const struct file_operations g_network_simple_file_operations =
{
    .owner = THIS_MODULE,
};

int network_simple_tx(struct sk_buff *skb, struct net_device *network_device)
{
    int len;
    char *data, shortpkt[ETH_ZLEN];
    struct network_simple_private_data *private_data = netdev_priv(network_device);

    printk(KERN_NOTICE "%s: transmit\n", __FUNCTION__);

    data = skb->data;
    len = skb->len;
    if (len < ETH_ZLEN)
    {
        memset(shortpkt, 0, ETH_ZLEN);
        memcpy(shortpkt, skb->data, skb->len);
        len = ETH_ZLEN;
        data = shortpkt;
    }

    // save the timestamp
    network_device->trans_start = jiffies;

    // Remember the skb, so we can free it at interrupt time
    priv->skb = skb;

    // actual deliver of data is device-specific, and not shown here
    snull_hw_tx(data, len, dev);

    // Our simple device can not fail
    return 0;
}

static int __init network_simple_init(void)
{
    int result;
    struct device *dev_ret;

    printk(KERN_NOTICE "%s: network driver init\n", __FUNCTION__);

    if ((result = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "periodic_timer_module_ioctl")) < 0)
    {
       return result;
    }

    cdev_init(&c_dev, &g_periodic_timer_file_operations);

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

    ether_setup(g_network_device); // assign some of the fields

    g_network_device->open            = snull_open;
    g_network_device->stop            = snull_release;
    g_network_device->set_config      = snull_config;
    g_network_device->hard_start_xmit = network_simple_tx;
    g_network_device->get_stats       = snull_stats;
    g_network_device->rebuild_header  = snull_rebuild_header;
    g_network_device->hard_header     = snull_header;
    g_network_device->tx_timeout      = snull_tx_timeout;
    g_network_device->watchdog_timeot = timeout;
    // keep the default flags, just add NOARP
    g_network_device->flags           |= IFF_NOARP;
    g_network_device->features        |= NETIF_F_NO_CSUM;
    g_network_device->hard_header_cache = NULL;      // Disable caching
}
