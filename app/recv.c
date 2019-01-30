#include <sys/socket.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define ETH_P_GOOSE 0x88b8
/***************************************************************
 * @Function: dump_hex
 * @Arguments:
 *  fp:  file to dump to
 *  buf: buffer to dump
 *  len: length to dump
 * @Returns:void
 * @Description: Dumps buffer to fp in an easy-to-read farmat
 * ************************************************************/
void dump_hex(FILE *fp, unsigned char const *buf, int len)
{
	int i;
	int base;

	if (!fp)
		return;

	for (base = 0; base < len; base += 16) {
		for (i = base; i < base + 16; i++) {
			if (i < len) {
				fprintf(fp, "%02x ", (unsigned)buf[i]);
			} else {
				fprintf(fp, "	");
			}
		}
		fprintf(fp, "	");
		for (i = base; i < base + 16; i++) {
			if (i < len) {
				if (isprint(buf[i])) {
					fprintf(fp, "%c", buf[i]);
				} else {
					fprintf(fp, ".");
				}
			} else {
				break;
			}
		}
		fprintf(fp, "\n");
	}
}

/*
 * set or unset promisc mode for ethernet specified bu ifname.
 * sockfd: socket file descriptor to set.
 * ifname: ether name to set.
 * flag: 0 indicate unset, 1 indicates set.
 */
int eth_promisc(int sockfd, char *ifname, int flag)
{
	struct ifreq ethreq;

	strncpy(ethreq.ifr_name, ifname, IFNAMSIZ);

	if (-1 == ioctl(sockfd, SIOCGIFFLAGS, &ethreq)) {
		perror("ioctl");
		return -1;
	}

	if (flag)
		ethreq.ifr_flags |= IFF_PROMISC;
	else
		ethreq.ifr_flags &= ~(IFF_PROMISC);

	if (-1 == ioctl(sockfd, SIOCSIFFLAGS, &ethreq)) {
		perror("ioctl");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int sockfd = -1;
	int len = 0;
	int rv;
	struct sockaddr_ll sll;
	unsigned char buf[1024] = {0x0};
	unsigned char src[18] = "";
	unsigned char dst[18] = "";


	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (socket < 0) {
		perror("socket:");
		return -1;
	}

	if (argc > 1)
		rv = eth_promisc(sockfd, argv[1], 1);
	else
		rv = eth_promisc(sockfd, "eth0", 1);
	if (rv) {
		close(sockfd);
		return -1;
	}

	/* bind */
	memset(&sll, 0x0, sizeof(struct sockaddr_ll));
	sll.sll_family = PF_PACKET;
	sll.sll_protocol = htons(ETH_P_GOOSE);
	sll.sll_ifindex = if_nametoindex("eth0");
	sll.sll_hatype = ARPHRD_ETHER;
	sll.sll_pkttype = PACKET_OTHERHOST;
	sll.sll_halen = ETH_ALEN;
	if (bind(sockfd, (struct sockaddr *)&sll, sizeof(struct sockaddr_ll))) {
		perror("bind:");
		return -1;
	}

	bool dump = false;
	while (1) {
		len = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
		if (len > 0) {
			sprintf(dst, "%02x:%02x:%02x:%02x:%02x:%02x", 
					buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
			sprintf(src, "%02x:%02x:%02x:%02x:%02x:%02x", 
					buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);

			if (buf[12] == 0x08 && buf[13] == 0x00) {
				//continue;
				printf("received IP packet:\n");
			} else if (buf[12] == 0x08 && buf[13] == 0x06) {
				//continue;
				printf("received ARP packet:\n");
			} else if (buf[12] == 0x08 && buf[13] == 0x35) {
				//continue;
				printf("received RPRP packet:\n");
			} else if (buf[12] == 0x81 && buf[13] == 0x00) {		/* virtual LAN */
				if (buf[16] == 0x88 && buf[17] == 0xb8) {
					printf("received GOOSE packet:\n");
					dump = true;
				}
			} else if (buf[12] == 0x88 && buf[13] == 0xb8) {
				printf("received GOOSE packet:\n");
				dump = true;
			} else {
				//continue;
				printf("received packet UNKNOW typea\n:");
			}

			printf("SRC MAC:%s DST MAC:%s\n", src, dst);

			if (dump) {
				dump_hex(stdout, buf, len);
				dump = false;
			}
		}
		if (len < 0) {
			perror("recvfrom:");
			break;
		}
	}

	close(sockfd);
	return 0;
}


