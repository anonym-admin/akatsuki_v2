#pragma once

struct CmdListContainer_t
{
	ID3D12CommandAllocator* pCmdAllocator = nullptr;
	ID3D12GraphicsCommandList* pCmdList = nullptr;
	List_t tLink = {};
	AkBool bClosed = AK_FALSE;
};

/*
====================
FCommandListPool
====================
*/

class FCommandListPool
{
public:
	FCommandListPool();
	~FCommandListPool();

	AkBool Initialize(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE eCmdListType, AkU32 uMaxCmdListNum);
	ID3D12GraphicsCommandList* GetCurrentCmdList();
	void Close();
	void CloseAndExecute(ID3D12CommandQueue* pCmdQueue);
	void Reset();

private:
	void CleanUp();
	AkBool AddCmdList();
	CmdListContainer_t* Alloc();

private:
	ID3D12Device* _pDevice = nullptr;
	CmdListContainer_t* _pCurCmdList = nullptr;
	D3D12_COMMAND_LIST_TYPE _eCmdListTpye = {};
	List_t* _pAvailableCmdListHead = nullptr;
	List_t* _pAvailableCmdListTail = nullptr;
	List_t* _pAllocatedCmdListHead = nullptr;
	List_t* _pAllocatedCmdListTail = nullptr;
	AkU32 _uAvailableCmdListNum = 0;
	AkU32 _uAllocatedCmdListNum = 0;
	AkU32 _uMaxCmdListNum = 0;
	AkU32 _uTotalCmdListNum = 0;
};

