#pragma once

int Clamp(const int iValue, const int iMin, const int iMax);
float Clamp(const float fValue, const float fMin, const float fMax);
double Clamp(const double dValue, const double dMin, const double dMax);
bool ReadBitmapFile(const wchar_t* wcFilename, unsigned __int8** pDestImage, unsigned __int32* pWidth, unsigned __int32* pHeight);
float GenterateRandomFloat(float fMin, float fMax);