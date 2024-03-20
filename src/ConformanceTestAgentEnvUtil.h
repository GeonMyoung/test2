#ifndef CONFORMANCE_TEST_AGENT_AGENT_UTIL_H_LOCAL
#define CONFORMANCE_TEST_AGENT_AGENT_UTIL_H_LOCAL

#include <ConformanceTestAgentMsg.h>
#include "ConformanceTestAgentEnv.h"

DZ1_CPPLINK bool_t ConformanceTestAgentEnv_establish(ConformanceTestAgentEnv* env, Dz1Asn1UTF8Str* ws_server_uri, Dz1Error* errp);
DZ1_CPPLINK bool_t ConformanceTestAgentEnv_send_MyType(ConformanceTestAgentEnv* env, Dz1Error* err);

DZ1_CPPLINK bool_t ConformanceTestAgentEnv_sendToServer(ConformanceTestAgentEnv* env, AgentSendMessage* msg, Dz1Error* err);
DZ1_CPPLINK bool_t ConformanceTestAgentEnv_send_ready(ConformanceTestAgentEnv* env, Dz1Error* err);

//gm
DZ1_CPPLINK bool_t ConformanceTestAgentEnv_sendStr(ConformanceTestAgentEnv* env, Dz1Str str, Dz1Error* err);

#endif
