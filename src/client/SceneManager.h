#pragma once

/*
===================
USceneManager
===================
*/

class UApplication;
class UScene;

class USceneManager
{
public:
	USceneManager();
	~USceneManager();

	AkBool Initialize(UApplication* pApp);
	void Update(const AkF32 fDeltaTime);
	void FinalUpdate(const AkF32 fDeltaTime);
	void RenderShadowPass();
	void Render();
	
	void ChangeScene(GAME_SCENE_TYPE eSceneType);
	UScene* GetCurrentScene() { return _pCurScene; }
	UScene* GetScene(GAME_SCENE_TYPE eSceneType) { return _pSceneList[(AkU32)eSceneType]; }

private:
	void CleanUp();

	UScene* CreateScene(GAME_SCENE_TYPE eSceneType);

private:
	UApplication* _pApp = nullptr;
	UScene* _pSceneList[(AkU32)GAME_SCENE_TYPE::SCENE_TYPE_COUNT] = {};
	UScene* _pCurScene = nullptr;
	AkU32 _uSceneNum = 0;
};

