#include <dz1_task_timer.h>
#include "ConformanceTestAgentTimerUtil.h"

Dz1Error ConformanceTestAgentTimer_set(void *timer, ConformanceTestAgentTimer **tmr, u64_t dur, Dz1TaskTimeoutProcFunc timeout)
{
	DZ1_ERROR_SAFE_VAR(errp, err);
	if ((*errp = Dz1TaskTimer_reg(timer, dur, (void **)tmr, (Dz1DelFunc)ConformanceTestAgentTimer_del, timeout, FALSE)).code) ERR_OUT(errp);
	return err;
}

Dz1Error ConformanceTestAgentTimer_update(void *timer, ConformanceTestAgentTimer **tmr, u64_t dur, Dz1TaskTimeoutProcFunc timeout)
{
	DZ1_ERROR_SAFE_VAR(errp, err);
	if ((*errp = Dz1TaskTimer_reg(timer, dur, (void **)tmr, (Dz1DelFunc)ConformanceTestAgentTimer_del, timeout, TRUE)).code) ERR_OUT(errp);
	return err;
}

void ConformanceTestAgentTimer_clr(void *timer, ConformanceTestAgentTimer *key)
{
	Dz1TaskTimer_dereg(timer, (void *)key);
}
