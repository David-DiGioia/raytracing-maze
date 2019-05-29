#pragma once
#include <stdint.h>
#include "Window.h"

namespace Raytracer
{
	struct RGB
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};

	template<typename T>
	struct Vec2
	{
		T x;
		T y;
	};

	struct Line
	{
		Vec2<int> start;
		Vec2<int> end;
	};

	void plot(win32_offscreen_buffer* buffer, RGB color, int x, int y);

	void plot(win32_offscreen_buffer* buffer, RGB color, Vec2<int> p);

	void plotLine(win32_offscreen_buffer* buffer, RGB color, int x0, int y0, int x1, int y1);

	void plotLine(win32_offscreen_buffer* buffer, RGB color, Line line);

	void plotRect(win32_offscreen_buffer* buffer, RGB color, int x, int y, int width, int height);

	void fill(win32_offscreen_buffer* buffer, RGB color);
}