#pragma once

/*
===================
USceneManager
===================
*/

class Application;
class Scene;

class SceneManager
{
public:
	~SceneManager();

	void Update();
	void FinalUpdate();
	void Render();
	void RenderShadow();

	void ChangeScene(SCENE_TYPE eType);
	Scene* GetCurrentScene() { return _pCurScene; }
	SCENE_TYPE GetCurrentSceneType() { return _eType; }
	Scene* GetScene(SCENE_TYPE eType) { return _pSceneList[(AkU32)eType]; }
	Scene* AddScene(SCENE_TYPE eType, Scene* pScene);
	Scene* BindCurrentScene(SCENE_TYPE eType);
	void UnBindCurrentScene();

private:
	void CleanUp();

private:
	Scene* _pSceneList[(AkU32)SCENE_TYPE::COUNT] = {};
	Scene* _pCurScene = nullptr;
	SCENE_TYPE _eType = {};
	AkU32 _uSceneNum = 0;
};

