#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/spinlock.h>

MODULE_DESCRIPTION("example of a network_filter");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Frazer");

// spinlock so we can see the actual ordering of filtering
DEFINE_SPINLOCK(g_spinlock);

static unsigned int network_filter_handler_ipv4_pre(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);

static unsigned int network_filter_handler_ipv4_post(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);

static unsigned int network_filter_handler_ipv6_pre(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);

static unsigned int network_filter_handler_ipv6_post(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);

// Handler registering struct 1
static struct nf_hook_ops network_filter_hook_ipv4_pre_ops __read_mostly = {
    .hook = network_filter_handler_ipv4_pre,
    .pf = NFPROTO_IPV4,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

// Handler registering struct 2
static struct nf_hook_ops network_filter_hook_ipv4_post_ops __read_mostly = {
    .hook = network_filter_handler_ipv4_post,
    .pf = NFPROTO_IPV4,
    .hooknum = NF_INET_POST_ROUTING,
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

// Handler registering struct 3
static struct nf_hook_ops network_filter_hook_ipv6_pre_ops __read_mostly = {
    .hook = network_filter_handler_ipv6_pre,
    .pf = NFPROTO_IPV6,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

// Handler registering struct 4
static struct nf_hook_ops network_filter_hook_ipv6_post_ops __read_mostly = {
    .hook = network_filter_handler_ipv6_post,
    .pf = NFPROTO_IPV6,
    .hooknum = NF_INET_POST_ROUTING,
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

// Handler function
static unsigned int network_filter_handler_inner(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state,
						 char *message
					   )
{
    struct iphdr *iph;
    struct udphdr *udph;

    spin_lock(&g_spinlock);
		printk(KERN_NOTICE "%s: message %s", __FUNCTION__, message);

    if (!socket_buff)
    {
        printk(KERN_NOTICE "%s: accept NULL", __FUNCTION__);
        goto network_filter_exit;
    }

    iph = ip_hdr(socket_buff);
	  if (iph->protocol == IPPROTO_UDP)
    {
		    udph = udp_hdr(socket_buff);
		    if (ntohs(udph->dest) == 53)
        {
             printk(KERN_NOTICE "%s: accept, upd, DNS", __FUNCTION__);
			       goto network_filter_exit;
		    }
	  }
	  else if (iph->protocol == IPPROTO_TCP)
    {
        printk(KERN_NOTICE "%s: accept, tcp", __FUNCTION__);
		    goto network_filter_exit;
	  }

network_filter_exit:
    spin_unlock(&g_spinlock);
	  return NF_ACCEPT;
}

static unsigned int network_filter_handler_ipv4_pre(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
     return network_filter_handler_inner(priv, socket_buff, state, "ipv4 pre");
}

static unsigned int network_filter_handler_ipv4_post(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
    return network_filter_handler_inner(priv, socket_buff, state, "ipv4 post");
}

static unsigned int network_filter_handler_ipv6_pre(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
     return network_filter_handler_inner(priv, socket_buff, state, "ipv6 pre");
}

static unsigned int network_filter_handler_ipv6_post(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
    return network_filter_handler_inner(priv, socket_buff, state, "ipv6 post");
}


static int __init network_filter_init(void)
{
	  nf_register_net_hook(&init_net, &network_filter_hook_ipv4_pre_ops);
    nf_register_net_hook(&init_net, &network_filter_hook_ipv4_post_ops);
    nf_register_net_hook(&init_net, &network_filter_hook_ipv6_pre_ops);
    nf_register_net_hook(&init_net, &network_filter_hook_ipv6_post_ops);
    return 0;
}

static void __exit network_filter_exit(void)
{
    nf_unregister_net_hook(&init_net, &network_filter_hook_ipv4_pre_ops);
    nf_unregister_net_hook(&init_net, &network_filter_hook_ipv4_post_ops);
    nf_unregister_net_hook(&init_net, &network_filter_hook_ipv6_pre_ops);
    nf_unregister_net_hook(&init_net, &network_filter_hook_ipv6_post_ops);
}

module_init(network_filter_init);
module_exit(network_filter_exit);
