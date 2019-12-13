#include "ygg_low_level.h"


/**********************************************************
 * Error prints
 **********************************************************/

 void logError(char* error) {

 	char buffer[26];
 	struct tm* tm_info;

 	struct timespec tp;
 	clock_gettime(CLOCK_REALTIME, &tp);
 	tm_info = localtime(&tp.tv_sec);

 	strftime(buffer, 26, "%Y:%m:%d %H:%M:%S", tm_info);

 	fprintf(stderr, "%s %ld :: %s\n", buffer, tp.tv_nsec, error);
 }

 void printError(int errorCode) {
 	char err[200];
 	bzero(err, 200);
 	switch(errorCode) {
 	case NO_IF_INDEX_ERR:
 		sprintf(err, "Could not extract the id for the wireless interface: %s",strerror(errno));
 		logError(err);
 		break;
 	case NO_IF_ADDR_ERR:
 		sprintf(err, "Could not extract the physical address of the interface: %s",strerror(errno));
 		logError(err);
 		break;
 	case NO_IF_MTU_ERR:
 		sprintf(err, "Could not extract the MTU of the interface: %s",strerror(errno));
 		logError(err);
 		break;
 	case SOCK_BIND_ERR:
 		sprintf(err, "Could not bind the socket adequately: %s",strerror(errno));
 		logError(err);
 		break;
 	case CHANNEL_SENT_ERROR:
 		sprintf(err, "Error transmitting message: %s",strerror(errno));
 		logError(err);
 		break;
 	case CHANNEL_RECV_ERROR:
 		sprintf(err, "Error receiving message: %s",strerror(errno));
 		logError(err);
 		break;
 	default:
 		sprintf(err, "An error has occurred within the COMM layer: %s",strerror(errno));
 		logError(err);
 		break;
 	}
 }


 /**********************************************************
 * Tools
 ***********************************************************/

 // Return the address in a human readable form
 char * wlan2asc(WLANAddr* addr, char str[]) {
 	sprintf(str, "%x:%x:%x:%x:%x:%x",
 			addr->data[0],addr->data[1],addr->data[2],
 			addr->data[3],addr->data[4],addr->data[5]);
 	return str;
 }

 // Convert a char to a hex digit
 int hexdigit(char a) {
 	if (a >= '0' && a <= '9') return(a-'0');
 	if (a >= 'a' && a <= 'f') return(a-'a'+10);
 	if (a >= 'A' && a <= 'F') return(a-'A'+10);
 	return -1;
 }

 int sscanf6(char str[], int *a1, int *a2, int *a3, int *a4, int *a5, int *a6){
 	int n;
 	*a1 = *a2 = *a3 = *a4 = *a5 = *a6 = 0;
 	while ((n=hexdigit(*str))>=0)
 		(*a1 = 16*(*a1) + n, str++);
 	if (*str++ != ':') return 1;
 	while ((n=hexdigit(*str))>=0)
 		(*a2 = 16*(*a2) + n, str++);
 	if (*str++ != ':') return 2;
 	while ((n=hexdigit(*str))>=0)
 		(*a3 = 16*(*a3) + n, str++);
 	if (*str++ != ':') return 3;
 	while ((n=hexdigit(*str))>=0)
 		(*a4 = 16*(*a4) + n, str++);
 	if (*str++ != ':') return 4;
 	while ((n=hexdigit(*str))>=0)
 		(*a5 = 16*(*a5) + n, str++);
 	if (*str++ != ':') return 5;
 	while ((n=hexdigit(*str))>=0)
 		(*a6 = 16*(*a6) + n, str++);
 	return 6;
 }

 int str2wlan(char machine[], char human[]) {
 	int a[6], i;
 	// parse the address
 	if (sscanf6(human, a, a+1, a+2, a+3, a+4, a+5) < 6) {
 		return -1;
 	}
 	// make sure the value of every component does not exceed on byte
 	for (i=0; i < 6; i++) {
 		if (a[i] > 0xff) return -1;
 	}
 	// assign the result to the member "data"
 	for (i=0; i < 6; i++) {
 		machine[i]= (char) a[i];
 	}
 	return 0;
 }

 int isYggMessage(void* buffer, unsigned int bufferLen) {
 	if(bufferLen < WLAN_HEADER_LEN+YGG_HEADER_LEN)
 		return 0;

 	unsigned char* p = (buffer+WLAN_HEADER_LEN);
 	unsigned char ygg[YGG_HEADER_LEN] = AF_YGG_ARRAY;
 	if(memcmp(p,ygg,3)==0) {
 		return 1;
 	}

 	return 0;
 }


 /*********************************************************
  * YggPhyMessage
  *********************************************************/
 int initYggPhyMessage(YggPhyMessage *msg) {
 	msg->phyHeader.type = IP_TYPE;
 	char id[] = AF_YGG_ARRAY;
 	memcpy(msg->yggHeader.data, id, YGG_HEADER_LEN);

 	return SUCCESS;
 }

 int initYggPhyMessageWithPayload(YggPhyMessage *msg, char* buffer, unsigned short bufferLen) {

 	if(bufferLen > MAX_PAYLOAD)
 		return FAILED;

 	msg->phyHeader.type = IP_TYPE;
 	char id[] = AF_YGG_ARRAY;
 	memcpy(msg->yggHeader.data, id, YGG_HEADER_LEN);
 	msg->dataLen = bufferLen;
 	memcpy(msg->data, buffer, bufferLen);

 	return SUCCESS;
 }

 int addPayload(YggPhyMessage *msg, char* buffer) {
 	size_t len = strlen(buffer);
 	if(len > MAX_PAYLOAD)
 		return -1;

 	msg->dataLen = (unsigned short) (len+1);
 	memcpy(msg->data, buffer, (size_t) (len+1));

 	return (int) len;
 }

