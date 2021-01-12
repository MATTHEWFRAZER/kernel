#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/spinlock.h>
#include <linux/socket.h>

MODULE_DESCRIPTION("example of a network_filter");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Frazer");

// spinlock so we can see the actual ordering of filtering
DEFINE_SPINLOCK(g_spinlock);

static unsigned int network_filter_handler_ipv4_in(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);

static unsigned int network_filter_handler_ipv4_out(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);

static unsigned int network_filter_handler_ipv6_in(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);

static unsigned int network_filter_handler_ipv6_out(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);

static unsigned int network_filter_handler_bt_in(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);

static unsigned int network_filter_handler_bt_out(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);

static unsigned int network_filter_handler_local_in(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);

static unsigned int network_filter_handler_local_out(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state);


// a good source https://programtip.com/en/art-101966

// Handler registering struct 1
static struct nf_hook_ops network_filter_hook_ipv4_in_ops __read_mostly = {
    .hook = network_filter_handler_ipv4_in,
    .pf = NFPROTO_IPV4,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

// Handler registering struct 2
static struct nf_hook_ops network_filter_hook_ipv4_out_ops __read_mostly = {
    .hook = network_filter_handler_ipv4_out,
    .pf = NFPROTO_IPV4,
    .hooknum = NF_INET_POST_ROUTING,
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

// Handler registering struct 3
static struct nf_hook_ops network_filter_hook_ipv6_in_ops __read_mostly = {
    .hook = network_filter_handler_ipv6_in,
    .pf = NFPROTO_IPV6,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

// Handler registering struct 4
static struct nf_hook_ops network_filter_hook_ipv6_out_ops __read_mostly = {
    .hook = network_filter_handler_ipv6_out,
    .pf = NFPROTO_IPV6,
    .hooknum = NF_INET_POST_ROUTING,
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

// Handler registering struct 5
static struct nf_hook_ops network_filter_hook_bt_in_ops __read_mostly = {
    .hook = network_filter_handler_bt_in,
    .pf = AF_BLUETOOTH,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

// Handler registering struct 6
static struct nf_hook_ops network_filter_hook_bt_out_ops __read_mostly = {
    .hook = network_filter_handler_bt_out,
    .pf = AF_BLUETOOTH,
    .hooknum = NF_INET_POST_ROUTING,
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

// Handler registering struct 7
static struct nf_hook_ops network_filter_hook_local_in_ops __read_mostly = {
    .hook = network_filter_handler_local_in,
    .pf = AF_LOCAL,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

// Handler registering struct 8
static struct nf_hook_ops network_filter_hook_local_out_ops __read_mostly = {
    .hook = network_filter_handler_local_out,
    .pf = AF_LOCAL,
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
		struct tcphdr *tcph;

    spin_lock(&g_spinlock);
		printk(KERN_NOTICE "%s: message %s", __FUNCTION__, message);

    if (!socket_buff)
    {
        printk(KERN_NOTICE "%s: accept NULL", __FUNCTION__);
        goto network_filter_exit;
    }

    iph = ip_hdr(socket_buff);

    if(iph == NULL)
		{
		    goto network_filter_exit;
		}

		printk(KERN_NOTICE "%s: protocol %d", __FUNCTION__, iph->protocol);
	  if (iph->protocol == IPPROTO_UDP)
    {
		    udph = udp_hdr(socket_buff);

				if(udph == NULL)
				{
				    goto network_filter_exit;
				}

		    if (ntohs(udph->dest) == 53)
        {
             printk(KERN_NOTICE "%s: accept, upd, DNS", __FUNCTION__);
			       goto network_filter_exit;
		    }
	  }
	  else if (iph->protocol == IPPROTO_TCP)
    {
		    tcph = tcp_hdr(socket_buff);
				if(tcph == NULL)
				{
				    goto network_filter_exit;
				}

        printk(KERN_NOTICE "%s: accept, tcp, dest %d", __FUNCTION__, ntohs(tcph->dest));
		    goto network_filter_exit;
	  }

network_filter_exit:
    spin_unlock(&g_spinlock);
	  return NF_ACCEPT;
}

static unsigned int network_filter_handler_ipv4_in(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
     return network_filter_handler_inner(priv, socket_buff, state, "ipv4 pre");
}

static unsigned int network_filter_handler_ipv4_out(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
    return network_filter_handler_inner(priv, socket_buff, state, "ipv4 post");
}

static unsigned int network_filter_handler_ipv6_in(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
     return network_filter_handler_inner(priv, socket_buff, state, "ipv6 pre");
}

static unsigned int network_filter_handler_ipv6_out(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
    return network_filter_handler_inner(priv, socket_buff, state, "ipv6 post");
}

static unsigned int network_filter_handler_bt_in(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
	   printk(KERN_NOTICE "bluetooth in\n");
     return network_filter_handler_inner(priv, socket_buff, state, "ipv6 pre");
}

static unsigned int network_filter_handler_bt_out(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
	  printk(KERN_NOTICE "bluetooth out\n");
    return network_filter_handler_inner(priv, socket_buff, state, "ipv6 post");
}

static unsigned int network_filter_handler_local_in(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
	   printk(KERN_NOTICE "local in\n");
     return network_filter_handler_inner(priv, socket_buff, state, "ipv6 pre");
}

static unsigned int network_filter_handler_local_out(void *priv,
			       struct sk_buff *socket_buff,
			       const struct nf_hook_state *state)
{
	  printk(KERN_NOTICE "local out\n");
    return network_filter_handler_inner(priv, socket_buff, state, "ipv6 post");
}


static int __init network_filter_init(void)
{
	  nf_register_net_hook(&init_net, &network_filter_hook_ipv4_in_ops);
    nf_register_net_hook(&init_net, &network_filter_hook_ipv4_out_ops);
    nf_register_net_hook(&init_net, &network_filter_hook_ipv6_in_ops);
    nf_register_net_hook(&init_net, &network_filter_hook_ipv6_out_ops);
		nf_register_net_hook(&init_net, &network_filter_hook_bt_in_ops);
    nf_register_net_hook(&init_net, &network_filter_hook_bt_out_ops);
		nf_register_net_hook(&init_net, &network_filter_hook_local_in_ops);
    nf_register_net_hook(&init_net, &network_filter_hook_local_out_ops);
    return 0;
}

static void __exit network_filter_exit(void)
{
    nf_unregister_net_hook(&init_net, &network_filter_hook_ipv4_in_ops);
    nf_unregister_net_hook(&init_net, &network_filter_hook_ipv4_out_ops);
    nf_unregister_net_hook(&init_net, &network_filter_hook_ipv6_in_ops);
    nf_unregister_net_hook(&init_net, &network_filter_hook_ipv6_out_ops);
		nf_unregister_net_hook(&init_net, &network_filter_hook_bt_in_ops);
    nf_unregister_net_hook(&init_net, &network_filter_hook_bt_out_ops);
		nf_unregister_net_hook(&init_net, &network_filter_hook_local_in_ops);
    nf_unregister_net_hook(&init_net, &network_filter_hook_local_out_ops);
}

module_init(network_filter_init);
module_exit(network_filter_exit);
