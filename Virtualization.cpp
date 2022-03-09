#include "globals.h"
#include "HideMemory.h"

DWORD64 hidemem;

typedef struct _ImprotantData
{
	char dat[0x999];
}ImprotantData;

//如果只想读写的话可以删除这个锁,并打开在VEH处理函数内的锁, 要执行的话可能会在执行时触发双重异常,所以在这里加锁
//std::mutex m;

typedef NTSTATUS (NTAPI* _NtClose)(IN HANDLE ObjectHandle);
typedef NTSTATUS (NTAPI* _NtReadVirtualMemory)(HANDLE ProcessHandle,PVOID BaseAddress,PVOID Buffer,ULONG NumberOfBytesToRead,PULONG NumberOfBytesReaded);

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#define STATUS_INVALID_HANDLE            ((NTSTATUS)0xC0000008L)   

BOOL CheckSEH()
{
	__try
	{
		std::cout << **(DWORD64**)hidemem << std::endl;		
	}
	__except (1)
	{
		return TRUE;
	}
	return FALSE;
}

//BOOL ExecuteHiddenMemory()
//{
//	//todo test long jump
//
//
//	auto ntdll = GetModuleHandle(L"ntdll.dll");
//	//auto NtClose = GetProcAddress(ntdll, "NtClose");
//	auto NtReadVirtualMemory = GetProcAddress(ntdll, "NtReadVirtualMemory");
//	
//	auto NtReadVirtualMemory_index = *(LONG*)((DWORD64)NtReadVirtualMemory + 4);
//
//	char NtReadShell[0x50] = { 0 };
//	*(UINT*)(NtReadShell) = 0xB8D18B4C;			//mov r10,rcx	mov eax,
//	*(UINT*)(NtReadShell + 4) = NtReadVirtualMemory_index;			 // index
//	*(USHORT*)(NtReadShell + 8) = (USHORT)0x050F;			//syscall
//	*(UCHAR*)(NtReadShell + 10) = (UCHAR)0xC3;				//ret
//	memcpy((PVOID)(hidemem + 0x10), NtReadShell, 0x50);
//
//	auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, GetCurrentProcessId());
//	char Buf[0x27] = { 0 };
//	
//	if (((_NtReadVirtualMemory)(hidemem + 0x10))(hProcess, (PVOID)((DWORD64)ntdll + 0x4E), Buf, 0x26, 0) == STATUS_SUCCESS)
//	{
//		if (strcmp(Buf, "This program cannot be run in DOS mode") == 0)//Should Be this.
//		{
//			CloseHandle(hProcess);
//			return TRUE;
//		}
//	}	
//	CloseHandle(hProcess);
//	return FALSE;
//}

BOOL IsMemoryHidden()
{
	MEMORY_BASIC_INFORMATION MemInfo = { 0 };
	VirtualQuery((LPVOID)hidemem, &MemInfo, sizeof(MemInfo));
	if (MemInfo.Protect != PAGE_NOACCESS)
	{		
		return FALSE;
	}
	return TRUE;
}

void ReadThreadProc2()
{
	while (1)
	{
		ULONGLONG tick = GetTickCount64();
		//m.lock();
		printf("Thread2 ReadTime %llu ms Data:%llx\n", GetTickCount64() - tick, *(DWORD64*)hidemem);		
		//m.unlock();
		Sleep(100);		
	}
}

void ReadThreadProc3()
{
	while (1)
	{
		ULONGLONG tick = GetTickCount64();
		//m.lock();
		printf("Thread3 ReadTime %llu ms Data:%llx\n", GetTickCount64() - tick, *(DWORD64*)hidemem);
		//m.unlock();
		Sleep(100);
	}
}


