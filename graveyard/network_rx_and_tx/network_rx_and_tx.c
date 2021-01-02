#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/kernel.h> // printk()
#include <linux/slab.h> // kmalloc

struct network_rx_and_tx_rx_private
{
	struct net_device_stats stats;
	int status;
	struct snull_packet *ppool;
	struct snull_packet *rx_queue;  /* List of incoming packets */
	int rx_int_enabled;
	int tx_packetlen;
	u8 *tx_packetdata;
	struct sk_buff *skb;
	spinlock_t lock;
	struct napi_struct napi;
	struct net_device *dev;
};

static void network_rx_and_tx_rx(struct net_device *network_device, int len, unsigned char *buf)
{
    struct sk_buff *socket_buffer;
    struct network_rx_and_tx_private *private_data = (struct network_rx_and_tx_private *) network_device->priv;
    char *p_extra_data;

    // The packet has been retrieved from the transmission
    // medium. Build an skb around it, so upper layers can handle it
    socket_buffer = dev_alloc_skb(len+2);
    if (!socket_buffer)
    {
        printk("%s: low on mem - packet dropped\n", __FUNCTION__);
        private_data->stats.rx_dropped++;
        return;
    }

    // get extra data portion of socket buffer
    p_extra_data = skb_put(socket_buffer, len);

    // copy to extra data portion of socket buffer
    memcpy(p_extra_data, buf, len);

    // Write metadata, and then pass to the receive level
    socket_buffer->dev = network_device;
    socket_buffer->protocol = eth_type_trans(socket_buffer, dev);
    socket_buffer->ip_summed = CHECKSUM_UNNECESSARY; // don't check it
    private_data->stats.rx_packets++;
    private_data->stats.rx_bytes += len;

    if(netif_rx(socket_buffer) == NET_RX_DROP)
    {
          printk("%s: packet dropped at a higher layer\n", __FUNCTION__);
    }

    return;
}

static irqreturn_t igb_intr_msi(int irq, void *data)
{
	struct igb_adapter *adapter = data;
	struct igb_q_vector *q_vector = adapter->q_vector[0];
	struct e1000_hw *hw = &adapter->hw;
	/* read ICR disables interrupts using IAM */
	u32 icr = rd32(E1000_ICR);

	igb_write_itr(q_vector);

	if (icr & E1000_ICR_DRSTA)
		schedule_work(&adapter->reset_task);

	if (icr & E1000_ICR_DOUTSYNC) {
		/* HW is reporting DMA is out of sync */
		adapter->stats.doosync++;
	}

	if (icr & (E1000_ICR_RXSEQ | E1000_ICR_LSC)) {
		hw->mac.get_link_status = 1;
		if (!test_bit(__IGB_DOWN, &adapter->state))
			mod_timer(&adapter->watchdog_timer, jiffies + 1);
	}

	if (icr & E1000_ICR_TS)
		igb_tsync_interrupt(adapter);

	  napi_schedule(&q_vector->napi);

    return IRQ_HANDLED;
}
