#pragma once
#include <stdint.h>
#include "Window.h"
#include "Draw.h"

namespace Raytracer
{
	inline constexpr int WIDTH{ 1280 };
	inline constexpr int HEIGHT{ 720 };

	struct Player
	{
		Vec2<int> pos;
		float rot;
	};

	void render(win32_offscreen_buffer* buffer);

	void keyboard(uint32_t code, bool wasDown, bool isDown);
}