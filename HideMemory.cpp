#include "globals.h"
#include "HideMemory.h"

std::vector<PROTECTEDMEMORY>PagesOfNoAccessOfData;

//只想读写的话可以打开这个锁
std::mutex m;

//DeFault Encrypt/Decrypt
void EncryptData(DWORD64 lpAddress, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		((char*)lpAddress)[i] = ((char*)lpAddress)[i] ^ 'b';
	}
}

LONG NTAPI VehExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	DWORD OldProtect = 0;
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
	{
		for (auto it = PagesOfNoAccessOfData.begin(); it != PagesOfNoAccessOfData.end(); it++)
		{
			if (ExceptionInfo->ExceptionRecord->ExceptionInformation[1] <= (*it).lpAddress + (*it).dwSize &&
				ExceptionInfo->ExceptionRecord->ExceptionInformation[1] >= (*it).lpAddress)
			{
				m.lock();
				//Restore Page Protection and Decrypt
				(*it).Protected = FALSE;//Not Protected
				VirtualProtect((LPVOID)(*it).lpAddress, (*it).dwSize, PAGE_EXECUTE_READWRITE, &OldProtect);

				if ((*it).Decrypt)
					(*it).Decrypt((*it).lpAddress, (*it).dwSize);
				else
					EncryptData((*it).lpAddress, (*it).dwSize);//Decrypt

				ExceptionInfo->ContextRecord->EFlags |= 0x100;//Do a single step
				return EXCEPTION_CONTINUE_EXECUTION;	

			}
		}

	}

	if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
	{
		for (auto it = PagesOfNoAccessOfData.begin(); it != PagesOfNoAccessOfData.end(); it++)
		{
			if ((*it).Protected == FALSE)
			{
				if ((*it).Encrypt)
					(*it).Encrypt((*it).lpAddress, (*it).dwSize);
				else
					EncryptData((*it).lpAddress, (*it).dwSize);//加密

				VirtualProtect((LPVOID)(*it).lpAddress, (*it).dwSize, PAGE_NOACCESS, &OldProtect);
				m.unlock();
				return EXCEPTION_CONTINUE_EXECUTION;
			}
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;

}

BOOL Init()
{
	return AddVectoredExceptionHandler(TRUE, VehExceptionHandler) == NULL ? FALSE : TRUE;
}

DWORD64 AllocateHiddenMemory(LPVOID lpAddress, SIZE_T dwSize,ENCRYPTDATAPROC Encrypt, DECRYPTDATAPROC Decrypt)
{
	PVOID allocated = VirtualAlloc(lpAddress, dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (allocated != NULL)
	{
		DWORD OldProtect = 0;
		dwSize = ((dwSize-1) & 0xfffffffffffff000) + 0x1000;//size是0 就走不到这里

		PROTECTEDMEMORY protected_mem = { 0 };
		protected_mem.lpAddress = (DWORD64)allocated;
		protected_mem.dwSize = dwSize;
		protected_mem.ExecutingProtection = FALSE;
		protected_mem.Protected = TRUE;
		protected_mem.Encrypt = Encrypt;
		protected_mem.Decrypt = Decrypt;


		if(!Encrypt)
			EncryptData((DWORD64)allocated, dwSize);
		else
			Encrypt((DWORD64)allocated, dwSize);

		PagesOfNoAccessOfData.push_back(protected_mem);

		VirtualProtect(allocated, dwSize, PAGE_NOACCESS, &OldProtect);
	}
	
	return (DWORD64)allocated;
}

BOOL FreeHiddenMemory(DWORD64 lpAddress)
{
	for (auto it = PagesOfNoAccessOfData.begin(); it != PagesOfNoAccessOfData.end(); it++)
	{
		if ((*it).lpAddress == lpAddress)
		{
			PagesOfNoAccessOfData.erase(it);
			return VirtualFree((LPVOID)lpAddress, 0, MEM_RELEASE);
		}
	}
	return FALSE;
}
