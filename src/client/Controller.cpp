#include "pch.h"
#include "Controller.h"
#include "GameInput.h"
#include "Player.h"
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

		if (KEY_HOLD(KEY_INPUT_LSHIFT))
		{
			_pOwner->SpeedUp = AK_TRUE;
			_pOwner->SetAnimation(ANIM_STATE::PLAYER_ANIM_STATE_RUN);
		}
		else
		{
			if (_pOwner->BindWeapon)
				_pOwner->SetAnimation(ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK);
			else
				_pOwner->SetAnimation(ANIM_STATE::PLAYER_ANIM_STATE_WALK);
		}
	}
	//if (KEY_HOLD(KEY_INPUT_S))
	//{
	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_BACK_WALK;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_WALK;
	//}
	//if (KEY_HOLD(KEY_INPUT_A))
	//{
	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_LEFT_WALK;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_WALK;
	//}
	//if (KEY_HOLD(KEY_INPUT_D))
	//{
	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_RIGHT_WALK;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_WALK;
	//}
	//if (KEY_HOLD(KEY_INPUT_W) && KEY_HOLD(KEY_INPUT_A))
	//{
	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_FRONT_WALK;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_WALK;
	//}
	//if (KEY_HOLD(KEY_INPUT_W) && KEY_HOLD(KEY_INPUT_D))
	//{
	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_FRONT_WALK;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_WALK;
	//}
	//if (KEY_HOLD(KEY_INPUT_S) && KEY_HOLD(KEY_INPUT_A))
	//{
	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_WALK;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_WALK;
	//}
	//if (KEY_HOLD(KEY_INPUT_S) && KEY_HOLD(KEY_INPUT_D))
	//{
	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIHGT_BACK_WALK;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_WALK;
	//}
	//// 2. run
	//else if (KEY_HOLD(KEY_INPUT_W) && KEY_HOLD(KEY_INPUT_LSHIFT))
	//{
	//	_pOwner->SpeedUp = AK_TRUE;

	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_FRONT_RUN;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->SetAnimation(ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN);
	//	else
	//		_pOwner->SetAnimation(ANIM_STATE::PLAYER_ANIM_STATE_RUN);
	//}
	//if (KEY_HOLD(KEY_INPUT_S) && KEY_HOLD(KEY_INPUT_LSHIFT))
	//{
	//	_pOwner->SpeedUp = AK_TRUE;

	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_BACK_RUN;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RUN;
	//}
	//if (KEY_HOLD(KEY_INPUT_A) && KEY_HOLD(KEY_INPUT_LSHIFT))
	//{
	//	_pOwner->SpeedUp = AK_TRUE;

	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_LEFT_RUN;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RUN;
	//}
	//if (KEY_HOLD(KEY_INPUT_D) && KEY_HOLD(KEY_INPUT_LSHIFT))
	//{
	//	_pOwner->SpeedUp = AK_TRUE;

	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_RIGHT_RUN;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RUN;
	//}
	//if (KEY_HOLD(KEY_INPUT_W) && KEY_HOLD(KEY_INPUT_D) && KEY_HOLD(KEY_INPUT_LSHIFT))
	//{
	//	_pOwner->SpeedUp = AK_TRUE;

	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_FRONT_RUN;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RUN;
	//}
	//if (KEY_HOLD(KEY_INPUT_W) && KEY_HOLD(KEY_INPUT_A) && KEY_HOLD(KEY_INPUT_LSHIFT))
	//{
	//	_pOwner->SpeedUp = AK_TRUE;

	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_FRONT_RUN;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RUN;
	//}
	//if (KEY_HOLD(KEY_INPUT_S) && KEY_HOLD(KEY_INPUT_D) && KEY_HOLD(KEY_INPUT_LSHIFT))
	//{
	//	_pOwner->SpeedUp = AK_TRUE;

	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_BACK_RUN;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RUN;
	//}
	//if (KEY_HOLD(KEY_INPUT_S) && KEY_HOLD(KEY_INPUT_A) && KEY_HOLD(KEY_INPUT_LSHIFT))
	//{
	//	_pOwner->SpeedUp = AK_TRUE;

	//	_pOwner->MoveState = MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_RUN;

	//	if (_pOwner->BindWeapon)
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
	//	else
	//		_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RUN;
	//}
	//if (KEY_DOWN(KEY_INPUT_SPACE))
	//{
	//	// Jump
	//	_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_JUMP;

	//	_pOwner->GroundCollision = AK_FALSE;
	//	_pOwner->Jumping = AK_TRUE;
	//}
}

void Controller::KeyBoardRelease()
{
	//if (KEY_UP(KEY_INPUT_W))
	//{
	//	_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::F_WALK);
	//	_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::IDLE);
	//}
	//if (KEY_UP(KEY_INPUT_S))
	//{
	//	_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::F_WALK);
	//	_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::IDLE);
	//}
	//if (KEY_UP(KEY_INPUT_A))
	//{
	//	_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::L_WALK);
	//	_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::IDLE);
	//}
	//if (KEY_UP(KEY_INPUT_D))
	//{
	//	_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::R_WALK);
	//	_pOwner->GetMotionGraph()->ResetAnim(ANIM_STATE::IDLE);
	//}
}

void Controller::Mouse()
{
	// Fire
	if (_pOwner->BindWeapon)
	{
		if (LBTN_DOWN)
		{
			// Test
			// GetApp()->GetTestSound()->PlayOnce();
		}
		if (LBTN_HOLD)
		{
			if (_pOwner->SpeedUp)
				_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE;
			else
				_pOwner->AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE;

			_pOwner->Fire = AK_TRUE;

			// Test
			// GetApp()->GetTestSound()->Play(AK_FALSE);
		}
	}

}
