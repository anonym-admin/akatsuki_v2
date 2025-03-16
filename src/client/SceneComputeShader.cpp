#include "pch.h"
#include "SceneComputeShader.h"

SceneComputeShader::~SceneComputeShader()
{
    EndScene();
}

AkBool SceneComputeShader::BeginScene()
{
    return AK_TRUE;
}

AkBool SceneComputeShader::EndScene()
{
    return AK_TRUE;
}

void SceneComputeShader::Update()
{
    printf("Scene Compute Shader. \n");
}

void SceneComputeShader::FinalUpdate()
{
}

void SceneComputeShader::Render()
{
}

void SceneComputeShader::RenderShadow()
{
}
