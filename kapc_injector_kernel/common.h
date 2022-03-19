
#include <ntifs.h>

typedef  void KeInitializeApc(
	PRKAPC Apc,
	PRKTHREAD Thread,
	UINT32 Environment,
	PVOID KernelRoutine,
	PVOID RundownRoutine,
	PVOID NormalRoutine,
	KPROCESSOR_MODE ProcessorMode,
	PVOID NormalContext
);
typedef BOOLEAN KeInsertQueueApc(
	PRKAPC Apc,
	PVOID SystemArgument1,
	PVOID SystemArgument2,
	KPRIORITY Increment);
typedef enum _KAPC_ENVIRONMENT
{
	OriginalApcEnvironment,
	AttachedApcEnvironment,
	CurrentApcEnvironment,
	InsertApcEnvironment
}KAPC_ENVIRONMENT, * PKAPC_ENVIRONMENT;
static UNICODE_STRING api1 = RTL_CONSTANT_STRING(L"KeInitializeApc");
static UNICODE_STRING api2 = RTL_CONSTANT_STRING(L"KeInsertQueueApc");
KeInitializeApc* kinitialize_apc;
KeInsertQueueApc* kqueue_apc;
typedef struct kapc_mem {
	char process_name[20];
	unsigned char* shellcode;
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
static bool b_injected = false;
static char* injected_process = NULL;
PVOID kaddr = NULL;
kapc_injector_result k_result;
pkapc_mem xapc_mem = NULL;

namespace offsets {
	static const unsigned int imagefilename_offset = 0x5a8;

};
namespace common {
	inline bool status(NTSTATUS status) { return NT_SUCCESS(status); }
	
	inline PVOID api(UNICODE_STRING api_func) { return MmGetSystemRoutineAddress(&api_func); }
	

}