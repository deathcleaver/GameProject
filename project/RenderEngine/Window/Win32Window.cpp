#ifdef _WIN32

// local includes
#include "Window.hpp"

// global includes
#include <GL\glew.h>
#include <GL\wglew.h>
#include <GL\GL.h>

#include <iostream>
#include <cstdint>

#include <tchar.h>
#include "WindowSystem.hpp"

#include <strsafe.h>

#define RI_MOUSE_HWHEEL 0x0800

// globals
const wchar_t* windowClassName = _T("WindowClass");
bool windowClassInitialized = false;
bool openglInitialized = false;

HHOOK g_hKeyboardHook;
BOOL g_bWindowActive;


// global pre-defines
HWND setupWindow();

// global function definitions

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if ( nCode < 0 || nCode != HC_ACTION )  // do not process message 
		return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);

	bool bEatKeystroke = false;
	KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
	switch ( wParam ) {
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			bEatKeystroke = (g_bWindowActive && ((p->vkCode == VK_LWIN) || (p->vkCode == VK_RWIN)));
			break;
		}
	}

	printf("Callback\n");

	if ( bEatKeystroke )
		return 1;
	else
		return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}


LRESULT WINAPI WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	BaseWindow* wnd = reinterpret_cast<BaseWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch ( msg ) {
		case WM_SIZE:
		{
			if ( wnd && wnd->resizeCallback ) {
				int width = LOWORD(lParam);
				int height = HIWORD(lParam);
				wnd->resizeCallback(wnd, width, height);
				printf("callback stuff");
			}
			printf("Resize\n");
		}
		break;
		case WM_MOVE:
		{
			int xPos = (int)(short)LOWORD(lParam);   // horizontal position 
			int yPos = (int)(short)HIWORD(lParam);   // vertical position
		}
			break;
		case WM_CLOSE:
			break;

		case WM_ACTIVATEAPP:
			// g_bWindowActive is used to control if the Windows key is filtered by the keyboard hook or not.
			if ( wParam == TRUE )
				g_bWindowActive = true;
			else
				g_bWindowActive = false;
			break;

		case WM_CHAR:
		case WM_SYSCHAR:
		case WM_UNICHAR:
		{
			const bool plain = (msg != WM_SYSCHAR);

			if ( msg == WM_UNICHAR && wParam == UNICODE_NOCHAR ) {
				// WM_UNICHAR is not sent by Windows, but is sent by some
				// third-party input method engine
				// Returning TRUE here announces support for this message
				return TRUE;
			}
			if ( wnd->characterCallback )
				wnd->characterCallback(wnd, (unsigned int)wParam);
			return 0;
		}
		break;
		case WM_INPUT:
		{
			if ( wnd ) {
				UINT dwSize;

				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize,
								sizeof(RAWINPUTHEADER));
				LPBYTE lpb = new BYTE[dwSize];
				if ( lpb == NULL ) {
					return 0;
				}

				if ( GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize,
									 sizeof(RAWINPUTHEADER)) != dwSize ) {
					//OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));
					std::cerr << "GetRawInputData does not return correct size !\n";
				}

				RAWINPUT* raw = (RAWINPUT*)lpb;

				if ( raw->header.dwType == RIM_TYPEKEYBOARD ) {
					//TCHAR szTempOutput[100];
					//HRESULT hResult = StringCchPrintf(szTempOutput, 100, TEXT(" Kbd: make=%04x Flags:%04x Reserved:%04x ExtraInformation:%08x, msg=%04x VK=%04x \n"),
					//								  raw->data.keyboard.MakeCode,
					//								  raw->data.keyboard.Flags,
					//								  raw->data.keyboard.Reserved,
					//								  raw->data.keyboard.ExtraInformation,
					//								  raw->data.keyboard.Message,
					//								  raw->data.keyboard.VKey);
					//if ( FAILED(hResult) ) {
					//	// TODO: write error handler
					//}
					//OutputDebugString(szTempOutput);
					//std::wcout << szTempOutput;

					//keyboardMsg->keyEvent = true;
					//if ( raw->data.keyboard.Flags == RI_KEY_BREAK ) {
					//	keyboardMsg->keyDown = false;
					//} else if ( raw->data.keyboard.Flags == RI_KEY_MAKE ) {
					//	keyboardMsg->keyDown = true;
					//}
					//keyboardMsg->scanCode = raw->data.keyboard.MakeCode;
					//std::cout << "Char " << MapVirtualKeyEx(raw->data.keyboard.VKey, MAPVK_VK_TO_CHAR, GetKeyboardLayout(0)) << std::endl;
					//std::cout << "VKey " << raw->data.keyboard.VKey << std::endl;
					//std::cout << "Make code " << raw->data.keyboard.MakeCode << std::endl;
					//std::cout << "Scan code " << MapVirtualKeyEx(raw->data.keyboard.VKey, MAPVK_VK_TO_VSC, GetKeyboardLayout(0)) << std::endl;

					if ( raw->data.keyboard.Flags == RI_KEY_MAKE ) {
						if ( raw->data.keyboard.VKey == VK_SHIFT ) {
							wnd->modkeys |= MODKEY_SHIFT;
						} else if ( raw->data.keyboard.VKey == VK_CONTROL ) {
							wnd->modkeys |= MODKEY_CTRL;
						} else if ( raw->data.keyboard.VKey == VK_MENU ) {
							wnd->modkeys |= MODKEY_ALT;
						} else if ( raw->data.keyboard.VKey == VK_LWIN || raw->data.keyboard.VKey == VK_RWIN ) {
							wnd->modkeys |= MODKEY_SUPER;
						}
					} else {
						if ( raw->data.keyboard.VKey == VK_SHIFT ) {
							wnd->modkeys &= ~MODKEY_SHIFT;
						} else if ( raw->data.keyboard.VKey == VK_CONTROL ) {
							wnd->modkeys &= ~MODKEY_CTRL;
						} else if ( raw->data.keyboard.VKey == VK_MENU ) {
							wnd->modkeys &= ~MODKEY_ALT;
						} else if ( raw->data.keyboard.VKey == VK_LWIN || raw->data.keyboard.VKey == VK_RWIN ) {
							wnd->modkeys &= ~MODKEY_SUPER;
						}
					}

					if ( wnd->keyCallback ) {

						int action = raw->data.keyboard.Flags == RI_KEY_BREAK ? ACTION_BUTTON_UP : ACTION_BUTTON_DOWN;

						int mods = wnd->modkeys;

						wnd->keyCallback(wnd, raw->data.keyboard.MakeCode, action, mods);
						printf("keyCallback\n");
					}

					printf("key event\n");

				} else if ( raw->header.dwType == RIM_TYPEMOUSE ) {
					//TCHAR szTempOutput[200];
					//HRESULT hResult = StringCchPrintf(szTempOutput, 200, TEXT("Mouse: usFlags=%04x ulButtons=%04x usButtonFlags=%04x usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY=%04x ulExtraInformation=%04x\r\n"),
					//								  raw->data.mouse.usFlags,
					//								  raw->data.mouse.ulButtons,
					//								  raw->data.mouse.usButtonFlags,
					//								  raw->data.mouse.usButtonData,
					//								  raw->data.mouse.ulRawButtons,
					//								  raw->data.mouse.lLastX,
					//								  raw->data.mouse.lLastY,
					//								  raw->data.mouse.ulExtraInformation);

					//mouseMsg->mouseDX = (float)raw->data.mouse.lLastX / 2;
					//mouseMsg->mouseDY = (float)raw->data.mouse.lLastY / 2;
					//
					//mouseMsg->mouseBtnDown = false;
					//mouseMsg->mouseBtn = -1;

					//unsigned short btn = raw->data.mouse.usButtonFlags;

					//if ( FAILED(hResult) ) {
					//	// TODO: write error handler
					//}
					//OutputDebugString(szTempOutput);
					//std::wcout << szTempOutput;



					if ( wnd->mouseButtonCallback ) {

						int button = -1;
						int action = 0;
						int mods = wnd->modkeys;
						switch ( raw->data.mouse.usButtonFlags ) {
							case(RI_MOUSE_BUTTON_1_DOWN): {
								button = 0;
								action = ACTION_BUTTON_DOWN;
								break;
							}
							case(RI_MOUSE_BUTTON_1_UP): {
								button = 0;
								action = ACTION_BUTTON_UP;
								break;
							}
							case(RI_MOUSE_BUTTON_2_DOWN): {
								button = 1;
								action = ACTION_BUTTON_DOWN;
								break;
							}
							case(RI_MOUSE_BUTTON_2_UP): {
								button = 1;
								action = ACTION_BUTTON_UP;
								break;
							}
							case(RI_MOUSE_BUTTON_3_DOWN): {
								button = 2;
								action = ACTION_BUTTON_DOWN;
								break;
							}
							case(RI_MOUSE_BUTTON_3_UP): {
								button = 2;
								action = ACTION_BUTTON_UP;
								break;
							}
							case(RI_MOUSE_BUTTON_4_DOWN): {
								button = 3;
								action = ACTION_BUTTON_DOWN;
								break;
							}
							case(RI_MOUSE_BUTTON_4_UP): {
								button = 3;
								action = ACTION_BUTTON_UP;
								break;
							}
							case(RI_MOUSE_BUTTON_5_DOWN): {
								button = 4;
								action = ACTION_BUTTON_DOWN;
								break;
							}
							case(RI_MOUSE_BUTTON_5_UP): {
								button = 4;
								action = ACTION_BUTTON_UP;
								break;
							}
							default:
								break;
						}
						wnd->mouseButtonCallback(wnd, button, action, mods);
						//printf("mouse button callback\n");

						//std::cout << button;
					}

					if ( wnd->mouseMoveCallback ) {
						POINT pt;
						GetCursorPos(&pt);
						ScreenToClient(hWnd, &pt);
						//wnd->mouseMoveCallback(wnd, pt.x, pt.y);
						//printf("move callback\n");
					}

					if ( wnd->scrollCallback ) {

						short scrollX = 0;
						short scrollY = 0;

						if ( RI_MOUSE_WHEEL == raw->data.mouse.usButtonFlags ) {
							scrollX = (short)raw->data.mouse.usButtonData;
						}
						if ( RI_MOUSE_HWHEEL == raw->data.mouse.usButtonFlags ) {
							scrollY = (short)raw->data.mouse.usButtonData;
						}

						wnd->scrollCallback(wnd, (int)scrollX, (int)scrollY);
					}

					if ( wnd->lockCursor ) {
						RECT rec;
						GetWindowRect(hWnd, &rec);

						int posX = (rec.right - rec.left) / 2;
						posX += rec.left;

						int posY = (rec.bottom - rec.top) / 2;
						posY += rec.top;

						SetCursorPos(posX, posY);
					}

					//printf("\nmouse event\n");

				}
				delete[] lpb;
				return 1;
			}
		}
		break;
		default:
			break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void setPixelFormatOGL(HDC windowDC) {
	int selectedPixelFormat = 0;
	uint32_t foundPixelFormats = 0;

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd  
		1,                     // version number  
		PFD_DRAW_TO_WINDOW |   // support window  
		PFD_SUPPORT_OPENGL |   // support OpenGL  
		PFD_DOUBLEBUFFER,      // double buffered  
		PFD_TYPE_RGBA,         // RGBA type  
		24,                    // 24-bit color depth  
		0, 0, 0, 0, 0, 0,      // color bits ignored  
		8,                     // no alpha buffer  
		0,                     // shift bit ignored  
		0,                     // no accumulation buffer  
		0, 0, 0, 0,            // accum bits ignored  
		24,                    // 32-bit z-buffer  
		8,                     // no stencil buffer  
		0,                     // no auxiliary buffer  
		PFD_MAIN_PLANE,        // main layer  
		0,                     // reserved  
		0, 0, 0                // layer masks ignored  
	};

	if ( !openglInitialized ) {
		// todo create tempContext and init ogl extensions
		HWND window = setupWindow();
		HDC dc = GetDC(window);
		selectedPixelFormat = ChoosePixelFormat(dc, &pfd);
		SetPixelFormat(dc, selectedPixelFormat, &pfd);
		HGLRC rc = wglCreateContext(dc);
		wglMakeCurrent(dc, rc);

		//glewInit();
		wglewInit();

		wglMakeCurrent(0, 0);
		wglDeleteContext(rc);
		ReleaseDC(window, dc);
		DestroyWindow(window);

		openglInitialized = true;
	}
	// todo fix attribute stuff
	int intAttributes[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_COLOR_BITS_ARB, 24,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		//WGL_SWAP_METHOD_ARB, WGL_SWAP_UNDEFINED_ARB,
		0

	};
	float floatAttributes[] = { 0 };

	selectedPixelFormat = 0;

	if ( wglChoosePixelFormatARB ) {
		wglChoosePixelFormatARB(windowDC, intAttributes, floatAttributes, 1, &selectedPixelFormat, &foundPixelFormats);
	} else {
		selectedPixelFormat = ChoosePixelFormat(windowDC, &pfd);
	}


	SetPixelFormat(windowDC, selectedPixelFormat, &pfd);


}

