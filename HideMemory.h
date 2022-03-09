#pragma once

typedef void(*ENCRYPTDATAPROC)(
	_Out_writes_bytes_all_(_Size) DWORD64 lpAddress,//内存区域起始地址
	_In_ size_t _Size//内存区域大小
	);

typedef void(*DECRYPTDATAPROC)(
	_Out_writes_bytes_all_(_Size) DWORD64 lpAddress,//内存区域起始地址
	_In_ size_t _Size//内存区域大小
	);

typedef struct _PROTECTEDMEMORY
{
	DWORD64 lpAddress;
	SIZE_T dwSize;//0x1000为单位
	BOOL Protected;//True则表示被保护
	BOOL ExecutingProtection;//TRUE表示这段代码要被运行
	ENCRYPTDATAPROC Encrypt;
	DECRYPTDATAPROC Decrypt;

}PROTECTEDMEMORY, * PPROTECTEDMEMORY;

extern std::vector<PROTECTEDMEMORY>PagesOfNoAccessOfData;

/// <summary>
/// 初始化，函数会占用VEH
/// </summary>
/// <returns></returns>
BOOL Init();

/// <summary>
/// 分配一块受保护的供读写内存
/// </summary>
/// <param name="lpAddress">分配的地址，为NULL则自动分配</param>
/// <param name="dwSize">分配的大小，向上取整到0x1000</param>
/// <param name="Encrypt">加密回调，为NULL则用默认加密</param>
/// <param name="Decrypt">解密回调，为NULL则用默认解密</param>
/// <returns>分配的地址</returns>
DWORD64 AllocateHiddenMemory(LPVOID lpAddress, SIZE_T dwSize, ENCRYPTDATAPROC Encrypt, DECRYPTDATAPROC Decrypt);

/// <summary>
/// 释放一块受保护的内存
/// </summary>
/// <param name="lpAddress">内存起始地址</param>
/// <returns></returns>
BOOL FreeHiddenMemory(DWORD64 lpAddress);

