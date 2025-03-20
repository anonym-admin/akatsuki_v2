#pragma once

#include <string>
#include <vector>

std::vector<std::string> SplitString(std::string path, std::string tok);
std::string GetFileExtension(const std::string& filePath);
std::string GetFilePath(const std::string& fullPath);
std::string GetFileName(const std::string& filePath);
std::string GetCurrentFolder(const std::string& filePath);
std::wstring GetFileExtension(const std::wstring& filePath);
std::wstring GetFilePath(const std::wstring& filePath);
std::wstring GetFileName(const std::wstring& filePath);
std::wstring GetCurrentFolder(const std::wstring& filePath);
std::string GetFileNmaeExcludeExt(const std::string& fileName);
std::wstring GetFileNmaeExcludeExt(const std::wstring& fileName);
std::wstring ToWString(const std::string& str);
std::string ToString(const std::wstring& str);