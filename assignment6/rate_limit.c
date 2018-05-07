/* Part 1 - Header files and module setup */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netdevice.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <linux/netfilter_ipv4.h>
/* Time manangement */
#include <asm/param.h> // HZ
#include <linux/jiffies.h> // jiffies and time_after

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Peter Chung");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("A packet flow limit module");

static int port = 80;
MODULE_PARM_DESC(port, "Port number to limit, default is port 80");
module_param(port, int, 0644);

static int rate = 2000;
MODULE_PARM_DESC(rate, "Target rate in bytes/ms, default(min) is 2000");
module_param(rate, int, 0644);

static struct nf_hook_ops nfho_incoming;  // Hook struct at pre-routing 

/* Part 2 - Flow struct defintion */
struct Flow {
	unsigned int local_ip;
	unsigned int remote_ip;
	unsigned short int local_port;
	unsigned short int remote_port;
	// statistics
	unsigned int pktcount, dropcount, bytecount, bytedropcount, buf;
	unsigned long time, start;
};
// SIZE = 1 << 12 = 4096
#define SIZE (1<<12)
static struct Flow flist[SIZE];

static inline unsigned int hash(struct Flow* f) {
    // a simple hash function, given a Flow struct (non-NULL), return a value in [0,SIZE-1]=1
    return ( (f->local_ip%SIZE+1) * (f->remote_ip%SIZE+1) * ( f->local_port%SIZE+1) * (f->remote_port%SIZE+1) )%SIZE;
}

static inline void reset_flow(struct Flow* f) {
    // assumption: f is non-NULL
    f->local_port = f->remote_port = 0;
    f->local_ip = f->remote_ip = 0;
    f->pktcount=f->dropcount=f->bytecount=f->bytedropcount=0;
    f->buf = rate;
    f->time=f->start=0;
}

/* Part 3 - The hook function */
static unsigned int hook_func_in(unsigned int hooknum, struct sk_buff *skb, 
                            const struct net_device *in, const struct net_device *out, 
                          int (*okfn)(struct sk_buff *)) {

    struct iphdr *ip_header;          // IP header structure
    struct tcphdr *tcp_header;        // TCP header structure
    struct ethhdr *eth_header;        // Ethernet Header
    unsigned int sIP, dIP;	      // IP addresses
    unsigned int sport, dport;        // Source and Destination port

    struct Flow f ;                   // a local flow struct 
    struct Flow *flow;                // a pointer (to be pointed)
    unsigned int payload_len;	      // TCP payload length

    // Get the eth_header by pointer arthematics
    eth_header = (struct ethhdr *)skb_mac_header(skb); 

    // The packet does not have an ethernet header
    if(!eth_header || (eth_header && eth_header->h_proto != __constant_htons(ETH_P_IP))) {
	//printk(KERN_INFO "[ACCEPT] Other packets\n");
        return NF_ACCEPT;
    }

    // Get the ip_header by pointer arthematics
    ip_header=(struct iphdr *)skb_network_header(skb);

    // The packet is not a IP packet 
    if (!ip_header) {
	//printk(KERN_INFO "[ACCEPT] Other packets\n");
        return NF_ACCEPT;
    }

    // A TCP packet
    if(ip_header->protocol==IPPROTO_TCP) {
	tcp_header = (struct tcphdr *)((__u32 *)ip_header+ ip_header->ihl);
	sIP = (unsigned int) ip_header->saddr;
	dIP = (unsigned int) ip_header->daddr;
	sport = ntohs((unsigned short int) tcp_header->source);
	dport = ntohs((unsigned short int) tcp_header->dest); 
 	if( sport != port && dport != port) {
	    //printk(KERN_INFO "[ACCEPT] TCP - Src:%pI4:%d, Dest:%pI4:%d\n", &sIP, sport, &dIP, dport);
            return NF_ACCEPT;
	}

	// Calculate the payload length
        payload_len= (unsigned int)ntohs(ip_header->tot_len)
                                -(ip_header->ihl<<2)
                                -(tcp_header->doff<<2);


        // reset a local flow struct
        // Note: source and estination should be changed
        reset_flow(&f);
	f.local_ip = dIP;
	f.remote_ip = sIP;
	f.local_port = dport;
	f.remote_port = sport;

	// Hash to find the flow index within the global array
	flow = &flist[ hash(&f) ];
	if ( tcp_header->fin) {
		printk(KERN_INFO "[Finish rate=%d] t=%lu ms, receive/drop(bytes): %d/%d\n", rate, 1000 * (jiffies - flow->start) / HZ, flow->bytecount, flow->bytedropcount);
		reset_flow(flow);
	}
	if (payload_len <= 0)
		return NF_ACCEPT;

	// Set the start time
	if (flow->start == 0) {
		flow->start = jiffies;
		flow->time = jiffies;
	}
	
	// Update buf and time
	flow->buf += 1000 * (jiffies - flow->time) / HZ * rate;
	flow->time = jiffies;

	// Accept pkt if payload_len < buf, else drop pkt
	if (payload_len <= flow->buf) {
		flow->buf -= payload_len;
		flow->pktcount++;
		flow->bytecount += payload_len;
		return NF_ACCEPT;
	}
	else {
		flow->bytedropcount += payload_len;
		flow->dropcount++;
		return NF_DROP;
	}
    }
    //printk(KERN_INFO "[ACCEPT] TCP \n");
    return NF_ACCEPT;
}

/* Part 4 - Initialize and clean up the module */
int init_module(void) {
	int i;
	for (i=0; i<SIZE; i++)
		reset_flow(&flist[i]);

	nfho_incoming.hook=hook_func_in; 			// Setup the hook function
	nfho_incoming.hooknum= NF_INET_PRE_ROUTING;		// pre-routing
    	nfho_incoming.pf = PF_INET;				// IPV4 packets
    	nfho_incoming.priority = NF_IP_PRI_FIRST;		// set to the highest priority 
    	nf_register_hook(&nfho_incoming);			// register hook
    	printk(KERN_INFO "[Init] Packet Flow Limit Module at port %d\n", port);
    	return 0;
}

void cleanup_module(void) {
    nf_unregister_hook(&nfho_incoming);
    printk(KERN_INFO "[Exit] Packet Flow Limit Module\n");
}



