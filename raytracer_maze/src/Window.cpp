#include "Window.h"

#include <stdint.h>
#include <Xinput.h>
#include "Raytracer.h"

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// This is only temporarily global. Static variables are initialized to zero
global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;

// We don't want to directly link to the XInput library because its required spec is limiting,
// so we are making pointers to the two functions we need from the library and defining them
// as the original names of the functions. We make them point to a stub by default so the program
// doesn't crash even if we don't define the functions.
//NOTE(casey): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return 0;
}
global_variable x_input_get_state *XInputGetState_{ XInputGetStateStub };
#define XInputGetState XInputGetState_

//NOTE(casey): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputGetStateStub)
{
	return 0;
}
global_variable x_input_set_state *XInputSetState_{ XInputGetStateStub };
#define XInputSetState XInputSetState_

internal void
Win32LoadXInput(void)
{
	HMODULE XInputLibrary{ LoadLibraryA("xinput1_3.dll") };
	if (XInputLibrary)
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputGetState");
	}
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return Result;
}

internal void
RenderBuffer(win32_offscreen_buffer *Buffer, int XOffset, int YOffset)
{
	Raytracer::render(Buffer);
}
internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
	// TODO(casey): Bulletproof this.
	// Maybe don't free first, free after, then free first if that fails.

	if (Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;
	int BytesPerPixel{ 4 };

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // Negative so that pixels will stored topdown
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	Buffer->Pitch = Width * BytesPerPixel;

	// TODO(casey): Probably clear this to black
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer,
	HDC DeviceContext, int WindowWidth, int WindowHeight)
{
	// TODO(casey): Aspect ratio correction
	StretchDIBits(DeviceContext,
		0, 0, Buffer->Width, Buffer->Height,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory,
		&Buffer->Info,
		DIB_RGB_COLORS, SRCCOPY);
}


// This is passed as function pointer to register class function
// (indirectly, as a member of WNDCLASS WindowClass)
LRESULT CALLBACK
Win32MainWindowCallback(
	HWND   Window,
	UINT   Message,
	WPARAM WParam,
	LPARAM LParam)
{
	LRESULT Result{ 0 };

	switch (Message)
	{
	case WM_CLOSE:
	{
		// TODO(casey) Handle this with a message to the user?
		GlobalRunning = false;
		OutputDebugStringA("WM_CLOSE\n");
	} break;

	case WM_ACTIVATEAPP:
	{
		OutputDebugStringA("WM_ACTIVATEAPP\n");
	} break;

	case WM_DESTROY:
	{
		// TODO(casey): Handle this as an error - recreate window?
		GlobalRunning = false;
		OutputDebugStringA("WM_DESTROY\n");
	} break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		uint32 VKCode{ WParam };
		bool WasDown{ (LParam & (1 << 30)) != 0 };
		bool IsDown{ (LParam & (1 << 31)) == 0 };

		Raytracer::keyboard(VKCode, WasDown, IsDown);

		if (WasDown != IsDown)
		{
			if (VKCode == 'W')
			{
			}
			else if (VKCode == 'A')
			{
			}
			else if (VKCode == 'S')
			{
			}
			else if (VKCode == 'Q')
			{
			}
			else if (VKCode == 'E')
			{
			}
			else if (VKCode == VK_UP)
			{
			}
			else if (VKCode == VK_LEFT)
			{
			}
			else if (VKCode == VK_DOWN)
			{
			}
			else if (VKCode == VK_RIGHT)
			{
			}
			else if (VKCode == VK_ESCAPE)
			{
				OutputDebugStringA("ESCAPE: ");
				if (IsDown)
				{
					OutputDebugStringA("IsDown ");
				}
				if (WasDown)
				{
					OutputDebugStringA("WasDown");
				}
				OutputDebugStringA("\n");
			}
			else if (VKCode == VK_SPACE)
			{
			}
		}
	} break;

	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext{ BeginPaint(Window, &Paint) };
		win32_window_dimension Dimension{ Win32GetWindowDimension(Window) };
		Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext,
			Dimension.Width, Dimension.Height);
		EndPaint(Window, &Paint);
	} break;

	default:
	{
		//OutputDebugStringA("default\n");
		Result = DefWindowProc(Window, Message, WParam, LParam);
	} break;
	}
	return Result;
}


// Entry point for program
int CALLBACK
Win32WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR     CommandLine,
	int       ShowCode)
{
	Win32LoadXInput();

	// Zero-initialize. This class exists as a container for arguments, in our case for RegisterClass
	WNDCLASSA WindowClass{};

	Win32ResizeDIBSection(&GlobalBackBuffer, Raytracer::WIDTH, Raytracer::HEIGHT);

	// This redraws the whole window when it's resized
	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	// Populate WindowClass with arguments
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	//WindowClass.hIcon;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	// RegisterClass defines the class that we call WindowClass
	if (RegisterClass(&WindowClass))
	{
		// Create WindowHandle which is a handle to the window created with CreateWindowExA function
		HWND Window{
		CreateWindowExA(
			0,
			WindowClass.lpszClassName,
			"Raytrace Maze",
			//WS_OVERLAPPEDWINDOW |
			WS_VISIBLE |
			WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, // these prevent resizing
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			GlobalBackBuffer.Width,
			GlobalBackBuffer.Height,
			0,
			0,
			Instance,
			0) };

		// If creation is successful, repeatedly listen for messages, and once they're dispacted
		// they will be handled by the MainWindowCallBack function we wrote
		if (Window)
		{
			// Note(casey): Since we specified CS_OWNDC, we can just
			// get one device context and use it forever because we
			// are not sharing it with anyone
			HDC DeviceContext{ GetDC(Window) };

			int XOffset{ 0 };
			int YOffset{ 0 };

			GlobalRunning = true;
			while (GlobalRunning)
			{
				// As long as there are messages in the queue, we will process them
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if (Message.message == WM_QUIT)
					{
						GlobalRunning = false;
					}

					// Takes message off message queue, processes it, and makes it ready to send it out
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}

				// TODO(casey): Should we poll this more frequently
				for (DWORD ControllerIndex{ 0 }; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex)
				{
					XINPUT_STATE ControllerState;
					if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
					{
						// NOTE(casey): This controller is plugged in
						// NOTE(casey): See if ControllerState.dwPacketNumber increments too rapidly
						XINPUT_GAMEPAD *Pad{ &ControllerState.Gamepad };

						bool Up{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP) };
						bool Down{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) };
						bool Left{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) };
						bool Right{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) };
						bool Start{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_START) };
						bool Back{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_BACK) };
						bool LeftShoulder{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) };
						bool RightShoulder{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) };
						bool AButton{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_A) };
						bool BButton{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_B) };
						bool XButton{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_X) };
						bool YButton{ (bool)(Pad->wButtons & XINPUT_GAMEPAD_Y) };

						int16 StickX{ Pad->sThumbLX };
						int16 Sticky{ Pad->sThumbLY };

						if (AButton)
						{
							YOffset += 2;
						}
					}
					else
					{
						// NOTE(casey): This controller is not available
					}
				}

				RenderBuffer(&GlobalBackBuffer, XOffset, YOffset);

				win32_window_dimension Dimension{ Win32GetWindowDimension(Window) };
				Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext,
					Dimension.Width, Dimension.Height);
				ReleaseDC(Window, DeviceContext);

				++XOffset;
			}
		}
		else
		{
			// TODO(casey): Logging
		}
	}
	else
	{
		// TODO(casey): Logging
	}

	return 0;
}