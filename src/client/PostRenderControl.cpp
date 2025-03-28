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

	ImGui::End();
}
