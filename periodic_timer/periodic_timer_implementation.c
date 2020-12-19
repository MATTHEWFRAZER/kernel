#include <linux/timer.h>
#include <linux/spinlock.h>

struct timer_list g_periodic_timer;
static int g_expiration_time = 10000;
static int g_initialized = 0;
DEFINE_SPINLOCK(g_spinlock);

static void periodic_timer_callback(struct timer_list *timer)
{
    spin_lock(&g_spinlock);
    printk(KERN_NOTICE "%s: timer triggered, expiration %d\n", __FUNCTION__, g_expiration_time);
    if(mod_timer(&g_periodic_timer, msecs_to_jiffies(g_expiration_time)))
    {
        printk(KERN_NOTICE "%s: timer was already active\n", __FUNCTION__);
    }
    spin_unlock(&g_spinlock);
}

int pt_timer_init(void)
{
    if(g_initialized)
    {
        return 0;
    }

    g_initialized = 1;

    timer_setup(&g_periodic_timer, periodic_timer_callback, 0);

    if(mod_timer(&g_periodic_timer, msecs_to_jiffies(g_expiration_time)))
    {
        printk(KERN_NOTICE "%s: timer was already active\n", __FUNCTION__);
    }

    return 1;
}

int pt_timer_exit(void)
{
    if(!g_initialized)
    {
        return 0;
    }
    g_initialized = 0;
    del_timer_sync(&g_periodic_timer);
    return 1;
}

int pt_set_timer(int expiration_time)
{
    spin_lock(&g_spinlock);
    g_expiration_time = expiration_time;
    spin_unlock(&g_spinlock);
    return 1;
}
