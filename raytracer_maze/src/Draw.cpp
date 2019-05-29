#include "Draw.h"

#include <cmath>
#include <algorithm>
#include "Window.h"
#include "Raytracer.h"

namespace Raytracer
{
	static uint32_t RGBtoUInt32(RGB rgb)
	{
		// xx RR GG BB
		return (rgb.r << 16) | (rgb.g << 8) | rgb.b;
	}

	void plot(win32_offscreen_buffer* buffer, RGB color, int x, int y)
	{
		// TEMP FIX, DON'T LET IT GET THIS FAR-----------------------------------------
		if (x < 0 || x >= Raytracer::WIDTH || y < 0 || y >= Raytracer::HEIGHT)
			return;
		// ----------------------------------------------------------------------------

		uint32_t* data = (uint32_t*)buffer->Memory;
		data[y * buffer->Width + x] = RGBtoUInt32(color);
	}

	void plot(win32_offscreen_buffer* buffer, RGB color, Vec2<int> p)
	{
		plot(buffer, color, p.x, p.y);
	}

	static void lineLow(win32_offscreen_buffer* buffer, RGB color, int x0, int y0, int x1, int y1)
	{
		uint32_t* data = (uint32_t*)buffer->Memory;

		int dx{ x1 - x0 };
		int dy{ y1 - y0 };
		int yi{ 1 };

		if (dy < 0)
		{
			yi = -1;
			dy = -dy;
		}

		int D{ 2 * dy - dx };
		int y{ y0 };

		for (int x{ x0 }; x <= x1; ++x)
		{
			plot(buffer, color, x, y);
			if (D > 0)
			{
				y += yi;
				D -= 2 * dx;
			}
			D += 2 * dy;
		}
	}

	static void lineHigh(win32_offscreen_buffer* buffer, RGB color, int x0, int y0, int x1, int y1)
	{
		uint32_t* data = (uint32_t*)buffer->Memory;

		int dx{ x1 - x0 };
		int dy{ y1 - y0 };
		int xi{ 1 };

		if (dx < 0)
		{
			xi = -1;
			dx = -dx;
		}

		int D{ 2 * dx - dy };
		int x{ x0 };

		for (int y{ y0 }; y <= y1; ++y)
		{
			plot(buffer, color, x, y);
			if (D > 0)
			{
				x += xi;
				D -= 2 * dy;
			}
			D += 2 * dx;
		}
	}

	void plotLine(win32_offscreen_buffer* buffer, RGB color, int x0, int y0, int x1, int y1)
	{
		if (std::abs(y1 - y0) < std::abs(x1 - x0))
		{
			if (x0 > x1)
				lineLow(buffer, color, x1, y1, x0, y0);
			else
				lineLow(buffer, color, x0, y0, x1, y1);
		}
		else
		{
			if (y0 > y1)
				lineHigh(buffer, color, x1, y1, x0, y0);
			else
				lineHigh(buffer, color, x0, y0, x1, y1);
		}
	}

	void plotLine(win32_offscreen_buffer* buffer, RGB color, Line line)
	{
		plotLine(buffer, color, line.start.x, line.start.y, line.end.x, line.end.y);
	}

	// x and y specify top left of rectangle
	void plotRect(win32_offscreen_buffer* buffer, RGB color, int x, int y, int width, int height)
	{
#undef min
		width = std::min(width, buffer->Width - x);
		height = std::min(height, buffer->Height - y);

		for (int yi{ y }; yi < y + height; ++yi)
			for (int xi{ x }; xi < x + width; ++xi)
				plot(buffer, color, xi, yi);
	}

	void fill(win32_offscreen_buffer* buffer, RGB color)
	{
		for (int y{ 0 }; y < buffer->Height; ++y)
			for (int x{ 0 }; x < buffer->Width; ++x)
				plot(buffer, color, x, y);
	}
}