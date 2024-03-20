#ifndef TEST_AGENT_H
#define TEST_AGENT_H

#include <dz1_asn1.h>
#include <ConformanceTestAgentDef.h>


DZ1_CPPLINK void * ConformanceTestAgent_new(Dz1Asn1UTF8Str *ws_werer_uri, ConformanceTestAgentException exceptionProc, void *exceptionArg, Dz1Error *err);
DZ1_CPPLINK void ConformanceTestAgent_del(void *task);

// cmd by shell 
DZ1_CPPLINK bool_t ConformanceTestAgent_sendTextToWebSocket(Dz1Task* task, Dz1Str _text, Dz1Error* err);
#endif // TEST_AGENT
