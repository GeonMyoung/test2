#include <dz1_web_socket.h>
#include "ConformanceTestAgentEnvUtil.h"

// gm
DZ1_CPPLINK bool_t ConformanceTestAgentEnv_sendStr(ConformanceTestAgentEnv* env, Dz1Str str, Dz1Error* err)
{
	DZ1_ERROR_SAFE_PTR(errp, err);

	Dz1Asn1UTF8Str* send_text = NULL;

	if ((send_text = Dz1Asn1UTF8Str_newFromStr(str, errp)) == NULL) ERR_OUT(errp);
	else
	{
		pthread_cleanup_push(Dz1Asn1UTF8Str_delAndSetNull, (void*)&send_text);

		if (Dz1WebSocket_sendText(env->wsock, send_text, errp) == FALSE) ERR_OUT(errp);
		else Dz1Error_set(errp, 0);

		pthread_cleanup_pop(1); // (Dz1Asn1UTF8Str_delAndSetNull, (void *)&my_type);
	}
	return ERR_IS_SUCCESS(errp);
}

bool_t ConformanceTestAgentEnv_establish(ConformanceTestAgentEnv* env, Dz1Asn1UTF8Str* ws_server_uri, Dz1Error* errp)
{
	u32_t my_queue_id = Dz1Task_getQueueID(env->tSelf);
	if (0) {}
	else if ((env->wsock = Dz1WebSocket_createU(ws_server_uri, my_queue_id, TRUE, errp)) == NULL) ERR_OUT(errp);
	else Dz1Error_set(errp, 0);

	return ERR_IS_SUCCESS(errp);
}

bool_t ConformanceTestAgentEnv_send_MyType(ConformanceTestAgentEnv* env, Dz1Error* err)
{
	DZ1_ERROR_SAFE_PTR(errp, err);

	Dz1Asn1UTF8Str* my_type = NULL;

	if ((my_type = Dz1Asn1UTF8Str_newFromStr(Dz1T("{ \"type\": \"agent\" }"), errp)) == NULL) ERR_OUT(errp);
	else
	{
		pthread_cleanup_push(Dz1Asn1UTF8Str_delAndSetNull, (void*)&my_type);

		if (Dz1WebSocket_sendText(env->wsock, my_type, errp) == FALSE) ERR_OUT(errp);
		else Dz1Error_set(errp, 0);

		pthread_cleanup_pop(1); // (Dz1Asn1UTF8Str_delAndSetNull, (void *)&my_type);
	}
	return ERR_IS_SUCCESS(errp);
}

bool_t ConformanceTestAgentEnv_send_ready(ConformanceTestAgentEnv* env, Dz1Error* err)
{
	DZ1_ERROR_SAFE_PTR(errp, err);
	AgentSendMessage* msg = AgentSendMessage_new("main2web", NULL, errp);
	if (msg == NULL) ERR_OUT(errp);
	else
	{
		pthread_cleanup_push(AgentSendMessage_delAndSetNull, (void*)&msg);
		if ((msg->context = AgentSendMessageContext_new(AgentSendMessageContextPresent_main2web, NULL, errp)) == NULL) ERR_OUT(errp);
		else if ((msg->context->x.main2web = MainTasktoWebMsg_new(MainTasktoWebMsgPresent_notify_agentReady, NULL, errp)) == NULL) ERR_OUT(errp);
		else if ((msg->context->x.main2web->x.notify_agentReady = NotifyAgentReady_new(env->seq++, Dz1T("Test Agent Ready"), errp)) == NULL) ERR_OUT(errp);
		else if (ConformanceTestAgentEnv_sendToServer(env, msg, errp) == FALSE) ERR_OUT(errp);
		else Dz1Error_set(errp, 0);

		pthread_cleanup_pop(1); // (AgentSendMessage_delAndSetNull, (void*)&msg);
	}
	return ERR_IS_SUCCESS(errp);
}

bool_t ConformanceTestAgentEnv_sendToServer(ConformanceTestAgentEnv* env, AgentSendMessage* msg, Dz1Error* err)
{
	DZ1_ERROR_SAFE_PTR(errp, err);
	Dz1Json* json = NULL;

	u32_t log_lv = 0;
	Dz1Thread_getLogLevel(Dz1Thread_self(), &log_lv);

	if (log_lv & DZ1_THREAD_LOG_DEBUG)
	{
		_Dz1Thread_log("Send Message = ");
		AgentSendMessage_dump(msg, 0);
	}

	if ((json = Dz1Json_new(DZ1_SYSTEM_CHARSET, env->spec, "AgentSendMessage", errp)) == FALSE) ERR_OUT(errp);
	else
	{
		Dz1Str src = NULL;
		pthread_cleanup_push(Dz1Json_delAndSetNull, (void*)&src);
		if (Dz1Json_stringify(json, Dz1T("AgentSendMessage"), (void*)msg, Dz1JsonNewLine_continuous, errp) == FALSE) ERR_OUT(errp);
		else if ((src = Dz1Json_stringified(json, errp)) == NULL) ERR_OUT(errp);
		else
		{
			Dz1Asn1UTF8Str* str_u8 = NULL;
			pthread_cleanup_push(Dz1Str_delAndSetNull, (void*)&src);
			if ((str_u8 = Dz1Asn1UTF8Str_newFromStr(src, errp)) == NULL) ERR_OUT(errp);
			else
			{
				pthread_cleanup_push(Dz1Asn1UTF8Str_delAndSetNull, (void*)&str_u8);
				if (Dz1WebSocket_sendText(env->wsock, str_u8, errp) == FALSE) ERR_OUT(errp);
				else Dz1Error_set(errp, 0);
				pthread_cleanup_pop(1); // (Dz1Asn1UTF8Str_delAndSetNull, (void*)&str_u8);
			}
			pthread_cleanup_pop(1); // (Dz1Str_delAndSetNull, (void*)&src);
		}
		pthread_cleanup_pop(1); // (Dz1Json_delAndSetNull, (void*)&src);
	}

	return ERR_IS_SUCCESS(errp);
}