int registerWindowClass() {
	if ( windowClassInitialized )
		return windowClassInitialized;
	WNDCLASS wc{};

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = windowClassName;

	return RegisterClass(&wc);
}

HWND setupWindow() {
	HWND windowHandle = NULL;
	if ( windowClassInitialized ) {
		windowHandle = CreateWindowEx(0,
									  windowClassName,
									  _T("Window"),
									  WS_OVERLAPPEDWINDOW,
									  CW_USEDEFAULT, CW_USEDEFAULT,
									  CW_USEDEFAULT, CW_USEDEFAULT,
									  NULL,
									  NULL,
									  GetModuleHandle(NULL),
									  NULL);
	}

	return windowHandle;
}

HGLRC createOpenGLContext(HDC windowDC) {

	setPixelFormatOGL(windowDC);

	HGLRC renderingContext = 0;
	if ( wglCreateContextAttribsARB ) {
		int contextAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 5,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
			0
		};

		renderingContext = wglCreateContextAttribsARB(windowDC, NULL, contextAttributes);
	} else {
		renderingContext = wglCreateContext(windowDC);
	}

	return renderingContext;
}

// window classes 

// BaseWindow -> win32 implementation

void BaseWindow::setWindowPos(int x, int y) {
	SetWindowPos(windowHandle, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void BaseWindow::setWindowSize(int x, int y) {
	SetWindowPos(windowHandle, NULL, 0, 0, x, y, SWP_NOZORDER | SWP_NOMOVE);
}

void BaseWindow::showWindow(bool visible) {
	this->visible = visible;
	ShowWindow(windowHandle, visible ? SW_SHOW : SW_HIDE);
}

void BaseWindow::setWindowBorderless(bool borderless) {

	DWORD style = (borderless ? WS_POPUP : WS_OVERLAPPEDWINDOW) | (visible ? WS_VISIBLE : 0);
	SetWindowLongPtr(windowHandle, GWL_STYLE, style);

}

void BaseWindow::setWindowedTrueFullscreen(bool trueFullscreen) {
	// todo figure out how to make exclusive fullscreen
}

// GLWindow -> win32 implementation

void GLWindow::init() {
	windowHandle = setupWindow();

	if ( windowHandle ) {
		SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)this);

		deviceContext = GetDC(windowHandle);
		openglRenderContext = createOpenGLContext(deviceContext);

		if ( !openglRenderContext )
			printf("something went wrong");
		if ( wglMakeCurrent(deviceContext, openglRenderContext) == FALSE ) {
			printf("making current failed\n");
		}

		if ( wglSwapIntervalEXT )
			wglSwapIntervalEXT(1);
	}
}

