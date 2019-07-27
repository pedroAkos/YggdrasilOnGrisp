#include <yggdrasil/api.h>

#include "core/ygg_runtime.h"
#include "protocols/discovery.h"

queue_t* inBox =  NULL;
unsigned short myId = 400;

void init_yggdrasil(const char* ssid, const char* wifi_channel) {
  NetworkConfig ntconf;
  ntconf.wifi_channel = (char*)(unsigned long) wifi_channel;
  ntconf.name = (char*)(unsigned long) ssid;
  ntconf.filter = (struct bpf_insn*)(unsigned long) YGG_filter;
  ntconf.filter_len = ygg_bpf_filter_len;

  ygg_runtime_init(&ntconf);

	ygg_log("APP", "AFTER INIT", "");


  //TODO Change this to a config
  simple_discovery_args* sargs = simple_discovery_args_init(true, 2, 0);
  registerProtocol(PROTO_DISCOVER_IP_ID, discover_ip_init, sargs);
  simple_discovery_args_destroy(sargs);


  //TODO call this from start, with arguments ?????
	app_def* myapp = create_application_definition(myId, "MyApp");
  app_def_add_consumed_events(myapp, PROTO_DISCOVER_IP_ID, NEIGHBOUR_UP);
  app_def_add_consumed_events(myapp, PROTO_DISCOVER_IP_ID, NEIGHBOUR_DOWN);

	inBox = registerApp(myapp);

}

void start_yggdrasil(void) {
  ygg_log("YGGDRASIL", "NOTICE", "STARTING RUNTIME");
  ygg_runtime_start();
}


short send_msg(void* msg_contents, unsigned short msg_contents_size) {

  if(msg_contents_size > YGG_MESSAGE_PAYLOAD)
    return -1;

  YggMessage msg;
  YggMessage_initBcast(&msg, myId);
  YggMessage_addPayload(&msg, msg_contents, msg_contents_size);

  ygg_dispatch(&msg);

  return 1;
}


msg_type* deliver_msg(void) {
  if(!inBox)
    return NULL;

  queue_t_elem el;
  queue_pop(inBox, &el);

  msg_type* msg =  NULL;

  if(el.type == YGG_MESSAGE) {
    msg = malloc(sizeof(msg_type));
    msg->type = NET_MESSAGE;
    msg->content = malloc(el.data.msg.dataLen);
    memcpy(msg->content, el.data.msg.data, el.data.msg.dataLen);

  } else if(el.type == YGG_EVENT) {
    msg = malloc(sizeof(msg_type));
    msg->type = NOTIFY;
    msg->content = malloc(el.data.event.length);
    memcpy(msg->content, el.data.event.payload, el.data.event.length);
  }

  free_elem_payload(&el);

  return msg;
}


const char* get_ip(void) {
  return getChannelIpAddress();
}
