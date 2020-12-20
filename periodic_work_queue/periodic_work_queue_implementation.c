#include <linux/workqueue.h>
#include <linux/spinlock.h>

#define DELAY_IN_MILLISECONDS 10000 // ~10 seconds

DEFINE_SPINLOCK(g_spinlock);
static bool g_enabled = false;
static int g_delay_in_milliseconds = DELAY_IN_MILLISECONDS;

extern void pwq_work_handler(struct work_struct *work);

DECLARE_DELAYED_WORK(g_pwq_work_item, pwq_work_handler);

void pwq_work_handler(struct work_struct *work)
{
    if(!g_enabled)
    {
       return;
    }

    spin_lock(&g_spinlock);
    printk("%s: work handler scheduled with delay %d\n", __FUNCTION__, g_delay_in_milliseconds);
    if(!schedule_delayed_work((struct delayed_work*)work, msecs_to_jiffies(g_delay_in_milliseconds)))
    {
        printk("%s: could not schedule delayed work item\n", __FUNCTION__);
    }
    spin_unlock(&g_spinlock);
    return;
}

int pwq_work_queue_init(void)
{
    if(g_enabled)
    {
       return -1;
    }
    g_enabled = true;

    return schedule_delayed_work(&g_pwq_work_item, msecs_to_jiffies(g_delay_in_milliseconds));
}

int pwq_work_queue_exit(void)
{
    if(!g_enabled)
    {
        return -1;
    }
    g_enabled = false;
    if(!cancel_delayed_work_sync(&g_pwq_work_item))
    {
          printk("%s: could not cancel work item\n", __FUNCTION__);
    }
    return 0;
}

int pwq_set_delay(int delay)
{
    spin_lock(&g_spinlock);
    g_delay_in_milliseconds = delay;
    spin_unlock(&g_spinlock);
    return 1;
}
