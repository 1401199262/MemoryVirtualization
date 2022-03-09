#pragma once

typedef void(*ENCRYPTDATAPROC)(
	_Out_writes_bytes_all_(_Size) DWORD64 lpAddress,//�ڴ�������ʼ��ַ
	_In_ size_t _Size//�ڴ������С
	);

typedef void(*DECRYPTDATAPROC)(
	_Out_writes_bytes_all_(_Size) DWORD64 lpAddress,//�ڴ�������ʼ��ַ
	_In_ size_t _Size//�ڴ������С
	);

typedef struct _PROTECTEDMEMORY
{
	DWORD64 lpAddress;
	SIZE_T dwSize;//0x1000Ϊ��λ
	BOOL Protected;//True���ʾ������
	BOOL ExecutingProtection;//TRUE��ʾ��δ���Ҫ������
	ENCRYPTDATAPROC Encrypt;
	DECRYPTDATAPROC Decrypt;

}PROTECTEDMEMORY, * PPROTECTEDMEMORY;

extern std::vector<PROTECTEDMEMORY>PagesOfNoAccessOfData;

/// <summary>
/// ��ʼ����������ռ��VEH
/// </summary>
/// <returns></returns>
BOOL Init();

/// <summary>
/// ����һ���ܱ����Ĺ���д�ڴ�
/// </summary>
/// <param name="lpAddress">����ĵ�ַ��ΪNULL���Զ�����</param>
/// <param name="dwSize">����Ĵ�С������ȡ����0x1000</param>
/// <param name="Encrypt">���ܻص���ΪNULL����Ĭ�ϼ���</param>
/// <param name="Decrypt">���ܻص���ΪNULL����Ĭ�Ͻ���</param>
/// <returns>����ĵ�ַ</returns>
DWORD64 AllocateHiddenMemory(LPVOID lpAddress, SIZE_T dwSize, ENCRYPTDATAPROC Encrypt, DECRYPTDATAPROC Decrypt);

/// <summary>
/// �ͷ�һ���ܱ������ڴ�
/// </summary>
/// <param name="lpAddress">�ڴ���ʼ��ַ</param>
/// <returns></returns>
BOOL FreeHiddenMemory(DWORD64 lpAddress);

