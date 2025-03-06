#pragma once

enum class MODEL_TYPE
{
	BLENDER_MODEL_HERO,
	BLENDER_MODEL_DANCER,
	
	// Weapon model
	BRS_74,

	MODEL_TYPE_COUNT,
};

/*
=================
ModelManager
=================
*/

class UModel;
class Application;

class ModelManager
{
public:
	ModelManager();
	~ModelManager();

	AkBool Initialize(Application* pApp);
	AkBool InitDefaultModels();
	UModel* GetModel(MODEL_TYPE eType) { return _ppModelList[(AkU32)eType]; }

	void CleanUpDefaultModels();

	// For Custum Model
	void AddModel();

private:
	void CleanUp();

private:
	Application* _pApp = nullptr;
	UModel* _ppModelList[(AkU32)MODEL_TYPE::MODEL_TYPE_COUNT] = {};
};

