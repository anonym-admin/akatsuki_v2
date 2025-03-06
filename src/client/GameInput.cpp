#include "pch.h"
#include "GameInput.h"
#include "Application.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

AkU8 KEY_TABLE[256] =
{
	DIK_ESCAPE			,
	DIK_1				,
	DIK_2				,
	DIK_3				,
	DIK_4				,
	DIK_5				,
	DIK_6				,
	DIK_7				,
	DIK_8				,
	DIK_9				,
	DIK_0				,
	DIK_MINUS			,
	DIK_EQUALS			,
	DIK_BACK			,
	DIK_TAB				,
	DIK_Q				,
	DIK_W				,
	DIK_E				,
	DIK_R				,
	DIK_T				,
	DIK_Y				,
	DIK_U				,
	DIK_I				,
	DIK_O				,
	DIK_P				,
	DIK_LBRACKET		,
	DIK_RBRACKET		,
	DIK_RETURN			,
	DIK_LCONTROL		,
	DIK_A				,
	DIK_S				,
	DIK_D				,
	DIK_F				,
	DIK_G				,
	DIK_H				,
	DIK_J				,
	DIK_K				,
	DIK_L				,
	DIK_SEMICOLON		,
	DIK_APOSTROPHE		,
	DIK_GRAVE			,
	DIK_LSHIFT			,
	DIK_BACKSLASH		,
	DIK_Z				,
	DIK_X				,
	DIK_C				,
	DIK_V				,
	DIK_B				,
	DIK_N				,
	DIK_M				,
	DIK_COMMA			,
	DIK_PERIOD			,
	DIK_SLASH			,
	DIK_RSHIFT			,
	DIK_MULTIPLY		,
	DIK_LMENU			,
	DIK_SPACE			,
	DIK_CAPITAL			,
	DIK_F1				,
	DIK_F2				,
	DIK_F3				,
	DIK_F4				,
	DIK_F5				,
	DIK_F6				,
	DIK_F7				,
	DIK_F8				,
	DIK_F9				,
	DIK_F10				,
	DIK_NUMLOCK			,
	DIK_SCROLL			,
	DIK_NUMPAD7			,
	DIK_NUMPAD8			,
	DIK_NUMPAD9			,
	DIK_SUBTRACT		,
	DIK_NUMPAD4			,
	DIK_NUMPAD5			,
	DIK_NUMPAD6			,
	DIK_ADD				,
	DIK_NUMPAD1			,
	DIK_NUMPAD2			,
	DIK_NUMPAD3			,
	DIK_NUMPAD0			,
	DIK_DECIMAL			,
	DIK_OEM_102			,
	DIK_F11				,
	DIK_F12				,
	DIK_F13				,
	DIK_F14				,
	DIK_F15				,
	DIK_KANA			,
	DIK_ABNT_C1			,
	DIK_CONVERT			,
	DIK_NOCONVERT		,
	DIK_YEN				,
	DIK_ABNT_C2			,
	DIK_PREVTRACK		,
	DIK_AT				,
	DIK_COLON			,
	DIK_UNDERLINE		,
	DIK_KANJI			,
	DIK_STOP			,
	DIK_AX				,
	DIK_UNLABELED		,
	DIK_NEXTTRACK		,
	DIK_NUMPADENTER		,
	DIK_RCONTROL		,
	DIK_MUTE			,
	DIK_CALCULATOR		,
	DIK_PLAYPAUSE		,
	DIK_MEDIASTOP		,
	DIK_VOLUMEDOWN		,
	DIK_VOLUMEUP		,
	DIK_WEBHOME			,
	DIK_NUMPADCOMMA		,
	DIK_DIVIDE			,
	DIK_SYSRQ			,
	DIK_RMENU			,
	DIK_PAUSE			,
	DIK_HOME			,
	DIK_UP				,
	DIK_PRIOR			,
	DIK_LEFT			,
	DIK_RIGHT			,
	DIK_END				,
	DIK_DOWN			,
	DIK_NEXT			,
	DIK_INSERT			,
	DIK_DELETE			,
	DIK_LWIN			,
	DIK_RWIN			,
	DIK_APPS			,
	DIK_POWER			,
	DIK_SLEEP			,
	DIK_WAKE			,
	DIK_WEBSEARCH		,
	DIK_WEBREFRESH		,
	DIK_WEBSTOP			,
	DIK_WEBFORWARD		,
	DIK_WEBBACK			,
	DIK_MYCOMPUTER		,
	DIK_MAIL			,
	DIK_MEDIASELECT		,
};

/*
============================
Game Input (keyboard, mouse)
============================
*/

UGameInput::UGameInput()
{
}

UGameInput::~UGameInput()
{
	CleanUp();
}

