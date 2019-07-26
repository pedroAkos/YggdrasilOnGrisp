#ifndef INTERFACES_DISCOVERY_EVENTS_H_
#define INTERFACES_DISCOVERY_EVENTS_H_

#include "../core/ygg_runtime.h"

typedef enum {
	NEIGHBOUR_UP,
	NEIGHBOUR_DOWN
}discovery_events;

void send_event_neighbour_up(short proto_id, uuid_t neighbour_id, WLANAddr* neighbour_addr);

void send_event_neighbour_down(short proto_id, uuid_t neighbour_id);

#endif /* INTERFACES_DISCOVERY_EVENTS_H_ */