/**********************************************************
* Interface manipulation
***********************************************************/


int getInterfaceID(Channel* ch, char* interface){

#ifdef DEBUG
	fprintf(stderr, "Interface: %s\n", interface);
#endif
  ch->ifindex = if_nametoindex(interface);
  if(ch->ifindex == 0) {
    lk_error_code = NO_IF_INDEX_ERR;
    return FAILED;
  }

	return SUCCESS;
}

int getInterfaceMACAddress(Channel* ch, char* interface){

  struct ifaddrs* ifap, *ifaptr;;

  if (getifaddrs(&ifap) == 0) {
       for(ifaptr = ifap; ifaptr != NULL; ifaptr = (ifaptr)->ifa_next) {
           if (!strcmp((ifaptr)->ifa_name, interface) && (((ifaptr)->ifa_addr)->sa_family == AF_LINK)) {
              unsigned char *ptr;
               ptr = (unsigned char *)LLADDR((struct sockaddr_dl *)(ifaptr)->ifa_addr);
               memcpy(ch->hwaddr.data, ptr, WLAN_ADDR_LEN);
               char macaddrstr[33];
               bzero(macaddrstr, 33);
               sprintf(macaddrstr, "%02x:%02x:%02x:%02x:%02x:%02x",
                                   *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
              char wlan_addr[33];
              bzero(wlan_addr, 33);
              wlan2asc(&ch->hwaddr, wlan_addr);
              printf("ptr: %s\nchn: %s\n", macaddrstr, wlan_addr);
              freeifaddrs(ifap);
              return SUCCESS;
           }
       }

   }
   	lk_error_code =  NO_IF_ADDR_ERR;
  return FAILED;


  /*
	struct ifreq ifr;
	strcpy(ifr.ifr_name, interface);
	if (ioctl(ch->sockid, SIOCGIFHWADDR, &ifr) == -1) {
		lk_error_code =  NO_IF_ADDR_ERR;
		return FAILED;
	}
	memcpy(&(ch->hwaddr.data), &(ifr.ifr_hwaddr.sa_data), WLAN_ADDR_LEN);
	return SUCCESS;
  */
}

int getInterfaceMTU(Channel* ch) {
	if (ioctl(ch->sockid_recv, BIOCGBLEN, &ch->mtu) == -1) {
		lk_error_code = NO_IF_MTU_ERR;
		return FAILED;
	}
	return SUCCESS;
}


int set_ip_addr(Channel* ch)
{

  char ifname[20];
  bzero(ifname, 20);
  if_indextoname(ch->ifindex, ifname);
  struct ifaddrs* ifap, *ifaptr;

  if (getifaddrs(&ifap) == 0) {

   for(ifaptr = ifap; ifaptr != NULL; ifaptr = (ifaptr)->ifa_next) {
           if (!strcmp((ifaptr)->ifa_name, ifname) && (((ifaptr)->ifa_addr)->sa_family == AF_INET)) {
             struct sockaddr_in *sa = (struct sockaddr_in *) (unsigned long) ifaptr->ifa_addr;
             char* addr = inet_ntoa(sa->sin_addr);
             printf("Interface: %s\tAddress: %s\n", ifaptr->ifa_name, addr);
             memcpy(ch->ip_addr, addr, strlen(addr));
             freeifaddrs(ifap);
             return SUCCESS;
              /*  if (inet_ntop(AF_INET, &(((struct sockaddr_in *)&ifaptr->ifa_addr)->sin_addr), ch->ip_addr, INET_ADDRSTRLEN) != NULL) {
                  freeifaddrs(ifap);
               return SUCCESS;
             } */
           }
   }

  }

	bzero(ch->ip_addr, INET_ADDRSTRLEN);
	return FAILED;
}

/*
int set_if_flags(Channel* ch, char *interface, short flags)
{
	struct ifreq ifr;

	ifr.ifr_flags = flags;
	strcpy(ifr.ifr_name, interface);

	return ioctl(ch->sockid_recv, SIOCSIFFLAGS, &ifr);

}

int setInterfaceUP(Channel* ch, char* interface)
{
	return set_if_flags(ch, interface, IFF_UP);
}

int setInterfaceDOWN(Channel* ch, char* interface)
{
	return set_if_flags(ch, interface, ~IFF_UP);
}

int checkInterfaceUP(Channel* ch, char* interface){
	struct ifreq if_req;
	strcpy(if_req.ifr_name, interface);
	int rv = ioctl(ch->sockid_recv, SIOCGIFFLAGS, &if_req);

	if ( rv == -1) return -1;

	return (if_req.ifr_flags & IFF_UP);
}

int checkInterfaceConnected(Channel* ch, char* interface){
	struct ifreq if_req;
	strcpy(if_req.ifr_name, interface);
	int rv = ioctl(ch->sockid_recv, SIOCGIFFLAGS, &if_req);

	if ( rv == -1) return -1;

	return (if_req.ifr_flags & IFF_RUNNING);
}
*/
/**********************************************************
 * Network configuration
 **********************************************************/


static void
create_wlandev_adhoc(void)
{
	int exit_code;
	char *ifcfg[] = {
		"ifconfig",
		"wlan0",
		"create",
		"wlandev",
		"rtwn0",
    "wlanmode",
    "adhoc",
    "up",
		NULL
	};

	exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(ifcfg), ifcfg);
	if(exit_code != EXIT_SUCCESS) {
		printf("ERROR while creating wlan0.");
	}
}