AkBool UGameInput::Initialize(UApplication* pApp)
{
	_hWnd = pApp->GetHwnd();

	if (FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&_pDirectInput), nullptr)))
	{
		__debugbreak();
		return AK_FALSE;
	}
	if (FAILED(_pDirectInput->CreateDevice(GUID_SysKeyboard, &_pKeyboardDevice, nullptr)))
	{
		__debugbreak();
		return AK_FALSE;
	}
	if (FAILED(_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
	{
		__debugbreak();
		return AK_FALSE;
	}
	if (FAILED(_pKeyboardDevice->SetCooperativeLevel(_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	AcquireKeyboadrDevice();

	if (FAILED(_pDirectInput->CreateDevice(GUID_SysMouse, &_pMouseDevice, nullptr)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (FAILED(_pMouseDevice->SetDataFormat(&c_dfDIMouse)))
	{
		__debugbreak();
		return AK_FALSE;
	}
	if (FAILED(_pMouseDevice->SetCooperativeLevel(_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	AcquireMouseDevice();

	return AK_TRUE;
}

void UGameInput::Update()
{
	GetCursorPos(&_tMousePos);
	ScreenToClient(_hWnd, &_tMousePos);

	if (!_pMouseDevice || !_pKeyboardDevice)
	{
		__debugbreak();
	}

	if (FAILED(_pKeyboardDevice->GetDeviceState(256, _btKeyState)))
	{
		AcquireKeyboadrDevice();
	}

	if (FAILED(_pMouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), &_tDiMouseState)))
	{
		AcquireMouseDevice();
	}

	for (AkU32 uBtn = 0; uBtn < 3; uBtn++)
	{
		_btMouseState[uBtn] = _tDiMouseState.rgbButtons[uBtn];
	}
	for (AkU32 uBtn = 0; uBtn < 3; uBtn++)
	{
		if (_btMouseState[uBtn] & 0x80)
		{
			if (_btPrevMouseState[uBtn] == KEY_FREE)
			{
				_btMouseState[uBtn] = KEY_PUSH;
			}
			else
			{
				_btMouseState[uBtn] = KEY_HOLD;
			}
		}
		else
		{
			if (_btPrevMouseState[uBtn] == KEY_PUSH || _btPrevMouseState[uBtn] == KEY_HOLD)
			{
				_btMouseState[uBtn] = KEY_UP;
			}
			else
			{
				_btMouseState[uBtn] = KEY_FREE;
			}
		}
	}

	for (AkU32 i = 0; i < KEY_INPUT_NUM; i++)
	{
		if (_btKeyState[KEY_TABLE[i]] & 0x80)
		{
			if (_btPrevKeyState[KEY_TABLE[i]] == KEY_FREE)
			{
				_btKeyState[KEY_TABLE[i]] = KEY_PUSH;
			}
			else
			{
				_btKeyState[KEY_TABLE[i]] = KEY_HOLD;
			}
		}
		else
		{
			if (_btPrevKeyState[KEY_TABLE[i]] == KEY_PUSH || _btPrevKeyState[KEY_TABLE[i]] == KEY_HOLD)
			{
				_btKeyState[KEY_TABLE[i]] = KEY_UP;
			}
			else
			{
				_btKeyState[KEY_TABLE[i]] = KEY_FREE;
			}
		}
	}

	// printf("%d %d\n", (int)_tDiMouseState.lX, (int)_tDiMouseState.lY);
	// printf("%d %d\n", (int)_tMousePos.x, (int)_tMousePos.y);

	_iReleasedMousePosX += _tDiMouseState.lX;
	_iReleasedMousePosY += _tDiMouseState.lY;

	memcpy(_btPrevMouseState, _btMouseState, sizeof(AkU8) * 3);
	memcpy(_btPrevKeyState, _btKeyState, sizeof(AkU8) * KEY_INPUT_NUM);
}

bool UGameInput::LeftBtnDown()
{
	return (_btMouseState[0] == KEY_PUSH);
}

bool UGameInput::RightBtnDown()
{
	return (_btMouseState[1] == KEY_PUSH);
}

bool UGameInput::LeftBtnHold()
{
	return (_btMouseState[0] == KEY_HOLD);
}

bool UGameInput::RightBtnHold()
{
	return (_btMouseState[1] == KEY_HOLD);
}

bool UGameInput::LeftBtnUp()
{
	return (_btMouseState[0] == KEY_UP);
}

bool UGameInput::RightBtnUp()
{
	return (_btMouseState[1] == KEY_UP);
}

bool UGameInput::KeyFirstDown(KEY_INPUT input)
{
	return (_btKeyState[KEY_TABLE[input]] == KEY_PUSH);
}

bool UGameInput::KeyHoldDown(KEY_INPUT input)
{
	return (_btKeyState[KEY_TABLE[input]] == KEY_HOLD);
}

bool UGameInput::KeyUp(KEY_INPUT input)
{
	return (_btKeyState[KEY_TABLE[input]] == KEY_UP);
}

void UGameInput::CleanUp()
{
	if (_pMouseDevice)
	{
		_pMouseDevice->Unacquire();
		_pMouseDevice->Release();
		_pMouseDevice = nullptr;
	}
	if (_pKeyboardDevice)
	{
		_pKeyboardDevice->Unacquire();
		_pKeyboardDevice->Release();
		_pKeyboardDevice = nullptr;
	}
	if (_pDirectInput)
	{
		_pDirectInput->Release();
		_pDirectInput = nullptr;
	}
}

void UGameInput::AcquireKeyboadrDevice()
{
	while (_pKeyboardDevice->Acquire() == DIERR_INPUTLOST)
	{

	}
}

void UGameInput::AcquireMouseDevice()
{
	while (_pMouseDevice->Acquire() == DIERR_INPUTLOST)
	{

	}
}