int main()
{
	WCHAR title[64];
	_snwprintf_s(title, sizeof(title), L"PID: %lx", GetCurrentProcessId());
	SetConsoleTitleW(title);
	
	Init();
	hidemem = AllocateHiddenMemory(NULL, 1, 
		[](DWORD64 lpAddress, size_t _Size) {
			for (int i = 0; i < _Size; i++)
			{
				((char*)lpAddress)[i] += (char)6;
				((char*)lpAddress)[i] = ((char*)lpAddress)[i] ^ 'a';
			}
		},
		[](DWORD64 lpAddress, size_t _Size) {
			for (int i = 0; i < _Size; i++)
			{
				((char*)lpAddress)[i] = ((char*)lpAddress)[i] ^ 'a';
				((char*)lpAddress)[i] -= (char)6;
			} 
		});	

	std::thread ReadThread1(ReadThreadProc2);
	std::thread ReadThread2(ReadThreadProc3);

	//BOOL MessageBoxState = TRUE;

	while (1)
	{
		printf("Allocated %llx\n\n", hidemem);

		ULONGLONG tick = GetTickCount64();

		//R/W ==========================================================================================
		//m.lock();
		*(DWORD64*)hidemem += 1;
		printf("ReadWriteTime %llu ms Data:%llx\n", GetTickCount64() - tick, *(DWORD64*)hidemem);
		//m.unlock();
		//R/W ==========================================================================================

		//Ptr
		//m.lock();
		//*(DWORD64*)((DWORD64)hidemem + 8) = (DWORD64)&MessageBox;
		//printf("Bytes In MessageBoxA %llx\n", **(DWORD64**)((DWORD64)hidemem + 8));
		//if(MessageBoxState)
		//	MessageBoxState = ((decltype(&MessageBox))(*(DWORD64*)((DWORD64)hidemem + 8)))(0, L"Continue Executing MessageBox?", L"LOL", MB_YESNO) == IDNO ? FALSE : TRUE;
		//m.unlock();


		//Execute =======================================================================================
		//tick = GetTickCount64(); 请查看当前文件头部的锁
		//m.lock();
		//if (ExecuteHiddenMemory())
		//	printf("ExecuteTime   %llu ms \n", GetTickCount64() - tick);
		//else
		//	printf("Execute Failed\n");
		//m.unlock();
		//Execute =======================================================================================


		//SEH ===========================================================================================
		tick = GetTickCount64();
		//m.lock();
		if(CheckSEH())
			printf("Support SEH   %llu ms\n", GetTickCount64() - tick);
		//m.unlock();
		//SEH ===========================================================================================


		//For Safety ====================================================================================
		//m.lock();
		if (!IsMemoryHidden())
		{
			ReadThread1.detach();
			ReadThread2.detach();

			//m.unlock();
			std::cout << "Error: DONT debug the critical R/W/X Code" << std::endl;
			system("pause");
			exit(0);
		}
		//m.unlock();
		//For Safety ====================================================================================

		Sleep(200);
		system("cls");
	}

END:
	//m.lock();
	FreeHiddenMemory(hidemem);
	//m.unlock();
}



//int main()
//{
//    ZyanU8 data[] =
//    {
//        0x51, 0x8D, 0x45, 0xFF, 0x50, 0xFF, 0x75, 0x0C, 0xFF, 0x75,
//        0x08, 0xFF, 0x15, 0xA0, 0xA5, 0x48, 0x76, 0x85, 0xC0, 0x0F,
//        0x88, 0xFC, 0xDA, 0x02, 0x00
//    };
//
//    // Initialize decoder context
//    
//    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
//
//    // Initialize formatter. Only required when you actually plan to do instruction
//    // formatting ("disassembling"), like we do here
//    ZydisFormatter formatter;
//    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);
//
//    // Loop over the instructions in our buffer.
//    // The runtime-address (instruction pointer) is chosen arbitrary here in order to better
//    // visualize relative addressing
//    ZyanU64 runtime_address = 0x007FFFFFFF400000;
//    ZyanUSize offset = 0;
//    const ZyanUSize length = sizeof(data);
//    ZydisDecodedInstruction instruction;
//    while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, data + offset, length - offset,
//        &instruction)))
//    {
//        // Print current instruction pointer.
//        printf("%016" PRIX64 "  ", runtime_address);
//
//        // Format & print the binary instruction structure to human readable format
//        char buffer[256];
//        ZydisFormatterFormatInstruction(&formatter, &instruction, buffer, sizeof(buffer),
//            runtime_address);
//        puts(buffer);
//
//        offset += instruction.length;
//        runtime_address += instruction.length;
//    }
//}
