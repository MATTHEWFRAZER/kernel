#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/dma-mapping.h>

MODULE_DESCRIPTION("example of a network driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Frazer");

#define FIRST_MINOR 0
#define MINOR_CNT 1

static const char g_device_name[] = "network_module";

static const struct pci_device_id g_network_pci_table[] = {
    { //PCI_VDEVICE(PCI_ANY_ID, PCI_ANY_ID), NULL // this as a similar example except no subclass or class, which was needed below
        PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_BASE_CLASS_NETWORK, 0, NULL
    },
    /* required last entry */
   {0, }
};

MODULE_DEVICE_TABLE(pci, network_pci_table);

static pci_ers_result_t network_io_error_detected(struct pci_dev *pdev, pci_channel_state_t state);
static int network_probe(struct pci_dev *pci_device, const struct pci_device_id *ent);

static const struct pci_error_handlers g_network_error_handler = {
    .error_detected = network_io_error_detected,
    .slot_reset = igb_io_slot_reset,
    .resume = igb_io_resume,
};

static struct pci_driver network_pci_driver = {
    .name     = g_device_name,
    .id_table = g_network_pci_table,
    .probe    = network_probe,
    .remove   = network_remove,

    .shutdown = ,
	//.sriov_configure = igb_pci_sriov_configure,
    .err_handler = &g_network_error_handler
};

/*
// Mask off interrupt generation on the NIC
static void igb_irq_disable(struct igb_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;

	 // we need to be careful when disabling interrupts.  The VFs are also
	 // mapped into these registers and so clearing the bits can cause
	 // issues on the VF drivers so we only need to clear what we set
	 //
	if (adapter->flags & IGB_FLAG_HAS_MSIX) {
    // read u32 from E1000_EIAM register (device specific)
		u32 regval = rd32(E1000_EIAM);

    // write to E1000_EIAM register (device specific)
		wr32(E1000_EIAM, regval & ~adapter->eims_enable_mask);
		wr32(E1000_EIMC, adapter->eims_enable_mask);
		regval = rd32(E1000_EIAC);
		wr32(E1000_EIAC, regval & ~adapter->eims_enable_mask);
	}

	wr32(E1000_IAM, 0);
	wr32(E1000_IMC, ~0);
	wrfl();
	if (adapter->flags & IGB_FLAG_HAS_MSIX) {
		int i;

		for (i = 0; i < adapter->num_q_vectors; i++)
    {
      // wait for other irq handlers (on other cpus)
			synchronize_irq(adapter->msix_entries[i].vector);
    }
	} else {
    // wait for other irq handlers (on other cpus)
		synchronize_irq(adapter->pdev->irq);
	}
}*/

static void network_down(struct igb_adapter *adapter)
{
    struct net_device *netdev = adapter->netdev;
    struct e1000_hw *hw = &adapter->hw;
    u32 tctl, rctl;
    int i;

    // signal that we're down so the interrupt handler does not
    //reschedule our watchdog timer
    set_bit(__IGB_DOWN, &adapter->state);

	/* disable receives in the hardware */
	//rctl = rd32(E1000_RCTL);
	//wr32(E1000_RCTL, rctl & ~E1000_RCTL_EN);
	/* flush and sleep below */

	//igb_nfc_filter_exit(adapter);

    // device has detected a loss of carrier (clear carrier)
    netif_carrier_off(netdev);

    // prevents any transmission
    netif_tx_stop_all_queues(netdev);

	/* disable transmits in the hardware */
	//tctl = rd32(E1000_TCTL);
	//tctl &= ~E1000_TCTL_EN;
	//wr32(E1000_TCTL, tctl);
	/* flush both disables and wait for them to finish */
//	wrfl();
	//usleep_range(10000, 11000);

	igb_irq_disable(adapter);

	adapter->flags &= ~IGB_FLAG_NEED_LINK_UPDATE;

	for (i = 0; i < adapter->num_q_vectors; i++) {
		if (adapter->q_vector[i]) {
			napi_synchronize(&adapter->q_vector[i]->napi);
			napi_disable(&adapter->q_vector[i]->napi);
		}
	}

	del_timer_sync(&adapter->watchdog_timer);
	del_timer_sync(&adapter->phy_info_timer);

	/* record the stats before reset*/
	spin_lock(&adapter->stats64_lock);
	igb_update_stats(adapter);
	spin_unlock(&adapter->stats64_lock);

	adapter->link_speed = 0;
	adapter->link_duplex = 0;

	if (!pci_channel_offline(adapter->pdev))
		igb_reset(adapter);

	/* clear VLAN promisc flag so VFTA will be updated if necessary */
	adapter->flags &= ~IGB_FLAG_VLAN_PROMISC;

	igb_clean_all_tx_rings(adapter);
	igb_clean_all_rx_rings(adapter);
}

static pci_ers_result_t network_io_error_detected(struct pci_dev *pdev, pci_channel_state_t state)
{
    struct net_device *netdev = pci_get_drvdata(pdev);
    struct igb_adapter *adapter = netdev_priv(netdev);

    // detach device
    netif_device_detach(netdev);

    if (state == pci_channel_io_perm_failure)
    {
        return PCI_ERS_RESULT_DISCONNECT;
    }

    // test if up
    if (netif_running(netdev))
    {
        // force down
        network_down(adapter);
    }

    pci_disable_device(pdev);

    // request a slot slot reset
    return PCI_ERS_RESULT_NEED_RESET;
}


static int network_probe(struct pci_dev *pci_device, const struct pci_device_id *ent)
{
    int ret = 0;
    printk(KERN_NOTICE "%s(): %s (%hx:%hx)\n", __FUNCTION__, pci_name(pci_device), pci_device->vendor, pci_device->device);

    if (pdev->is_virtfn)
    {
       printk(KERN_ERR "%s(): %s (%hx:%hx) should not be a virtual function!\n", __FUNCTION__, pci_name(pci_device), pci_device->vendor, pci_device->device);
       return -EINVAL;
    }

    // set DMA mask, inform the kernel of the device's DMA addressnig capabilities
    // could be a call to dma_set_mask to set streaming mappings and then dma_set_coherent_mask to setup consistent allocations
    if(dma_set_mask_and_coherent(pci_device->dev, DMA_BIT_MASK(64)))
    {
        ret = dma_set_mask_and_coherent(pci_device->dev, DMA_BIT_MASK(32));
        if(ret)
        {
            goto error_network_probe;
        }
    }

    // reserves selected PCI I/O and memory resources
    // -- calls pci_request_selected_regions with the bars agument with a call to  pci_select_bars(pdev, IORESOURCE_MEM)
    ret = pci_request_mem_regions(pci_device, g_device_name);
	  if (ret)
    {
        goto error_network_probe;
    }

    // make the driver aer aware - will generate an interrupt if an error is detected
    pci_enable_pcie_error_reporting(pci_device);

    // enables bus mastering for device dev (communicate directly with device without CPU)
    pci_set_master(pci_device);

    // save PCI configuration space of device before suspending
    pci_save_state(pci_device);

error_network_probe:
    // disable pic device
    pci_disable_device(pci_device);
    return ret;
}
