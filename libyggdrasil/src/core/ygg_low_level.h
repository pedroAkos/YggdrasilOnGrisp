#ifndef YGG_LL_RTEMS_H_
#define YGG_LL_RTEMS_H_

//linux libs
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <inttypes.h>
#include <limits.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <sys/ioctl.h>

 #include <ifaddrs.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <net/bpf.h>

#define makedev(x,y) rtems_filesystem_make_dev_t(x,y)

#ifdef __64BIT__
#define domakedev makedev64
#else /* __64BIT__ */
#define domakedev makedev
#endif /* __64BIT__ */


#ifdef AF_PACKET
#  include <netpacket/packet.h>
#endif


#ifdef __linux__
# include <asm/types.h> /* needed for 2.4 kernels for the below header */
# include <linux/filter.h>
# include <linux/if_packet.h>
#define bpf_insn sock_filter
#define SO_ACCEPTFILTER SO_ATTACH_FILTER

#endif

//RTEMS libs

#include <rtems.h>
//#include <rtems/ramdisk.h>
#include <rtems/diskdevs.h>

#include <machine/rtems-bsd-commands.h>



//defines
#define WLAN_ADDR_LEN 6
#define YGG_HEADER_LEN 3

#define DEFAULT_MTU 1500
#define DEFAULT_FREQ 2412

#define CH_BROAD 0X001
#define CH_SENDTO 0x002
#define CH_SEND 0x003

#define IP_TYPE 0x3901
#define WLAN_BROADCAST "ff:ff:ff:ff:ff:ff"

#define AF_LK 0x4C4B50 //LKP
#define AF_LK_ARRAY {0x4C, 0x4B, 0x50}

#define AF_YGG 0x594747 //YGG
#define AF_YGG_ARRAY {0x59, 0x47, 0x47}

//error codes:
#define NO_IF_INDEX_ERR 1
#define NO_IF_ADDR_ERR 2
#define NO_IF_MTU_ERR 3

#define SOCK_BIND_ERR 4

#define CHANNEL_SENT_ERROR 5
#define CHANNEL_RECV_ERROR 6
#define CHANNEL_FILTER_ERROR 7

int lk_error_code;

//misc
#define SUCCESS 1
#define FAILED 0

/**********************************************************
 * Filters
 *********************************************************/

static const struct bpf_insn YGG_filter[] = {
   { .code=0x30, .jt=0, .jf=0, .k=0x0000000e },
   { .code=0x15, .jt=0, .jf=5, .k=0x00000059 },
   { .code=0x30, .jt=0, .jf=0, .k=0x0000000f },
   { .code=0x15, .jt=0, .jf=3, .k=0x00000047 },
   { .code=0x30, .jt=0, .jf=0, .k=0x00000010 },
   { .code=0x15, .jt=0, .jf=1, .k=0x00000047 },
   { .code=0x6, .jt=0, .jf=0, .k=0x00040000 },
   { .code=0x6, .jt=0, .jf=0, .k=0x00000000 },
 };

 static const size_t ygg_bpf_filter_len =     sizeof(YGG_filter) / sizeof(YGG_filter[0]);

/**********************************************************
 * Messages
 *********************************************************/

#pragma pack(1)
// Structure for a holding a mac address
typedef struct _WLANAddr{
   // address
   unsigned char data[WLAN_ADDR_LEN];
} WLANAddr;

// Structure of a frame header
typedef struct _WLANHeader{
    // destination address
    WLANAddr destAddr;
    // source address
    WLANAddr srcAddr;
    // type
    unsigned short type;
} WLANHeader;

#define WLAN_HEADER_LEN sizeof(struct _WLANHeader)

// Structure of an Yggdrasil message
typedef struct _YGGHeader{
   // Protocol family identifation
   unsigned char data[YGG_HEADER_LEN];
} YggHeader;

#define MAX_PAYLOAD DEFAULT_MTU -WLAN_HEADER_LEN -YGG_HEADER_LEN -sizeof(unsigned short)

// Yggdrasil physical layer message
typedef struct _YggPhyMessage{
  //Physical Level header;
  WLANHeader phyHeader;
  //Lightkone Protocol header;
  YggHeader yggHeader;
  //PayloadLen
  unsigned short dataLen;
  //Payload
  char data[MAX_PAYLOAD];
} YggPhyMessage;

#pragma pack()


/**********************************************************
 * Channel
 *********************************************************/

typedef struct _Channel {
    // socket descriptor
    int sockid_recv;
    int sockid_send;
    // interface index
    int ifindex;
    // mac address
    WLANAddr hwaddr;
    // maximum transmission unit
    int mtu;
    //ip address (if any)
    char ip_addr[INET_ADDRSTRLEN];
} Channel;


typedef struct _interface {
	int id;
	char* name;
	int type;
	struct _interface* next;
} Interface;

/*************************************************
 * Structure to configure the network device
 ************************************************/
typedef struct _NetworkConfig {
	int type; //type of the required network (IFTYPE)
	char* wifi_channel; //frequency of the signal
	int nscan; //number of times to perform network scans
	short mandatoryName; //1 if must connect to named network,; 0 if not
	char* name; //name of the network to connect
	struct bpf_insn* filter; //filter for the network
  size_t filter_len;
  char ip_addr[16];

	Interface* interfaceToUse; //interface to use
} NetworkConfig;

/*********************************************************
 * MISC
 *********************************************************/
 int str2wlan(char machine[], char human[]);
char* wlan2asc(WLANAddr* addr, char str[]);
int isYggMessage(void* buffer, int bufferLen);
int getInterfaceID(Channel* ch, char* ifname);
int getInterfaceMACAddress(Channel* ch, char* ifname);
int getInterfaceMTU(Channel* ch, char* ifname);
/*********************************************************
 * Setup
 *********************************************************/

 /*************************************************
  * YggPhyMessage
  *************************************************/
 int initYggPhyMessage(YggPhyMessage *msg); //initializes an empty payload lkmessage
 int initYggPhyMessageWithPayload(YggPhyMessage *msg, char* buffer, short bufferlen); //initializes a non empty payload lkmessage

 int addPayload(YggPhyMessage *msg, char* buffer);
 int deserializeYggPhyMessage(YggPhyMessage *msg, unsigned short msglen, void* buffer, int bufferLen);


 /*************************************************
  * API
  *************************************************/

int setupSimpleChannel(Channel* ch, NetworkConfig* ntconf);
int setupChannelNetwork(Channel* ch, NetworkConfig* ntconf);

/*********************************************************
 * Basic I/O
 *********************************************************/

/**
 * Send a message through the channel to the destination defined
 * in the message
 * @param ch The channel
 * @param message The message to be sent
 * @return The number of bytes sent through the channel
 */
int chsend(Channel* ch, YggPhyMessage* message);

/**
 * Send a message through the channel to the given address
 * @param ch The channel
 * @param message The message to be sent
 * @param addr The mac address of the destination
 * @return The number of bytes sent through the channel
 */
int chsendTo(Channel* ch, YggPhyMessage* message, char* addr);

/**
 * Send a message through the channel to the broadcast address
 * (one hop broadcast)
 * @param ch The channel
 * @param message The message to be sent
 * @return The number of bytes sent through the channel
 */
int chbroadcast(Channel* ch, YggPhyMessage* message);

/**
 * Receive a message through the channel
 * @param ch The channel
 * @param message The message to be received
 * @return The number of bytes received
 */
int chreceive(Channel* ch, YggPhyMessage* message);


int createChannel(Channel* ch, char* interface);
int bindChannel(Channel* ch);

#endif /* YGG_LL_RTEMS_H_ */
