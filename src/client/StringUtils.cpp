#include "pch.h"
#include "StringUtils.h"

std::vector<std::string> SplitString(std::string path, std::string tok)
{
	std::vector<std::string> result;

	size_t cutAt = 0;

	while ((cutAt = path.find_first_of(tok)) != path.npos)
	{
		if (cutAt > 0)
			result.push_back(path.substr(0, cutAt));
		path = path.substr(cutAt + 1);
	}

	return result;
}

std::string GetFileExtension(const std::string& filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == std::string::npos) {
		return "";
	}
	size_t slashPos = filePath.find_last_of("/\\");
	if (slashPos != std::string::npos && slashPos > dotPos) {
		return "";
	}
	return filePath.substr(dotPos + 1);
}

std::string GetFilePath(const std::string& filePath)
{
	size_t pos = filePath.find_last_of("/\\");
	if (pos == std::string::npos) {
		return "";
	}
	return filePath.substr(0, pos + 1);
}

std::string GetFileName(const std::string& filePath)
{
	size_t pos = filePath.find_last_of("/\\");
	if (pos == std::string::npos) {
		return "";
	}
	return filePath.substr(pos + 1);
}

std::string GetCurrentFolder(const std::string& filePath)
{
	std::string path = GetFilePath(filePath);
	path.erase(path.size() - 1);
	size_t pos = path.find_last_of("/\\");
	if (pos == std::string::npos) {
		return "";
	}
	return path.substr(pos + 1);
}

std::string GetFileNmaeExcludeExt(const std::string& fileName)
{
	size_t pos = fileName.find_last_of(".");
	if (pos == std::string::npos) {
		return "";
	}
	return fileName.substr(0, pos);
}

std::wstring GetFileExtension(const std::wstring& filePath)
{
	size_t dotPos = filePath.find_last_of(L'.');
	if (dotPos == std::wstring::npos) {
		return L"";
	}
	size_t slashPos = filePath.find_last_of(L"/\\");
	if (slashPos != std::wstring::npos && slashPos > dotPos) {
		return L"";
	}
	return filePath.substr(dotPos + 1);
}

std::wstring GetFilePath(const std::wstring& fullPath)
{
	size_t pos = fullPath.find_last_of(L"/\\");
	if (pos == std::wstring::npos) {
		return L"";
	}
	return fullPath.substr(0, pos + 1);
}

std::wstring GetFileName(const std::wstring& filePath)
{
	size_t pos = filePath.find_last_of(L"/\\");
	if (pos == std::wstring::npos) {
		return L"";
	}
	return filePath.substr(pos + 1);
}

std::wstring GetCurrentFolder(const std::wstring& filePath)
{
	std::wstring path = GetFilePath(filePath);
	path.erase(path.size() - 1);
	size_t pos = path.find_last_of(L"/\\");
	if (pos == std::wstring::npos) {
		return L"";
	}
	return path.substr(pos + 1);
}

std::wstring GetFileNmaeExcludeExt(const std::wstring& fileName)
{
	size_t pos = fileName.find_last_of(L".");
	if (pos == std::wstring::npos) {
		return L"";
	}
	return fileName.substr(0, pos);
}

std::wstring ToWString(const std::string& str)
{
	std::wstring ret;
	return ret.assign(str.begin(), str.end());
}

std::string ToString(const std::wstring& str)
{
	std::string ret;
	return ret.assign(str.begin(), str.end());
}