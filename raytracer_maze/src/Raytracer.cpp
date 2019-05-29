#include "Raytracer.h"

#include <random>
#include <limits>
#include <vector>
#include <algorithm>

namespace Raytracer
{
	constexpr float PI{ 3.14159265358979323846f };
	constexpr int LINE_COUNT{ 5 };
	const int MAX_RAY_LENGTH{ static_cast<int>(std::sqrtf((WIDTH / 2) * (WIDTH / 2) + (HEIGHT) * (HEIGHT))) };
	const std::uniform_int_distribution<int> randX{ 0, WIDTH / 2 };
	const std::uniform_int_distribution<int> randY{ 0, HEIGHT };
	std::mt19937 rng;

	Line mazeLines[LINE_COUNT + 4]; // add 4 for boundaries
	Player player;
	float fov{ PI / 4 };

	Vec2<int> lineIntersection(const Line& l1, const Line& l2)
	{
		int x1{ l1.start.x }, x2{ l1.end.x }, x3{ l2.start.x }, x4{ l2.end.x };
		int y1{ l1.start.y }, y2{ l1.end.y }, y3{ l2.start.y }, y4{ l2.end.y };

		int denom{ (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4) };
		float t{ ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / (float)denom };
		float u{ -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / (float)denom };

		if (denom == 0 || t < 0.0f || t > 1.0f || u < 0.0f || u > 1.0f)
			return Vec2<int>{-1, -1};

		int x{ static_cast<int>(std::round(x1 + t * (x2 - x1))) };
		int y{ static_cast<int>(std::round(y1 + t * (y2 - y1))) };
		return Vec2<int>{x, y};
	}

	int squareDistance(const Vec2<int> p1, const Vec2<int> p2)
	{
		int x{ p2.x - p1.x };
		int y{ p2.y - p1.y };
		return x * x + y * y;
	}

	float length(const Line& line)
	{
		return std::sqrtf(squareDistance(line.start, line.end));
	}

	Line raycast(const Player& player, float angle)
	{
		angle += player.rot;
		int endX{ static_cast<int>(player.pos.x + std::cosf(angle) * MAX_RAY_LENGTH) };
		int endY{ static_cast<int>(player.pos.y + std::sinf(angle) * MAX_RAY_LENGTH) };
		Vec2<int> end{ endX, endY };

		Line line{ Vec2<int>{player.pos.x, player.pos.y}, end };

		Vec2<int> nearest{ -1, -1 };
		float best{ std::numeric_limits<float>::infinity() };
		for (int i{ 0 }; i < LINE_COUNT + 4; ++i)
		{
			Vec2<int> intersection{ lineIntersection(line, mazeLines[i]) };
			if (intersection.x == -1)
				continue;

			int dist{ squareDistance(intersection, player.pos) };

			if (nearest.x == -1) {
				nearest = intersection;
				best = dist;
				continue;
			}
			if (dist < best)
			{
				nearest = intersection;
				best = dist;
			}
		}
		return Line{ player.pos, (nearest.x == -1) ? end : nearest };
	}

	std::vector<Line> raycast(const Player& player, int count, float fov)
	{
		std::vector<Line> result(count);

		const float stepSize{ fov / count };
		float currentAngle{ -fov / 2 };

		for (Line& l : result)
		{
			l = raycast(player, currentAngle);
			currentAngle += stepSize;
		}
		return result;
	}

	void drawPlayer(win32_offscreen_buffer* buffer, Player& p)
	{
		int radius{ 3 };

		plotRect(buffer, RGB{ 255, 0, 0 }, p.pos.x - radius, p.pos.y - radius, radius * 2, radius * 2);
	}

	static float remap01(float a, float b, float t)
	{
		return std::clamp((t - a) / (b - a), 0.0f, 1.0f);
	}

