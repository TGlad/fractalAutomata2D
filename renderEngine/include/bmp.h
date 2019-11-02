#pragma once
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <strstream>

using namespace std;

BYTE* ConvertRGBToBMPBuffer(BYTE* Buffer, int width, int height, long* newsize);
bool SaveBMP(BYTE* Buffer, int width, int height, long paddedsize, LPCTSTR bmpfile);
BYTE* LoadBMP(int* width, int* height, long* size, LPCTSTR bmpfile);

