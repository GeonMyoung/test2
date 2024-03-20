#include <Dz1SockUtil.h>
#include <dz1_web_socket.h>

#include <TrafficLightChk.h>
#include <TrafficLightMsg.h>

#include "ConformanceTestAgentEnvUtil.h"
#include "ConformanceTestAgentMsg.h"



///////////////////////////////////////////////////////////////////////////////
// Message : DZ1_SOCK_UTIL_RECEIVER_RECEIVED
// json_proc 함수 내부 동작은 이벤트 말고 함수로 빼기
// Agent 연동 관련 메시지 / 시나리오 Task로 보내는 메시지기준으로 파일 나누기



static bool_t _testfunc1(void* exceptionArg, Dz1Error* errp) {
	Dz1Thread_printf("111111\n");
	return TRUE;
}


static bool_t _testfunc2(void* exceptionArg, Dz1Error* errp) {
	Dz1Thread_printf("222222\n");
	return TRUE;
}
// static 함수 위치 조정 필요 -- 20240316
static void _task_exception_handler(void* ptr, Dz1Error* reason)
{

	Dz1Thread_log(DZ1_THREAD_LOG_ERROR, "%s : Exception Catch\n", (str_t)ptr);
	ERR_OUT(reason);
}

// AgentSendMessage 생성 함수로 변경 고려 -- 20240316
static AgentSendMessage* AgentSendMessage_Confirm_TestStart_Handler(ConformanceTestAgentEnv* env, void* timer, AgentSendMessage* send_msg, bool_t task_start_result, Dz1Error* errp)
{
	AgentSendMessage* ret = send_msg;
	//ConfirmTestStart cnf = { env->seq, env->testIdx, TRUE };
	//MainTasktoWebMsg mtw = { MainTasktoWebMsgPresent_confirm_testStart, &cnf };
	//AgentSendMessageContext ctx = { AgentSendMessageContextPresent_main2web, &mtw };
	//AgentSendMessage reply = { "main2web", &ctx };

	if ((send_msg = AgentSendMessage_new("main2web", NULL, errp)) == NULL) ERR_OUT(errp);
	else if ((send_msg->context = AgentSendMessageContext_new(AgentSendMessageContextPresent_main2web, NULL, errp)) == NULL) ERR_OUT(errp);
	else if ((send_msg->context->x.main2web = MainTasktoWebMsg_new(MainTasktoWebMsgPresent_confirm_testStart, NULL, errp)) == NULL) ERR_OUT(errp);
	else
	{
		if(task_start_result)
		{
			if ((send_msg->context->x.main2web->x.confirm_testStart = ConfirmTestStart_new(env->seq, env->testIdx, TRUE, errp)) == NULL) ERR_OUT(errp);
			else Dz1Error_set(errp, 0);
		}
		else
		{
			if ((send_msg->context->x.main2web->x.confirm_testStart = ConfirmTestStart_new(env->seq, env->testIdx, FALSE, errp)) == NULL) ERR_OUT(errp);
			else Dz1Error_set(errp, 0);
		}
	}
	return ret;
}

