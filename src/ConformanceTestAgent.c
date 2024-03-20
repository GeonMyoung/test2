#include <dz1_task.h>
#include <dz1_web_socket.h>

#include "ConformanceTestAgentArg.h"
#include "ConformanceTestAgentEnvUtil.h"
#include "ConformanceTestAgentTimerUtil.h"
#include "ConformanceTestAgentMsg.h"

#include <ConformanceTestAgent.h>

static void _task_exception_handler(void* ptr, Dz1Error* reason)
{

	Dz1Thread_log(DZ1_THREAD_LOG_ERROR, "%s : Exception Catch\n", (str_t)ptr);
	ERR_OUT(reason);
}

static Dz1Error ConformanceTestAgent_init(void **uRsc, void *tSelf, str_t name, u32_t my_queue_id, void *uArg, void *timer)
{
	DZ1_ERROR_SAFE_VAR(errp, err);
	ConformanceTestAgentArg *arg = (ConformanceTestAgentArg *)uArg;
	ConformanceTestAgentEnv *env = NULL;

	if (((*uRsc) = env = ConformanceTestAgentEnv_new(tSelf, NULL, TestIdx_unknown, NULL, NULL, 0, NULL, errp)) == NULL) ERR_OUT(errp);
	else
	{
		pthread_cleanup_push(ConformanceTestAgentEnv_delAndSetNull,(void *)uRsc);
		if (0) { }
		else
		{
			if (0) {}
			else if ((env->spec = ConformanceTestAgentMsgDef_genJsonSpec(errp)) == NULL) ERR_OUT(errp);
			//Dz1Task_postSimple(tSelf, DZ1_TASK_QUEUE_NONE, TEST_AGENT_DO_DEFAULT);
			else if (ConformanceTestAgentEnv_establish(env, arg->ws_server, errp) == FALSE) ERR_OUT(errp);
			Dz1Error_set(errp, 0);
		}	
		pthread_cleanup_pop(errp->code); // (TestAgentEnv_delAndSetNull,(void *)uRsc);
	}

	return err;
}

static void ConformanceTestAgent_exception(void *uArg, void *uRsc, void *timer, Dz1Error *reason)
{
	ConformanceTestAgentArg *arg = (ConformanceTestAgentArg *)uArg;
	if (arg->exceptionProc)
		arg->exceptionProc(arg->exceptionArg, reason);
}

static void ConformanceTestAgent_cleanup(void *uRsc)
{
	ConformanceTestAgentEnv *env = (ConformanceTestAgentEnv *)uRsc;
	// Individual Cleanup Code Here
	ConformanceTestAgentEnv_del(env);
}

static Dz1TaskProcStatus _doDefault(void *uArg, void *uRsc, void *timer, void **sg, Dz1Error *errp)
{	// Task work starting point

	ConformanceTestAgentEnv* env = (ConformanceTestAgentEnv*)uRsc;
	if(0) {}
	if (ConformanceTestAgentEnv_send_ready(env, errp) == FALSE) ERR_OUT(errp);
	else Dz1Error_set(errp, 0);

	return SELECT_BY_ERR(errp, Dz1TaskProcStatus_processed, Dz1TaskProcStatus_error);
}

static Dz1TaskProcStatus _send_text(void* uArg, void* uRsc, void* timer, void** sg, Dz1Error* errp)
{
	ConformanceTestAgentEnv* env = (ConformanceTestAgentEnv*)uRsc;
	Dz1Str msg = (Dz1Str)Dz1TaskSigMsg_getData(*sg);
	pthread_cleanup_push(Dz1Str_delAndSetNull, (void*)&msg);
	if (msg == NULL) ERR_OUT(errp);
	else if (ConformanceTestAgentEnv_sendStr(env, msg, errp) == FALSE) ERR_OUT(errp);
	else Dz1Error_set(errp, 0);
	pthread_cleanup_pop(1); // (Dz1Str_delAndSetNull, (void*)&msg);
	return SELECT_BY_ERR(errp, Dz1TaskProcStatus_processed, Dz1TaskProcStatus_error);
}