static void join_network(NetworkConfig* ntconf) {

  int exit_code;
	char *ifcfg[] = {
		"ifconfig",
		"wlan0",
    "inet",
    ntconf->ip_addr,
    "netmask",
    "255.255.0.0",
		"ssid",
		ntconf->name,
		"channel",
		ntconf->wifi_channel,
		NULL
	};

  exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(ifcfg), ifcfg);
	if(exit_code != EXIT_SUCCESS) {
		printf("ERROR while joining ledge.");
	}

}

static void
bpf_dump(const struct bpf_program *p, int option)
{
	struct bpf_insn *insn;
	unsigned int i;
	unsigned int n = p->bf_len;

	insn = p->bf_insns;
	if (option > 2) {
		printf("%d\n", n);
		for (i = 0; i < n; ++insn, ++i) {
			printf("%u %u %u %lu\n", insn->code,
			       insn->jt, insn->jf, insn->k);
		}
		return ;
	}
	if (option > 1) {
		for (i = 0; i < n; ++insn, ++i)
			printf("{ 0x%x, %d, %d, 0x%08lx },\n",
			       insn->code, insn->jt, insn->jf, insn->k);
		return;
	}
/*	for (i = 0; i < n; ++insn, ++i) {
#ifdef BDEBUG
		extern int bids[];
		printf(bids[i] > 0 ? "[%02d]" : " -- ", bids[i] - 1);
#endif
		puts(bpf_image(insn, i));
	} */
}