static bool_t _gen_scenario_task(ConformanceTestAgentEnv* env, Dz1Error* errp)
{
	if (env->testIdx == 0) ERR_SET_OUT(errp, EINVAL);
	else
	{
		Dz1Thread_printf("testIdx = %d\n", env->testIdx);
		switch (env->testIdx)
		{
		case TestIdx_TrafficLightChk:
			if ((env->tScenario = TrafficLightChk_new(env->tSelf, _task_exception_handler, (str_t)"TrafficLightChk", errp)) == NULL) ERR_OUT(errp);
			//else if ((*errp = Dz1Task_start(env->tScenario, DZ1_THREAD_LOG_BASIS | DZ1_THREAD_LOG_DEBUG)).code) ERR_OUT(errp);
			// else if () // 생성 완료 메시지 송신
			else
			{
				AgentSendMessage* msg = NULL;
				pthread_cleanup_push(AgentSendMessage_delAndSetNull, (void*)&msg);
				if ((msg = AgentSendMessage_new("main2web", NULL, errp)) == NULL) ERR_OUT(errp);
				else if ((msg->context = AgentSendMessageContext_new(AgentSendMessageContextPresent_main2web, NULL, errp)) == NULL) ERR_OUT(errp);
				else if ((msg->context->x.main2web = MainTasktoWebMsg_new(MainTasktoWebMsgPresent_confirm_testInform, NULL, errp)) == NULL) ERR_OUT(errp);
				else if ((msg->context->x.main2web->x.confirm_testInform = ConfirmTestInform_new(env->seq, (u8_t)TestIdx_TrafficLightChk, TRUE, errp)) == NULL) ERR_OUT(errp);
				else if (ConformanceTestAgentEnv_sendToServer(env, msg, errp) == FALSE) ERR_OUT(errp);
				Dz1Error_set(errp, 0);
				pthread_cleanup_pop(1); // (AgentSendMessage_delAndSetNull, (void*)&msg);
			}
			break;
		case TestIdx_Iso14827_2:
		case TestIdx_TechnicalRegulation1:
		case TestIdx_TechnicalRegulation2:
			break;
		case TestIdx_unknown:
			break;
		default:
			break;
		}
	}
	return ERR_IS_SUCCESS(errp);
}
// static 함수 위치 조정 필요 -- 20240316


