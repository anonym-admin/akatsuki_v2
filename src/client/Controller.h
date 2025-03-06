#pragma once

class Player;

class Controller
{
public:
	Controller(Player* pOwner);
	~Controller();

	AkBool Initialize(Player* pOwner);
	void Update();

private:
	void KeyBoard();
	void KeyBoardRelease();
	void Mouse();

private:
	Player* _pOwner = nullptr;
};