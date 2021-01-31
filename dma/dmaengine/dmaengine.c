#include <linux/dmaengine.h>
#include <linux/completion.h>
#include <linux/dma-mapping.h>

// https://blog.idv-tech.com/wp-content/uploads/2014/09/drivers-session4-dma-4public.pdf

void dma_async_tx_callback(void *param)
{
   struct completion *cmp = (struct completion *)param;
   if (cmp == NULL)
   {
     return;
   }

   // signal done
   cpm->done = 1;
}

void setup_dma(struct device *dev)
{
    struct completion cmp;
    unsigned long timeout = msecs_to_jiffies(3000);
    enum dma_status status;
    struct dma_chan* chan;
    dma_cap_mask_t mask;
    dma_addr_t dma_buffer;

    // request channel
    dma_cap_zero(mask);
    // DMA_SLAVE is where software initiates the DMA transactions to the DMA
    // controller hardware rather than the device with integrated DMA intiating
    // DMA_PRIVATE means other channels will not get in our way
    dma_cap_set(DMA_SLAVE | DMA_PRIVATE, mask);
    chan = dma_request_channel(mask, NULL, NULL);

    struct dma_async_tx_descriptor chan_desc;
    // DMA_CTRL_ACK initializes the descriptor indicating the client (us) owns it
    // DMA_PREP_INTERRUPT is used to cause an interrupt on completion (which calls our callback)
    enum dma_ctrl_flags flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
    char *buf = kmalloc(1024, GFP_KERNEL);

    // map memory for dma
    dma_buffer = dma_map_single(device, buf, 1024, DMA_TO_DEVICE);

    // prep slave for single transaction
    chan_desc = dmaengine_prep_slave_single(chan, dma_buffer, 1024, DMA_MEM_TO_DEV , flags);

    if (chan_desc == NULL)
    {
       return;
    }

    // set callback on completion (use completion as param)
    chan_desc->callback = dma_async_tx_callback;
    chan_desc->callback_param = cmp;

    // queue up transaction
    dma_cookie_t cookie = dmaengine_submit(chan_desc);

    // initialize completion struct
    init_completion(&cmp);

    // flushes pending transactions to hardware (cause the DMA engine to actually start queued up work)
    dma_async_issue_pending(chan);

    // wait untill completion to finish or timeout
    timeout = wait_for_completion_timeout(&cmp, timeout);

    // get status of transaction
    status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);

    if (timeout == 0)
    {
    // timeout processing
    }
    else if (status != DMA_COMPLETE)
    {
        if (status == DMA_ERROR)
        {
          // error processing
        }
    }

    if(chan != NULL)
    {
        dma_release_channel(chan);
    }

    if (!(device == NULL && dma_buffer == NULL))
    {
        dma_unmap_single(device, dma_buffer 1024, DMA_TO_DEVICE);
    }

    if (buf != NULL)
    {
        kfree(buf);
    }
}