static bool_t _json_text_proc(ConformanceTestAgentEnv* env, void* timer, Dz1WebSockMsg* ws_msg, Dz1Error* errp)
{
	int cnt = 0;
	Dz1Json* json = NULL;
	Dz1Binary* payload = NULL;
	if ((payload = ws_msg->payload) == NULL) ERR_SET_OUT(errp, EFAULT);
	else if ((json = Dz1Json_new("UTF-8", env->spec, "AgentReceiveMessage", errp)) == NULL) ERR_OUT(errp);
	else
	{
		ssize_t sz;
		pthread_cleanup_push(Dz1Json_delAndSetNull, (void*)&json);
		if ((sz = Dz1Json_push_stream(json, payload->data, payload->size, errp)) < 0) ERR_OUT(errp);
		else if (sz != (ssize_t)payload->size) ERR_SET_OUT(errp, EFAULT);
		else if ((cnt = Dz1Json_parse(json, errp)) < 0) ERR_OUT(errp);
		else if (cnt == 0) Dz1Error_set(errp, 0);
		else
		{
			AgentReceiveMessage* msg = NULL;
			for (int i = 0; i < cnt; i++)
			{
				if ((msg = (AgentReceiveMessage*)Dz1Json_read(json, errp)) == NULL) ERR_OUT(errp);
				else
				{
					u32_t log_lv = 0;
					pthread_cleanup_push(AgentReceiveMessage_delAndSetNull, (void*)&msg);
					Dz1Thread_getLogLevel(Dz1Thread_self(), &log_lv);

					if (log_lv & DZ1_THREAD_LOG_DEBUG)
					{
						_Dz1Thread_log("Received Message = ");
						AgentReceiveMessage_dump(msg, 0);
					}

					if(strcmp(msg->type, "web2main") == 0)
					{
						switch (msg->context->x.web2main->present)
						{
						case WebtoMainTaskMsgPresent_confirm_agentReady:
							if ((Dz1Task_postSimple(env->tSelf, DZ1_TASK_QUEUE_NONE, CONFORMANCE_TEST_AGENT_RCV_CNF_AGENT_READY)).code) ERR_OUT(errp);
							else Dz1Error_set(errp, 0);
							break;
						case WebtoMainTaskMsgPresent_notify_testInform:
							// Set Test Inform to Env
							// Scenario Task Start
							// Send to Web Confirm TestInform Msg
							env->testIdx = msg->context->x.web2main->x.notify_testInform->testInform->idx;
							env->seq = msg->context->x.web2main->x.notify_testInform->seq;
							switch (env->testIdx)	// env에 시나리오 핸들러 함수 지정
							{
							case 1:
								env->scenarioHandler = &_testfunc1;
								env->scenarioHandler(NULL, errp);
								break;
							case 2:
								env->scenarioHandler = &_testfunc2;
								env->scenarioHandler(NULL, errp);
								break;
							default:
								break;
							}
							if(_gen_scenario_task(env,errp) == FALSE) ERR_OUT(errp);
							else Dz1Error_set(errp, 0);
							break;

						case WebtoMainTaskMsgPresent_notify_testStart:
							if (env->testIdx != msg->context->x.web2main->x.notify_testStart->testIdx) ERR_OUT(errp);
							else if ((env->seq = msg->context->x.web2main->x.notify_testStart->seq) < 0) ERR_OUT(errp);
							else
							{
								AgentSendMessage* send_msg = NULL;
								pthread_cleanup_push(AgentSendMessage_delAndSetNull, (void*)&send_msg);
								if ((Dz1Task_start(env->tScenario, DZ1_THREAD_LOG_BASIS | DZ1_THREAD_LOG_DEBUG)).code) ERR_OUT(errp);
								else if ((send_msg = AgentSendMessage_new("main2web", NULL, errp)) == NULL) ERR_OUT(errp);
								else if ((send_msg->context = AgentSendMessageContext_new(AgentSendMessageContextPresent_main2web, NULL, errp)) == NULL) ERR_OUT(errp);
								else if ((send_msg->context->x.main2web = MainTasktoWebMsg_new(MainTasktoWebMsgPresent_confirm_testStart, NULL, errp)) == NULL) ERR_OUT(errp);
								else if ((send_msg->context->x.main2web->x.confirm_testStart = ConfirmTestStart_new(env->seq, env->testIdx, TRUE, errp)) == NULL) ERR_OUT(errp);
								else if (ConformanceTestAgentEnv_sendToServer(env, send_msg, errp) == FALSE) ERR_OUT(errp);
								else Dz1Error_set(errp, 0);
								pthread_cleanup_pop(1); // (AgentSendMessage_delAndSetNull, (void*)&send_msg);

							}
							break;
						case WebtoMainTaskMsgPresent_req_hb:
							// Reply HB

							break;

						default:
							break;

						}
					}
					else if (strcmp(msg->type, "web2scenario") == 0)
					{
						Dz1Thread_printf("web2ScenarioMsg entered!!!\n");
						Dz1Thread_printf("web2ScenarioMsg entered!!!\n");
						Dz1Thread_printf("web2ScenarioMsg entered!!!\n");
						Dz1Str ctx = Dz1Str_clone(msg->context->x.web2scenario, errp);
						if (ctx == NULL) ERR_OUT(errp);
						else if ((Dz1Task_post(env->tScenario, DZ1_TASK_QUEUE_NONE, CONFORMANCE_TEST_AGENT_WEB_TO_SCENARIO_MSG, (void**)&ctx, NULL, NULL, NULL, NULL, NULL)).code) ERR_OUT(errp);
					}
					pthread_cleanup_pop(1); // (AgentReceiveMessageTest_delAndSetNull, (void*)&msg);
				}
			}
		}
		pthread_cleanup_pop(1); //  (Dz1Json_delAndSetNull, (void*)&json);
	}
	return ERR_IS_SUCCESS(errp);
}

