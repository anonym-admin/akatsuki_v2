#include "pch.h"
#include "Controller.h"
#include "GameInput.h"
#include "Player2.h"
#include "Transform.h"
#include "Timer.h"

Controller::Controller(Player* pOwner)
{
	if (!Initialize(pOwner))
	{
		__debugbreak();
	}
}

Controller::~Controller()
{
}

AkBool Controller::Initialize(Player* pOwner)
{
	_pOwner = pOwner;
	return AK_TRUE;
}

void Controller::Update()
{
	KeyBoard();
	Mouse();

	KeyBoardRelease();
}

void Controller::KeyBoard()
{
	// TODO!!...


	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_NONE;

	if (KEY_HOLD(KEY_INPUT_W))
	{
		_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_FRONT_WALK;

		if (_pOwner->BindWeapon)
			_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
		else
			_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_WALK;
	}
	if (KEY_HOLD(KEY_INPUT_S))
	{
		_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_BACK_WALK;

		if (_pOwner->BindWeapon)
			_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
		else
			_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_WALK;
	}
	if (KEY_HOLD(KEY_INPUT_A))
	{
		_pOwner->MoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_LEFT_WALK;

		if (_pOwner->BindWeapon)
			_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
		else
			_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_WALK;
	}
	if (KEY_HOLD(KEY_INPUT_D))
	{
		vDeltaPos = Vector3(1.0f, 0.0f, 0.0f) * DT;
		_pOwner->GetMotionGraph()->SetAnim(ANIM_STATE::R_WALK);
	}
	if (KEY_HOLD(KEY_INPUT_SPACE))
	{
	}
}

void Controller::KeyBoardRelease()
{
	if (KEY_UP(KEY_INPUT_W))
	{
		_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::F_WALK);
		_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::IDLE);
	}
	if (KEY_UP(KEY_INPUT_S))
	{
		_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::F_WALK);
		_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::IDLE);
	}
	if (KEY_UP(KEY_INPUT_A))
	{
		_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::L_WALK);
		_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::IDLE);
	}
	if (KEY_UP(KEY_INPUT_D))
	{
		_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::R_WALK);
		_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::IDLE);
	}
}

void Controller::Mouse()
{
	if (LBTN_DOWN)
	{

	}
	if (LBTN_HOLD)
	{

	}
}