//static Dz1TaskProcStatus _gen_scenario_task(void *uArg, void* uRsc, void* timer, void **sg, Dz1Error *errp)
//{
//	ConformanceTestAgentEnv* env = (ConformanceTestAgentEnv*)uRsc;
//	void* tScenario = NULL;
//	if (env->testIdx == 0) ERR_SET_OUT(errp, EINVAL);
//	else 
//	{
//		Dz1Thread_printf("testIdx = %d\n", env->testIdx);
//		switch(env->testIdx)
//		{
//		case TestIdx_TrafficLightChk:
//			if ((env->tScenario = TrafficLightChk_new(_task_exception_handler, (str_t)"TrafficLIghtChk", errp)) == NULL) ERR_OUT(errp);
//			//else if ((*errp = Dz1Task_start(env->tScenario, DZ1_THREAD_LOG_BASIS | DZ1_THREAD_LOG_DEBUG)).code) ERR_OUT(errp);
//			// else if () // 생성 완료 메시지 송신
//			else
//			{
//				AgentSendMessage* msg = NULL;
//				pthread_cleanup_push(AgentSendMessage_delAndSetNull, (void*)&msg);
//				if ((msg = AgentSendMessage_new(AgentSendMessagePresent_confirm_testInform, NULL, errp)) == NULL) ERR_OUT(errp);
//				else if ((msg->x.confirm_testInform = ConfirmTestInform_new(env->seq, (u8_t)TestIdx_TrafficLightChk, TRUE, errp)) == NULL) ERR_OUT(errp);
//				else if (ConformanceTestAgentEnv_sendToServer(env, msg, errp) == FALSE) ERR_OUT(errp);
//				Dz1Error_set(errp, 0);
//				pthread_cleanup_pop(1); // (AgentSendMessage_delAndSetNull, (void*)&msg);
//			}
//			break;
//		case TestIdx_Iso14827_2:
//		case TestIdx_TechnicalRegulation1:
//		case TestIdx_TechnicalRegulation2:
//			break;
//		case TestIdx_unknown:
//			break;
//		default:
//			break;
//		}
//		Dz1Error_set(errp, 0);
//	}
//	return SELECT_BY_ERR(errp, Dz1TaskProcStatus_processed, Dz1TaskProcStatus_error);
//}

///////////////////////////////////////////////////////
// WEB MSG
static Dz1TaskProcStatus _rcv_confirm_agent_ready(void* uArg, void* uRsc, void* timer, void** sg, Dz1Error* errp)
{
	ConformanceTestAgentEnv* env = (ConformanceTestAgentEnv*)uRsc;

	return SELECT_BY_ERR(errp, Dz1TaskProcStatus_processed, Dz1TaskProcStatus_error);

}



static Dz1TaskProcStatus _rcv_scenario_msg(void* uArg, void* uRsc, void* timer, void** sg, Dz1Error* errp)
{
	ConformanceTestAgentEnv* env = (ConformanceTestAgentEnv*)uRsc;
	TrafficLightChk_ScenarioSendMessage *msg = (TrafficLightChk_ScenarioSendMessage *)Dz1TaskSigMsg_getData(*sg);
	AgentSendMessage* send_msg = NULL;
	pthread_cleanup_push(AgentSendMessage_delAndSetNull, (void*)&send_msg);
	if ((send_msg = AgentSendMessage_new("main2web", NULL, errp)) == NULL) ERR_OUT(errp);
	else if ((send_msg->context = AgentSendMessageContext_new(AgentSendMessageContextPresent_scenario2web, NULL, errp)) == NULL) ERR_OUT(errp);
	else if ((send_msg->context->x.scenario2web = ScenarioSendMessageContext_new(ScenarioSendMessageContextPresent_traffic_light_chk, NULL, errp)) == NULL) ERR_OUT(errp);
	else if ((send_msg->context->x.scenario2web->x.traffic_light_chk = TrafficLightChk_ScenarioSendMessage_new(msg->present, NULL, errp)) == NULL) ERR_OUT(errp);
	else 
	{
		switch(msg->present)
		{
		case TrafficLightChk_ScenarioSendMessagePresent_notify:
			if ((send_msg->context->x.scenario2web->x.traffic_light_chk->x.notify = TrafficLightChk_ScenarioNotifyMessage_new(msg->x.notify->present, NULL, errp)) == NULL) ERR_OUT(errp);
			else
			{
				switch(msg->x.notify->present)
				{
				case TrafficLightChk_ScenarioNotifyMessagePresent_error:
					if ((send_msg->context->x.scenario2web->x.traffic_light_chk->x.notify->x.error = Dz1Str_clone(msg->x.notify->x.error, errp)) == NULL) ERR_OUT(errp);
					break;
				case TrafficLightChk_ScenarioNotifyMessagePresent_notify:
					if ((send_msg->context->x.scenario2web->x.traffic_light_chk->x.notify->x.notify = Dz1Str_clone(msg->x.notify->x.notify, errp)) == NULL) ERR_OUT(errp);
					break;
				case TrafficLightChk_ScenarioNotifyMessagePresent_warning:
					if ((send_msg->context->x.scenario2web->x.traffic_light_chk->x.notify->x.warning = Dz1Str_clone(msg->x.notify->x.warning, errp)) == NULL) ERR_OUT(errp);
					break;

				}
			}
			break;
			case TrafficLightChk_ScenarioSendMessagePresent_reply:
				if ((send_msg->context->x.scenario2web->x.traffic_light_chk->x.reply = TrafficLightChk_ScenarioResponseMessage_new(msg->x.reply->request_id, msg->x.reply->context, errp)) == NULL) ERR_OUT(errp);
				break;

		}
		if (ConformanceTestAgentEnv_sendToServer(env, send_msg, errp) == FALSE) ERR_OUT(errp);
		else
		{
			msg = NULL;
			Dz1Error_set(errp, 0);
		}
	}
	pthread_cleanup_pop(1); //(AgentSendMessage_delAndSetNull, (void*)&send_msg);


	/*switch(env->testIdx)
	{
	case TestIdx_TrafficLightChk:
		if ((Dz1Task_postSimple(env->tScenario, DZ1_TASK_QUEUE_NONE, CONFORMANCE_TEST_AGENT_RCV_TEST_MSG)).code) ERR_OUT(errp);
		else Dz1Error_set(errp, 0);
		break;
	default:
		break;
	}
	*/
	return SELECT_BY_ERR(errp, Dz1TaskProcStatus_processed, Dz1TaskProcStatus_error);

}

