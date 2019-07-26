/*********************************************************
 * This code was written in the context of the Lightkone
 * European project.
 * Code is of the authorship of NOVA (NOVA LINCS @ DI FCT
 * NOVA University of Lisbon)
 * Authors:
 * Pedro Ákos Costa (pah.costa@campus.fct.unl.pt)
 * João Leitão (jc.leitao@fct.unl.pt)
 * (C) 2018
 *********************************************************/

#include "../discovery.h"

#define DEFAULT_ANNOUNCE_PERIOD_S 2
#define DEFAULT_ANNOUNCE_PERIOD_NS 0

typedef struct _neighbour_ip {
  char ip[16];
  char* hostname;
}neigh_ip;

static bool equal_ip(neigh_ip* n, const char* ip) {
  return strcmp(n->ip, ip) == 0;
}

typedef struct _discover_ip_state {
	const char* my_ip;
  const char* my_hostname;
  uint8_t hostname_len;
	list* neighbours;
	short proto_id;
	YggTimer announce;
}discover_ip_state;

static void ev_neigh(neigh_ip* n, short proto_id, discovery_events ev_t) {
  YggEvent ev;
  YggEvent_init(&ev, proto_id, ev_t);
  YggEvent_addPayload(&ev, n->ip, 16);
  int len = strlen(n->hostname)+1;
  char zero = '\0';
  YggEvent_addPayload(&ev, &len, sizeof(int));
  YggEvent_addPayload(&ev, n->hostname, len);
  YggEvent_addPayload(&ev, &zero, 1);

  deliverEvent(&ev);
  YggEvent_freePayload(&ev);
}

static short process_msg(YggMessage* msg, discover_ip_state* state) {

  char ip[16];
  bzero(ip, 16);
	void* ptr = YggMessage_readPayload(msg, NULL, ip, 16);

  if(list_find_item(state->neighbours, (comparator_function) equal_ip, ip) == NULL) {

#ifdef DEGUB
		printf("New neighbour\n");
#endif
    neigh_ip* n = malloc(sizeof(neigh_ip));
    memcpy(n->ip, ip, 16);
    uint8_t len;
    ptr = YggMessage_readPayload(msg, ptr, &len, sizeof(uint8_t));
    n->hostname = malloc(len);
    YggMessage_readPayload(msg, ptr, n->hostname, len);

    list_add_item_to_head(state->neighbours, n);

    ev_neigh(n, state->proto_id, NEIGHBOUR_UP);

	}

	return SUCCESS;
}

static short process_timer(YggTimer* timer, discover_ip_state* state) {

	YggMessage msg;
	YggMessage_initBcast(&msg, state->proto_id);
	YggMessage_addPayload(&msg, (void*) state->my_ip, 16);
  YggMessage_addPayload(&msg, (void*) &state->hostname_len, sizeof(uint8_t));
  YggMessage_addPayload(&msg, (void*) state->my_hostname, state->hostname_len);

	ygg_dispatch(&msg);

	return SUCCESS;
}

static void* simple_discovery_main_loop(main_loop_args* args) {
	discover_ip_state* state = (discover_ip_state*) args->state;
	queue_t* inBox = args->inBox;

	while(true) {
		queue_t_elem elem;
		queue_pop(inBox, &elem);

		switch(elem.type) {
		case YGG_MESSAGE:
			process_msg(&elem.data.msg, (void*) state);
			break;
		case YGG_TIMER:
			process_timer(&elem.data.timer, (void*) state);
			break;
		default:
			//noop
			break;
		}

		free_elem_payload(&elem);
	}

	return NULL;
}

static short discover_ip_destroy(discover_ip_state* state) {
	cancelTimer(&state->announce);
  while(state->neighbours->head != NULL) {
    neigh_ip* n = list_remove_head(state->neighbours);
    free(n);
  }
	free(state->neighbours);
	free(state);

	return SUCCESS;
}

proto_def* discover_ip_init(void* args) {

	simple_discovery_args* sargs = (simple_discovery_args*)args;

	discover_ip_state* state = malloc(sizeof(discover_ip_state));
  state->my_ip = getChannelIpAddress();
  state->my_hostname = getHostname();
  state->hostname_len = strlen(state->my_hostname);

	state->neighbours = list_init();
	state->proto_id = PROTO_DISCOVER_IP_ID;

	proto_def* discovery = create_protocol_definition(state->proto_id,
			"Discover IP", state, discover_ip_destroy);

	proto_def_add_produced_events(discovery, 1); //NEIGHBOUR_UP

	if(sargs->run_on_executor) {
		proto_def_add_msg_handler(discovery, process_msg);
		proto_def_add_timer_handler(discovery, process_timer);
	} else
		proto_def_add_protocol_main_loop(discovery, simple_discovery_main_loop);


	YggTimer_init(&state->announce, state->proto_id, state->proto_id);
	YggTimer_set(&state->announce, sargs->announce_period_s, sargs->announce_period_ns,
			sargs->announce_period_s, sargs->announce_period_ns);


	setupTimer(&state->announce);
	return discovery;
}
