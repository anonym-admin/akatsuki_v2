#include "pch.h"
#include "PostRenderControl.h"

void PostRenderControl::RendeGUI()
{
	ImGui::Begin("Post Process Control");

	// Select Tone mapping.
	AkI32 iSelectedItem = -1;
	const char* pItems[] =
	{
		"Linear",
		"Uncharted",
		"Filmic"
	};
	if (ImGui::Combo("Tome mapping", &iSelectedItem, pItems, IM_ARRAYSIZE(pItems)))
	{
		GRenderer->SetToneMappingType(iSelectedItem);
	}
	ImGui::SliderInt("Bloom Level", &_iBloomLevel, 0, 4);
	ImGui::SliderFloat("Bloom Strength", &_fBloomStrength, 0.0f, 1.0f, "%.5f");

	GRenderer->SetBloomLevels(_iBloomLevel);
	GRenderer->SetBloomStrength(_fBloomStrength);

	ImGui::Checkbox("Effect", &_bUseEffect);
	ImGui::SameLine();
	ImGui::Checkbox("Depth Map", &_bUseDepthMap);

	if (_bUseEffect)
	{
		GRenderer->SetPostEffectMode(1);
	}
	if (_bUseDepthMap)
	{
		GRenderer->SetPostEffectMode(2);
	}

	ImGui::SliderFloat("Depth Scale", &_fDepthScale, 0.0f, 1.0f);
	ImGui::SliderFloat("Fog Strength", &_fFogStrength, 0.0f, 1.0f);

	GRenderer->SetFogStrength(_fFogStrength);
	GRenderer->SetDepthScale(_fDepthScale);

	ImGui::End();
}