	// Draw left to right, starting at x
	void drawMaze(win32_offscreen_buffer* buffer, const std::vector<Line>& rays, int x, int width)
	{
		const int dx{ static_cast<int>(std::ceilf(width / (float)rays.size())) };
		const int projectionPlaneDist{ 25 };
		const float maxLenPercent{ 0.8f };

		for (int xi{ x }, i{ 0 }; i < rays.size(); xi += dx, ++i)
		{
			const float len{ length(rays[i]) };
			//const int height{ static_cast<int>(HEIGHT * remap01(MAX_RAY_LENGTH, 0, len)) };
			const int height{ (len < 1.0f) ? 0 : static_cast<int>(HEIGHT * projectionPlaneDist / len) };

			const float maxRayScaled{ MAX_RAY_LENGTH * maxLenPercent };
			const uint8_t gray{ static_cast<uint8_t>(255 * remap01(maxRayScaled * maxRayScaled, 0, len * len)) };

			RGB color{ gray, gray, gray };
			plotRect(buffer, color, xi, (HEIGHT - height) / 2, dx, height);
		}
	}

	void render(win32_offscreen_buffer* buffer)
	{
		plotRect(buffer, RGB{ 50, 50, 50 }, 0, 0, WIDTH / 2, HEIGHT);

		for (const Line& l : mazeLines)
			plotLine(buffer, RGB{ 255, 255, 255 }, l);

		std::vector<Line> rays{ raycast(player, 150, fov) };
		for (const Line& l : rays)
			plotLine(buffer, RGB{ 240, 220, 120 }, l);

		drawPlayer(buffer, player);

		plotRect(buffer, RGB{ 0, 0, 0 }, WIDTH / 2, 0, WIDTH / 2, HEIGHT);
		drawMaze(buffer, rays, WIDTH / 2, WIDTH / 2);
	}

	void randomizeLines()
	{
		for (int i{ 0 }; i < LINE_COUNT; ++i)
		{
			Vec2<int> start{ randX(rng), randY(rng) };
			Vec2<int> end{ randX(rng), randY(rng) };

			mazeLines[i] = Line{ start, end };
		}
	}

	void init()
	{
		std::random_device rd;
		rng = std::mt19937{ rd() };

		player = { Vec2<int>{100, 300}, 0.0f };

		randomizeLines();

		// Boundary lines
		const int w{ WIDTH / 2 };
		mazeLines[LINE_COUNT] = Line{ Vec2<int>{0, 0}, Vec2<int>{w, 0} };
		mazeLines[LINE_COUNT + 1] = Line{ Vec2<int>{w, 0}, Vec2<int>{w, HEIGHT} };
		mazeLines[LINE_COUNT + 2] = Line{ Vec2<int>{w, HEIGHT}, Vec2<int>{0, HEIGHT} };
		mazeLines[LINE_COUNT + 3] = Line{ Vec2<int>{0, HEIGHT}, Vec2<int>{0, 0} };
	}

	void keyboard(uint32_t code, bool wasDown, bool isDown)
	{
		const int speed{ 5 };
		const float dRot{ 0.05f };
		const float dFov{ 0.05f };

		if (code == 'W')
		{
			player.pos.x += std::roundf(std::cosf(player.rot) * speed);
			player.pos.y += std::roundf(std::sinf(player.rot) * speed);
		}
		if (code == 'A')
		{
			player.rot -= dRot;
		}
		if (code == 'S')
		{
			player.pos.x -= std::roundf(std::cosf(player.rot) * speed);
			player.pos.y -= std::roundf(std::sinf(player.rot) * speed);
		}
		if (code == 'D')
		{
			player.rot += dRot;
		}
		if (code == 'Q')
		{
			fov -= dFov;
		}
		if (code == 'E')
		{
			fov += dFov;
		}
		if (code == 'R' && isDown && !wasDown)
		{
			randomizeLines();
		}
	}
}

int CALLBACK
WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR     CommandLine,
	int       ShowCode)
{
	Raytracer::init();

	Win32WinMain(Instance, PrevInstance, CommandLine, ShowCode);
}