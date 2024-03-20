#ifndef CONFORMANCE_TEST_AGENT_MSG_H_LOCAL
#define CONFORMANCE_TEST_AGENT_MSG_H_LOCAL

#define CONFORMANCE_TEST_AGENT_CORE_MSG				0xF0000000
#define CONFORMANCE_TEST_AGENT_DO_DEFAULT			(CONFORMANCE_TEST_AGENT_CORE_MSG | 0x00000001)
#define CONFORMANCE_TEST_AGENT_SEND_TEXT			(CONFORMANCE_TEST_AGENT_CORE_MSG | 0x00000002)
#define CONFORMANCE_TEST_AGENT_GEN_SCENARIO_TASK	(CONFORMANCE_TEST_AGENT_CORE_MSG | 0x00000003)

// web<->Agent 
#define CONFORMANCE_TEST_AGENT_WEB_MSG				0x0F000000
#define CONFORMANCE_TEST_AGENT_RCV_CNF_AGENT_READY	(CONFORMANCE_TEST_AGENT_WEB_MSG | 0x00000001)
#define CONFORMANCE_TEST_AGENT_RCV_TESTINFORM		(CONFORMANCE_TEST_AGENT_WEB_MSG | 0x00000002)

#define CONFORMANCE_TEST_AGENT_RCV_TEST_MSG			(CONFORMANCE_TEST_AGENT_WEB_MSG | 0X0000000F)

// web<->Scenario
#define CONFORMANCE_TEST_AGENT_IPC_MSG					0x00F00000
#define CONFORMANCE_TEST_AGENT_WEB_TO_SCENARIO_MSG		(CONFORMANCE_TEST_AGENT_IPC_MSG | 0x00000001)
#define CONFORMANCE_TEST_AGENT_SCENARIO_TO_WEB_MSG		(CONFORMANCE_TEST_AGENT_IPC_MSG | 0x00000002)


#include "ConformanceTestAgentMsgs.h"


DZ1_CPPLINK bool_t ConformanceTestAgentMsgWebSock_msgInit(Dz1Task* task, Dz1Error* err);

#endif // CONFORMANCE_TEST_AGENT_MSG_H_LOCAL