void GLWindow::deinit() {
	wglMakeCurrent(0, 0);
	wglDeleteContext(openglRenderContext);

	ReleaseDC(windowHandle, deviceContext);
	DestroyWindow(windowHandle);
}

void GLWindow::setVsync(bool vSync) {
	wglSwapIntervalEXT(vSync ? 1 : 0);
}

void GLWindow::makeCurrent() {
	if ( wglMakeCurrent(deviceContext, openglRenderContext) == FALSE ) {
		printf("making current failed\n");
	}
}

void GLWindow::swapBuffers() {
	SwapBuffers(deviceContext);
}

void initWindowSystem() {

	windowClassInitialized = registerWindowClass() != 0 ? 1 : 0;

	RAWINPUTDEVICE rid[2];

	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x02;
	rid[0].dwFlags = 0;   // adds HID mouse and also ignores legacy mouse messages
	rid[0].hwndTarget = 0;

	rid[1].usUsagePage = 0x01;
	rid[1].usUsage = 0x06;
	rid[1].dwFlags = 0;   // adds HID keyboard and also ignores legacy keyboard messages
	rid[1].hwndTarget = 0;

	//g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

	if ( RegisterRawInputDevices(rid, 2, sizeof(rid[0])) == FALSE ) {
		//registration failed. Call GetLastError for the cause of the error
		std::cerr << "register RID failed\n";
		std::cerr << GetLastError() << std::endl;
	}
	
}

void deinitWindowSystem() {
	openglInitialized = false;
	windowClassInitialized = false;
	UnregisterClass(windowClassName, GetModuleHandle(NULL));

	//UnhookWindowsHookEx(g_hKeyboardHook);
}

#endif