int defineFilter(Channel* ch, struct bpf_insn * filter, size_t filter_len) {
	struct bpf_program bpf = {
			.bf_len = filter_len,
			.bf_insns = filter
	};

  printf("sizeof filter: %d  %d  %d\n", sizeof(filter), sizeof(filter[0]), ygg_bpf_filter_len);

  bpf_dump(&bpf, 2);

  int r = ioctl(ch->sockid_recv, BIOCSETF, &bpf);

	//int r = setsockopt(ch->sockid, SOL_SOCKET, SO_ACCEPTFILTER, &bpf, sizeof(bpf));
	if(r != 0){
    perror("\t fault at defining filter\n");
		return CHANNEL_FILTER_ERROR;
  }
	return 0;
}

/*********************************************************
 * Setup
 *********************************************************/
int setupSimpleChannel(Channel* ch, NetworkConfig* ntconf){ //more parameters could be added to support multiple types of decision making on the interface to operate on

  //TODO to be more dynamic: scan net.wlan.devices, choose one
  Interface* interfaceToUse = NULL;
  printf("creating ad hoc\n");
  create_wlandev_adhoc();
  printf("created ad hoc\n");
  interfaceToUse = malloc(sizeof(Interface));
  interfaceToUse->name = "wlan0";
  printf("iterfaceToUse GO\n");
	int r = createChannel(ch, interfaceToUse->name);
	fprintf(stderr, "INFO: CONNECTING TO INTERFACE -> '%s'\n", interfaceToUse->name);
	if(r != 0) {
		printError(r); fprintf(stderr,"\n"); return FAILED; //exit(10);
	}
	//if(checkInterfaceUP(ch, interfaceToUse->name) <= 0)
	//	setInterfaceUP(ch, interfaceToUse->name);

	ntconf->interfaceToUse = interfaceToUse;
	return SUCCESS;
}

#define WAIT_THRESHOLD 10

static uint64_t mac2int(const uint8_t hwaddr[])
{
    int8_t i;
    uint64_t ret = 0;
    const uint8_t *p = hwaddr;

    for (i = 5; i >= 0; i--) {
        ret |= (uint64_t) *p++ << (CHAR_BIT * i);
    }

    return ret;
}

int setupChannelNetwork(Channel* ch, NetworkConfig* ntconf) {

  unsigned int seed = (unsigned int) mac2int(ch->hwaddr.data);
  srand(seed);

  uint8_t ip1 = (uint8_t) rand()%256;
  uint8_t ip2 = (uint8_t) rand()%256;

  bzero(ntconf->ip_addr, 16);
  sprintf(ntconf->ip_addr, "169.254.%d.%d", ip1, ip2);

  char c1[16];
  for(int i = 0; i < 16; i++) {
    c1[i] = ntconf->ip_addr[i];
    if(c1[i] == '.')
      c1[i] = '-';
  }

  fprintf(stderr, "Hostname -> '%s'\n", c1);
  sethostname(c1, strlen(c1));


  //TODO be more dynamic: scan networks, and join/create
  join_network(ntconf);

	int r = bindChannel(ch);

	if(r!=0) {fprintf(stderr, "Error binding channel: %s\n", strerror(r)); return FAILED;/* exit(130);*/}

  set_ip_addr(ch);
  fprintf(stderr, "Ip address -> '%s'\n", ch->ip_addr);

	if(ntconf->filter != NULL){
		r = defineFilter(ch, ntconf->filter, ntconf->filter_len);
		if(r!=0) {fprintf(stderr, "Error defining socket filter: %s\n", strerror(r));}
	}
	return SUCCESS;
}


