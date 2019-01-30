#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netdevice.h>	/* for struct packet_type */
#include <linux/skbuff.h>
#include "hello.h"

static int 
hello_recv(struct sk_buff *skb, struct net_device *dev,
		struct packet_type *pt, struct net_device *orin_dev)
{
	int i = 0;

	printk("recv package:\n");
	for (i = 0; i < skb->len; i++)
		printk("0x%x ", skb->data[i]);
	printk("\n");
	return 0;
}

static struct packet_type hello_packet_type = {
	//.type = ntohs(ETH_P_IP),
	.type = ntohs(ETH_P_ALL),
	.dev = NULL,
	.func = hello_recv
};

int __init hello_init(void)
{
	dev_add_pack(&hello_packet_type);

	printk("hello_init!\n");
	return 0;
}

void __exit hello_exit(void)
{
	dev_remove_pack(&hello_packet_type);
	printk("hello_exit!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shad Chen");
MODULE_DESCRIPTION("Generary net test module");
MODULE_VERSION("0.0.1");

