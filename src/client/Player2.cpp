#include "pch.h"
#include "Player.h"
#include "Controller.h"

Player::~Player()
{
	CleanUp();
}

Controller* Player::CreateController()
{
	Controller* pController = new Controller(this);
	return pController;
}

void Player::CleanUp()
{
	if (_pController)
	{
		delete _pController;
		_pController = nullptr;
	}
}