/*********************************************************
 *  Channel
 *********************************************************/


int createChannel(Channel* ch, char* interface) {

  /* http://bastian.rieck.ru/howtos/bpf/ */
  const char *bpf_path = "/dev/bpf";
//  int fd = open(bpf_path, O_WRONLY, 0);
  char buf[16];
  bzero(buf, 16);

  printf("path: %s\n", bpf_path);
	ch->sockid_recv = open(bpf_path, O_RDONLY);

    for( int i = 0; i < 10; i++ )
    {
    sprintf( buf, "/dev/bpf%i", i );
    int bpf = open( buf, O_WRONLY );
    printf("path: %s\n", buf);
    if( bpf != -1 ) {
      ch->sockid_send = bpf;
        break;
      } else {
        perror("failed open bpf");
      }

    mode_t mode = 0666;
    mode |= S_IFCHR; //character device
    dev_t dev = domakedev(23, (unsigned long) i); //23 is the assigned number for bpf devices
    bpf = mknod(buf, mode, dev);
    printf("attempt mknode: %s\n", buf);
    if(bpf != -1) {
    printf("success mknode: %s\n", buf);
      bpf = open( buf, O_WRONLY );
      if( bpf != -1 ) {
        ch->sockid_send = bpf;
          break;
        } else {
          perror("failed open bpf");
        }
    } else {
      perror("failed mknode");
    }

  }

    if(ch->sockid_recv & (ch->sockid_send < 0))
		  return ch->sockid_send;

  printf("Created socket\n");


	int ret = getInterfaceID(ch, interface);
	if(ret != SUCCESS) return ret;

	printf("Interface id is %d\n",ch->ifindex);


	ret = getInterfaceMACAddress(ch, interface);
	if(ret != SUCCESS) return ret;

	char* addr = (char*) malloc(32);
	printf("Interface MAC address is %s\n",wlan2asc(&(ch->hwaddr),addr));
	free(addr);

	ret = getInterfaceMTU(ch);
	if(ret != SUCCESS) return 0;

	printf("Interface MTU is %d\n",ch->mtu);

	bzero(ch->ip_addr, INET_ADDRSTRLEN);

	return 0;
}

int bindChannel(Channel* ch){

  char ifname[20];
  bzero(ifname, 20);
  if_indextoname(ch->ifindex, ifname);

  printf("ifname: %s\n", ifname);

  struct ifreq bind_if;
  memset(&bind_if, 0, sizeof(bind_if));
  strcpy(bind_if.ifr_name, ifname);
  if (ioctl(ch->sockid_recv, BIOCSETIF, &bind_if) < 0 )
    return SOCK_BIND_ERR;

  if (ioctl(ch->sockid_send, BIOCSETIF, &bind_if) < 0 )
    return SOCK_BIND_ERR;

  int flags = 1;
  int ret = ioctl(ch->sockid_recv, BIOCIMMEDIATE, &flags);
  if(ret != SUCCESS) return ret;

  flags = BPF_D_IN;
  ret = ioctl(ch->sockid_recv, BIOCSDIRECTION, &flags);
  if(ret != SUCCESS) return ret;

  return 0;

  /*
	struct sockaddr_dl sdl;
	memset(&sdl, 0, sizeof(sdl));
	sdl.sdl_family=AF_LINK;
	sdl.sdl_index=ch->ifindex;
  sdl.sdl_type=IFT_ETHER;
	//sdl.sdl_protocol=htons(ETH_P_ALL);
	if (bind(ch->sockid, (struct sockaddr*)&sdl, sizeof(sdl)) < 0) {
		return SOCK_BIND_ERR;
	}
	return 0;

  */
}