static Dz1TaskProcStatus _ws_received(void* uArg, void* uRsc, void* timer, Dz1TaskSigMsg** sg, Dz1Error* errp)
{
	ConformanceTestAgentEnv* env = (ConformanceTestAgentEnv*)uRsc;
	Dz1SockUtilReceiverReceived* msg = (Dz1SockUtilReceiverReceived*)Dz1TaskSigMsg_getData(*sg);
	Dz1WebSockMsgList* msgs = env->ws_msgs;

	if(env->wsock != NULL)
	{
		if (Dz1WebSocket_onReceived(env->wsock, msg, msgs, errp) == FALSE)
			Dz1Thread_log(DZ1_THREAD_LOG_CRITICAL, "!!! Receive Message Process fail. but close anyway\n");
		else 
		{
			Dz1WebSockMsg* ws_msg = NULL;
			while((ws_msg = msgs->extract(msgs, NULL)) != NULL)
			{
				u32_t log_lv = 0;
				Dz1Thread_getLogLevel(Dz1Thread_self(), &log_lv);
				switch(ws_msg->op)
				{
				case Dz1WebSockOpCode_text:
					if(log_lv & DZ1_THREAD_LOG_DEBUG)
					{
						_Dz1Thread_log("<<< Text Message Received = ");
						DZ1_DUMP(Dz1Binary, ws_msg->payload, 0);
					}
					if (_json_text_proc(env, timer, ws_msg, errp) == FALSE) ERR_OUT(errp);
					else Dz1Error_set(errp, 0);
					break;
				case Dz1WebSockOpCode_max:
					Dz1Thread_log(DZ1_THREAD_LOG_DEBUG, "<<< Web Socket session established\n");
					if (ConformanceTestAgentEnv_send_MyType(env, errp) == FALSE) ERR_OUT(errp);
					else
					{
						Dz1Task_postSimple(env->tSelf, DZ1_TASK_QUEUE_NONE, CONFORMANCE_TEST_AGENT_DO_DEFAULT);
						Dz1Error_set(errp, 0);
					}
					break;
				default:
					break;
				}
			}
		}
	}
	return SELECT_BY_ERR(errp, Dz1TaskProcStatus_processed, Dz1TaskProcStatus_error);
}
// Message : DZ1_SOCK_UTIL_RECEIVER_RECEIVED
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Message : DZ1_SOCK_UTIL_RECEIVER_RECEIVED
static Dz1TaskProcStatus _ws_sent(void* uArg, void* uRsc, void* timer, Dz1TaskSigMsg** sg, Dz1Error* errp)
{
	Dz1TaskProcStatus ret = Dz1TaskProcStatus_processed;
	ConformanceTestAgentEnv* env = (ConformanceTestAgentEnv*)uRsc;
	Dz1SockUtilTransmiterSent* msg = (Dz1SockUtilTransmiterSent*)Dz1TaskSigMsg_getData(*sg);
	if(env->wsock != NULL)
	{
		if (Dz1WebSocket_onSent(env->wsock, msg, errp) == FALSE)
		{
			ERR_OUT(errp);
			Dz1Thread_log(DZ1_THREAD_LOG_CRITICAL, "!!! Transmission Process fail\n");
		}
	}
	return SELECT_BY_ERR(errp, Dz1TaskProcStatus_processed, Dz1TaskProcStatus_error);

}
// Message : DZ1_SOCK_UTIL_RECEIVER_RECEIVED
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Message : DZ1_SOCK_UTIL_RECEIVER_DISCONNECTED
static Dz1TaskProcStatus _ws_disconnected(void* uArg, void* uRsc, void* timer, Dz1TaskSigMsg **sg, Dz1Error *errp)
{
	ConformanceTestAgentEnv* env = (ConformanceTestAgentEnv*)uRsc;
	Dz1Thread_log(DZ1_THREAD_LOG_CRITICAL, "!!! Disconnected with Server\n");
	return Dz1TaskProcStatus_error;
}
// Message : DZ1_SOCK_UTIL_RECEIVER_DISCONNECTED
///////////////////////////////////////////////////////////////////////////////
bool_t ConformanceTestAgentMsgWebSock_msgInit(Dz1Task* task, Dz1Error* err)
{
	DZ1_ERROR_SAFE_PTR(errp, err);
	if (0) {}
	else if ((*errp = Dz1Task_regMsg(task, DZ1_TASK_MSG_EXACT(DZ1_SOCK_UTIL_RECEIVER_RECEIVED), _ws_received)).code) ERR_OUT(errp);
	else if ((*errp = Dz1Task_regMsg(task, DZ1_TASK_MSG_EXACT(DZ1_SOCK_UTIL_TRANSMITER_SENT), _ws_sent)).code) ERR_OUT(errp);
	else if ((*errp = Dz1Task_regMsg(task, DZ1_TASK_MSG_EXACT(DZ1_SOCK_UTIL_RECEIVER_DISCONNECTED), _ws_disconnected)).code) ERR_OUT(errp);
	else Dz1Error_set(errp, 0);
	return ERR_IS_SUCCESS(errp);
}
