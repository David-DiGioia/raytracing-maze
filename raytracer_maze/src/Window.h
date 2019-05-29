#pragma once
#include <windows.h>

struct win32_offscreen_buffer
{
	// Note(casey): Pixels are always 32-bits wide, Memory Order BB GG RR XX
	BITMAPINFO Info;
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

struct win32_window_dimension
{
	int Width;
	int Height;
};

int CALLBACK
Win32WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR     CommandLine,
	int       ShowCode);