/*********************************************************
 * Basic I/O
 *********************************************************/

 int deserializeYggPhyMessage(YggPhyMessage *msg, unsigned short msglen, void* buffer, int bufferLen) {
 	int checkType = isYggMessage(msg, msglen);
 	if(checkType != 0) {
 		memcpy(buffer, msg->data, (size_t) (bufferLen < msg->dataLen ? bufferLen : msg->dataLen));
 	}
 	return checkType;
 }

 void setToAddress(WLANAddr *daddr, unsigned short ifindex, struct sockaddr_dl *to) {

   //TODO this will probably not work
    to->sdl_family = AF_LINK;
    to->sdl_index = ifindex;
    memmove(&(to->sdl_data), daddr->data, WLAN_ADDR_LEN);
    to->sdl_alen=6;
 }

int chsend(Channel* ch, YggPhyMessage* message) {
	// send a frame

	memcpy(message->phyHeader.srcAddr.data, ch->hwaddr.data, WLAN_ADDR_LEN);
/*
	struct sockaddr_dl to = {0};
	setToAddress(&message->phyHeader.destAddr, ch->ifindex, &to);

	int sent=sendto(
			ch->sockid,
			message, (WLAN_HEADER_LEN+YGG_HEADER_LEN+(sizeof(unsigned short))+message->dataLen), //sizeof(YggPhyMessage)
			0,
			(struct sockaddr*) &to, sizeof(to));
*/
  int sent = write(ch->sockid_send, message, WLAN_HEADER_LEN+YGG_HEADER_LEN+(sizeof(unsigned short))+message->dataLen);

	if(sent < 0) {
		return CHANNEL_SENT_ERROR;
	}


#ifdef DEBUG
	fprintf(stderr,"Transmitted %d bytes\n", sent);
#endif
	return sent;
}

// Send
int chsendTo(Channel* ch, YggPhyMessage* message, char* addr) {

	memcpy(message->phyHeader.destAddr.data, addr, WLAN_ADDR_LEN);

	return chsend(ch, message);
}

// Send
int chbroadcast(Channel* ch, YggPhyMessage* message) {

	char mcaddr[WLAN_ADDR_LEN];
	str2wlan(mcaddr, WLAN_BROADCAST); //translate addr to machine addr

	return chsendTo(ch, message, mcaddr);
}


// Receive
int chreceive(Channel* ch, YggPhyMessage* message) {
	//struct sockaddr_dl from;
	//socklen_t fromlen=sizeof(struct sockaddr_dl);
	// wait and receive a frame
/*
	int recv = recvfrom(ch->sockid, message, DEFAULT_MTU,//sizeof(LKMessage),
			0, (struct sockaddr *) &from, &fromlen);
*/
  unsigned int buflen = ch->mtu;
  struct bpf_hdr* buf = (struct bpf_hdr *) malloc(buflen); bzero(buf, buflen);
  int recv = read(ch->sockid_recv, buf, buflen);

#ifdef DEBUG
  printf("bpf_hdr: %d + eth_hdr: %d  bh_hrdlen: %d  bh_datalen %d  bh_caplen %d\n", sizeof(struct bpf_hdr), sizeof(struct ether_header), buf->bh_hdrlen, buf->bh_datalen, buf->bh_caplen);
#endif

  recv = recv-buf->bh_hdrlen; //remove bpf header

  YggPhyMessage* msg = (YggPhyMessage*)((char*)buf+buf->bh_hdrlen);
  message->phyHeader = msg->phyHeader;
  if(memcmp(msg->phyHeader.srcAddr.data, ch->hwaddr.data, WLAN_ADDR_LEN) == 0) {
    recv = -1;
  goto error;
}

  message->yggHeader = msg->yggHeader;
  message->dataLen = msg->dataLen;
  bzero(message->data, MAX_PAYLOAD);
  memcpy(message->data, msg->data, msg->dataLen);
  //int recv = read(ch->sockid, message, DEFAULT_MTU);
error:
  free(buf);
	if(recv < 0) return CHANNEL_RECV_ERROR;

#ifdef DEBUG
	fprintf(stderr,"Received %d bytes\n", recv);
#endif
	return recv;
}
