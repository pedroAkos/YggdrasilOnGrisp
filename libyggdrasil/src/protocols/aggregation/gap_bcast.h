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


#ifndef PROTOCOLS_AGGREGATION_GAP_BCAST_H_
#define PROTOCOLS_AGGREGATION_GAP_BCAST_H_

#include "core/ygg_runtime.h"
#include "interfaces/aggregation/aggregation_operations.h"
#include "interfaces/aggregation/aggregation_functions.h"
#include "interfaces/discovery/discovery_events.h"
#include "data_structures/generic/list.h"

#include "gap.h"

#define PROTO_AGG_GAP_BCAST 222


proto_def* gap_bcast_init(void* args);


#endif /* PROTOCOLS_AGGREGATION_GAP_BCAST_H_ */