DZ1_CPPLINK bool_t ConformanceTestAgent_sendTextToWebSocket(Dz1Task* task, Dz1Str _text, Dz1Error* err)
{
	DZ1_ERROR_SAFE_PTR(errp, err);
	Dz1Str send_text = NULL;
	//pthread_cleanup_push(Dz1Str_delAndSetNull, (void*)&send_text);				// memory leak check! 20240305 -- gm
	if (task == NULL || _text == NULL) ERR_SET_OUT(errp, EINVAL);
	else if ((send_text = Dz1Str_clone(_text, errp)) == NULL) ERR_OUT(errp);
	else
	{
		//pthread_cleanup_push(Dz1Str_delAndSetNull, (void*)&send_text);
		if ((*errp = Dz1Task_post(task, DZ1_TASK_QUEUE_NONE, CONFORMANCE_TEST_AGENT_SEND_TEXT, (void **)&send_text, NULL, NULL, NULL, NULL, NULL)).code) ERR_OUT(errp);
		else Dz1Error_set(errp, 0);
		//pthread_cleanup_pop(1); // (Dz1Str_delAndSetNull, (void*)send_text);
	}
	return ERR_IS_SUCCESS(errp);
}


void *ConformanceTestAgent_new(Dz1Asn1UTF8Str *ws_server_uri, ConformanceTestAgentException e, void *eArg, Dz1Error *err)
{
	void *ret = NULL;
	DZ1_ERROR_SAFE_PTR(errp, err);
	ConformanceTestAgentArg* arg = NULL;

	if (ws_server_uri == NULL || ws_server_uri->data == NULL || ws_server_uri->size == 0) ERR_SET_OUT(errp, EINVAL);
	else if ((arg = ConformanceTestAgentArg_new(NULL, e, eArg, errp)) == NULL) ERR_OUT(errp);
	else
	{
		pthread_cleanup_push(ConformanceTestAgentArg_delAndSetNull, (void *)&arg);
		if ((arg->ws_server = Dz1Asn1UTF8Str_clone(ws_server_uri, errp)) == NULL) ERR_OUT(errp);
		else if ((ret = Dz1Task_new("ConformanceTestAgent" , 1024,
							   (void **)&arg, (Dz1DelFunc)ConformanceTestAgentArg_del,
							   (Dz1CmpFunc)ConformanceTestAgentTimer_cmp,
								ConformanceTestAgent_init,
								ConformanceTestAgent_exception,
								ConformanceTestAgent_cleanup, errp)) == NULL) ERR_OUT(errp);
		else
		{
			pthread_cleanup_push(Dz1Task_delAndSetNull, (void *)&ret);
			if (0) {}
			else if ((*errp = Dz1Task_regMsg(ret, DZ1_TASK_MSG_EXACT(CONFORMANCE_TEST_AGENT_DO_DEFAULT), _doDefault)).code) ERR_OUT(errp);
			else if ((*errp = Dz1Task_regMsg(ret, DZ1_TASK_MSG_EXACT(CONFORMANCE_TEST_AGENT_SEND_TEXT), _send_text)).code) ERR_OUT(errp);
			// WEB MSG
			else if ((*errp = Dz1Task_regMsg(ret, DZ1_TASK_MSG_EXACT(CONFORMANCE_TEST_AGENT_RCV_CNF_AGENT_READY), _rcv_confirm_agent_ready)).code) ERR_OUT(errp);
			//else if ((*errp = Dz1Task_regMsg(ret, DZ1_TASK_MSG_EXACT(CONFORMANCE_TEST_AGENT_RCV_TEST_MSG), _rcv_scenario_msg)).code) ERR_OUT(errp);
			// SCENARIO
			else if ((*errp = Dz1Task_regMsg(ret, DZ1_TASK_MSG_EXACT(CONFORMANCE_TEST_AGENT_SCENARIO_TO_WEB_MSG), _rcv_scenario_msg)).code) ERR_OUT(errp);
			else if (ConformanceTestAgentMsgWebSock_msgInit(ret, errp) == FALSE) ERR_OUT(errp);
			else
			{
				Dz1Error_set(errp, 0);
			}
			pthread_cleanup_pop(errp->code); // (Dz1Task_delAndSetNull, (void *)&ret);
		}
		pthread_cleanup_pop(1); // (TestAgentArg_delAndSetNull, (void *)&arg);
	}
	return ret;
}

void ConformanceTestAgent_del(void *task)
{
	DZ1_ERROR_SAFE_VAR(errp, err);

	if (task == NULL) return;
	else if ((*errp = Dz1Task_del(task)).code) ERR_OUT(errp);
}
