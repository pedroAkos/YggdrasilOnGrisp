#ifndef YGGDRASIL_PROTOCOLS_DISCOVERY_H_
#define YGGDRASIL_PROTOCOLS_DISCOVERY_H_

#include "../core/ygg_runtime.h"
#include "../interfaces/discovery_events.h"
#include "../data_structures/specialized/neighbour_list.h"


#define PROTO_SIMPLE_DISCOVERY_ID 100

typedef struct _simple_discovery_args {
	bool run_on_executor;
	time_t announce_period_s;
	unsigned long announce_period_ns;
}simple_discovery_args;

simple_discovery_args* simple_discovery_args_init(bool run_on_executor, time_t announce_period_s, unsigned long announce_period_ns);
void simple_discovery_args_destroy(simple_discovery_args* sargs);

proto_def* simple_discovery_init(void* args);


#define PROTO_FAULT_DETECTOR_DISCOVERY_ID 101

typedef struct _fault_detector_discovery_args {
	time_t announce_period_s;
	unsigned long announce_period_ns;

	unsigned short mgs_lost_per_fault; //0 fault detector off; number of messages to be missed before suspect;
	unsigned short black_list_links; //0 off; number of consecutive faults before black list;
}fault_detector_discovery_args;

fault_detector_discovery_args* fault_detector_discovery_args_init(time_t announce_period_s, unsigned long announce_period_ns, unsigned short mgs_lost_per_fault, unsigned short black_list_links);

void fault_detector_discovery_args_destroy(fault_detector_discovery_args* fdargs);

proto_def* fault_detector_discovery_init(void* args);

#define PROTO_DISCOVER_IP_ID 102

proto_def* discover_ip_init(void* args);


#endif /* YGGDRASIL_PROTOCOLS_DISCOVERY_H_ */
