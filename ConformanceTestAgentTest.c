#include <dz1_cleanup.h>
#include <dz1_thread_log.h>
#include <dz1_usleep.h>
#include <dz1_shell.h>
#include <dz1_str.h>
#include <dz1_socket.h>
#include <dz1_socket_ssl.h>
#include <ConformanceTestAgent.h>

#ifndef UNIX_SYSTEM
#include <TCHAR.h>
#endif // UNIX_SYSTEM

////////////////////////////////////////////////////////////////////////////////
// Log-File
static FILE *logfp = NULL;
static void _outputHook(void *ptr, unsigned char *data, size_t size)
{
	if (logfp != NULL)
	{
		fwrite(data, size, 1, logfp);
		fflush(logfp);
	}
}
static void _memoryExport(void *ptr) { if (logfp) Dz1Mem_export(logfp, TRUE); }
// Log-File
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Test-Env
typedef struct TestEnv
{
	void					*task;
	Dz1Asn1UTF8Str			*server_uri;
} TestEnv;
static void TestEnv_cleanup(void *ptr)
{
	TestEnv *env = (TestEnv *)ptr;
	Dz1Asn1UTF8Str_delAndSetNull(&env->server_uri);
	Dz1Task_delAndSetNull(&env->task);
}
// Test-Env
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// cmd_send
static bool_t _cmd_send(Dz1Task *task, Dz1Str send_text, Dz1Error *errp)
{
	if (send_text == NULL) ERR_OUT(errp);
	else if ((ConformanceTestAgent_sendTextToWebSocket(task, send_text, errp)) == FALSE) ERR_OUT(errp);
	else Dz1Error_set(errp, 0);
	return ERR_IS_SUCCESS(errp);
}
static void cmd_send_usage(Dz1Str n)
{
	Dz1Thread_printf(Dz1Text("Usage] %s <send_str>\n"), n);
}

static Dz1ShellCmdReturn cmd_send(void *ptr, int argc, Dz1Str argv[])
{
	DZ1_ERROR_SAFE_VAR(errp, err);
	void (*_print_usage)(Dz1Str n) = cmd_send_usage;
	Dz1Str n = argv[0];
	Dz1Str send_str = argv[1];
	TestEnv* env = (TestEnv*)ptr;

	if (argc != 2) _print_usage(n);
	else if (_cmd_send(env->task, send_str, errp) == FALSE) ERR_OUT(errp);
	else Dz1Error_set(errp, 0);
	return Dz1ShellCmdReturn_ok;
}
// cmd_send
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Shell
static void _shell_start(void *task)
{
	void *sh = NULL;
	DZ1_ERROR_SAFE_VAR(errp, err);

#ifdef WIN32
	if ((sh = Dz1Shell_new(Dz1Text("win32crt"), DZ1_SHELL_CMD_ALL, 16, Dz1Text("root"), Dz1Text("ConformanceTestAgent"), errp)) == NULL) ERR_OUT(errp);
#else
	if ((sh = Dz1Shell_new(Dz1Text("xterm"), DZ1_SHELL_CMD_ALL, 16, Dz1Text("root"), Dz1Text("ConformanceTestAgent"), errp)) == NULL) ERR_OUT(errp);
#endif
	else
	{
		pthread_cleanup_push((Dz1DelFunc)Dz1Shell_del, (void *)sh);
		if (0) { }
		//else if ((*errp = Dz1Shell_cmdReg(sh, Dz1Text("command_name"), command_func, task, Dz1Text("command_description") )).code) ERR_OUT(errp);
		else if ((*errp = Dz1Shell_cmdReg(sh, Dz1Text("send"), cmd_send, task, Dz1Text("Message Send/Receive test"))).code) ERR_OUT(errp);
		else
		{
			Dz1Shell_start(sh, NULL);
		}

		pthread_cleanup_pop(1); // ((Dz1DelFunc)Dz1Shell_del, (void *)sh);
	}
}
// Shell
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Process-Entry
static Dz1Str server_uri = NULL;
static void _task_exception_handler(void *ptr, Dz1Error *reason)
{
	Dz1Thread_log(DZ1_THREAD_LOG_ERROR, "ConformanceTestAgent : Exception Catch\n");
	ERR_OUT(reason);
}

