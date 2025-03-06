#pragma once

#include "Type.h"
#include "ModelImporter.h"

bool Write(ModelImporter& importer, std::string basePath, std::string fileName, bool isAnim);
bool WriteAnim(ModelImporter& importer, std::string basePath, std::string fileName);
void PNG_To_DDS(string filename);

// TBD
void Read(ModelImporter& importer, std::string basePath, std::string fileName);