#ifndef CONFORMANCE_TEST_AGENT_TIMER_UTIL_H_LOCAL
#define CONFORMANCE_TEST_AGENT_TIMER_UTIL_H_LOCAL

#include <dz1_task.h>
#include "ConformanceTestAgentTimer.h"

DZ1_CPPLINK Dz1Error ConformanceTestAgentTimer_set(void *timer, ConformanceTestAgentTimer **tmr, u64_t dur, Dz1TaskTimeoutProcFunc timeout);
DZ1_CPPLINK Dz1Error ConformanceTestAgentTimer_update(void *timer, ConformanceTestAgentTimer **tmr, u64_t dur, Dz1TaskTimeoutProcFunc timeout);
DZ1_CPPLINK void ConformanceTestAgentTimer_clr(void *timer, ConformanceTestAgentTimer *key);

#endif	// CONFORMANCE_TEST_AGENT_TIMER_UTIL_H_LOCAL