static bool_t _main_start(bool_t is_daemon, Dz1Error* errp)
{
	TestEnv env = { NULL, NULL };
	pthread_cleanup_push(TestEnv_cleanup, (void*)&env);
	if ((env.server_uri = Dz1Asn1UTF8Str_newFromStr(server_uri, errp)) == NULL) ERR_OUT(errp);
	else if ((env.task = ConformanceTestAgent_new(env.server_uri, _task_exception_handler, NULL, errp)) == NULL) ERR_OUT(errp);
	else if ((*errp = Dz1Task_start(env.task, DZ1_THREAD_LOG_BASIS | DZ1_THREAD_LOG_DEBUG)).code) ERR_OUT(errp);
	else
	{
		//pthread_cleanup_push(Dz1Task_startCancel, (void*)&env.task);
		if (is_daemon) { while (1) Dz1Thread_usleep(1000000); }
		else _shell_start((void*)&env);

		//pthread_cleanup_pop(1) // (Dz1Task_startCancel, (void*)&env.task);
	}
	pthread_cleanup_pop(1); // (TestEnv_cleanup, (void*)&env);

#ifdef _DEBUG
	if(is_daemon == FALSE)
	{
		Dz1Thread_printf("press any key...\n");
		Dz1Thread_getch(-1, NULL);
	}
#endif // _DEBUG
	return errp->code == 0 ? TRUE : FALSE;
}

static bool_t _process_start(bool_t is_daemon, Dz1Error *errp)
{
	Dz1Cleanup_init();
	Dz1Cleanup_setMemoryExporter(_memoryExport, NULL);
	if ((logfp = Dz1FileStream_open(Dz1Text("ConformanceTestAgentTest.log.txt"), Dz1Text("wb"), NULL)) != NULL)
	{
		fwrite(DZ1_SYSTEM_CHARSET_BOM, DZ1_SYSTEM_CHARSET_BOM_SZ, 1, logfp);
		fflush(logfp);
		Dz1OutputHook_set(_outputHook, NULL);
	}

	if (Dz1Socket_init() == FALSE) { Dz1Thread_log(DZ1_THREAD_LOG_CRITICAL, Dz1Text("Win sock Initialize Fail\n")); Dz1Error_set(errp, EFAULT); }
	else
	{
		pthread_cleanup_push(Dz1Socket_initCancel, NULL);
		if ((*errp = Dz1SocketCallback2_init()).code) ERR_OUT(errp);
		else
		{
			pthread_cleanup_push(Dz1SocketCallback2_initCancel, NULL);
			if ((*errp = Dz1SocketCallback2SSL_init()).code) ERR_OUT(errp);
			else
			{
				pthread_cleanup_push(Dz1SocketCallback2SSL_initCancel, NULL);
				if (_main_start(is_daemon, errp) == FALSE) ERR_OUT(errp);
				else Dz1Error_set(errp, 0);

				pthread_cleanup_pop(1); // (Dz1SocketCallback2SSL_initCancel, NULL);
			}
			pthread_cleanup_pop(1); // (Dz1SocketCallback2_initCancel, NULL);
		}
		pthread_cleanup_pop(1) // (Dz1Socket_initCancel, NULL);
		Dz1Socket_cleanup();
	}

#ifdef _DEBUG
	if (is_daemon == FALSE)
	{
		Dz1Thread_printf("press any key...\n");
		Dz1Thread_getch(-1, NULL);
	}
#endif // _DEBUG
	return errp->code == 0 ? TRUE : FALSE;
}
// Process-Entry
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Main-Entry
#ifndef UNIX_SYSTEM
int _tmain(int argc, TCHAR *argv[])
#else // UNIX_SYSTEM
int main(int argc, char *argv[])
#endif // UNIC_SYSTEM
{
	DZ1_ERROR_SAFE_VAR(errp, err);

	// parse argv, choice fork or shell
	if(argc!=2 && argc !=3)
	{
		Dz1Str bn = basename(argv[0]);
		fprintf(stderr, "%s <WebSocketServer> [-debug]\n", bn);
	}
	else 
	{
		bool_t	daemon_mode = TRUE;
		server_uri = argv[1];
		if (argc == 3 && strcmp(argv[2], "-debug") == 0) daemon_mode = FALSE;
		_process_start(daemon_mode, errp);

	}
	return errp->code;
}
// Main-Entry
////////////////////////////////////////////////////////////////////////////////
