#ifndef PAL_LIB_WIN32_H
#define PAL_LIB_WIN32_H

/********************************************/
/*  Graphics, Audio and Input library for   */
/*  Interactive Applications                */
/********************************************/

/*******************************************/
/*       Copyright (c) 2021 CTech          */
/*         All rights reserved.            */
/*******************************************/

#include "paldef.h"
#include "pal_win32_gl.h"

/********************************************/
/* @includes                                */
/* PalLib Win32 Includes                    */
/********************************************/

#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <mmsystem.h>
#include <intrin.h>
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <dsound.h>

/* win32 key codes */
typedef enum enum_keyCode {
  PAL_LBUTTON = 0x01,
  PAL_RBUTTON = 0x02,
  PAL_CANCEL = 0x03,
  PAL_MKBUTTON = 0x04,
  PAL_XBUTTON1 = 0x05,
  PAL_XBUTTON2 = 0x06,
  PAL_BACKSPACE = 0x08,
  PAL_TAB = 0x09,
  PAL_CLEAR = 0x0C,
  PAL_RETURN = 0x0D,
  PAL_SHIFT = 0x10,
  PAL_CONTROL = 0x11,
  PAL_ALT = 0x12,
  PAL_PAUSE = 0x13,
  PAL_CAPS = 0x14,
  PAL_ESC = 0x1B,
  PAL_SPACE = 0x20,
  PAL_PGUP = 0x21,
  PAL_PGDOWN = 0x22,
  PAL_END = 0x23,
  PAL_HOME = 0x24,
  PAL_LEFT = 0x25,
  PAL_UP = 0x26,
  PAL_RIGHT = 0x27,
  PAL_DOWN = 0x28,
  PAL_SELECT = 0x29,
  PAL_PRINT = 0x2A,
  PAL_EXEC = 0x2B,
  PAL_PRINTSCR = 0x2C,
  PAL_INSERT = 0x2D,
  PAL_DELETE = 0x2E,
  PAL_HELP = 0x2F,
  PAL_0 = 0x30,
  PAL_1 = 0x31,
  PAL_2 = 0x32,
  PAL_3 = 0x33,
  PAL_4 = 0x34,
  PAL_5 = 0x35,
  PAL_6 = 0x36,
  PAL_7 = 0x37,
  PAL_8 = 0x38,
  PAL_9 = 0x39,
  PAL_A = 0x41,
  PAL_B = 0x42,
  PAL_C = 0x43,
  PAL_D = 0x44,
  PAL_E = 0x45,
  PAL_F = 0x46,
  PAL_G = 0x47,
  PAL_H = 0x48,
  PAL_I = 0x49,
  PAL_J = 0x4A,
  PAL_K = 0x4B,
  PAL_L = 0x4C,
  PAL_M = 0x4D,
  PAL_N = 0x4E,
  PAL_O = 0x4F,
  PAL_P = 0x50,
  PAL_Q = 0x51,
  PAL_R = 0x52,
  PAL_S = 0x53,
  PAL_T = 0x54,
  PAL_U = 0x55,
  PAL_V = 0x56,
  PAL_W = 0x57,
  PAL_X = 0x58,
  PAL_Y = 0x59,
  PAL_Z = 0x5A,
  PAL_LSUPER = 0x5B,
  PAL_RSUPER = 0x5C,
  PAL_APPS = 0x5D,
  PAL_SLEEP = 0x5F,
  PAL_NUMPAD0 = 0x60,
  PAL_NUMPAD1 = 0x61,
  PAL_NUMPAD2 = 0x62,
  PAL_NUMPAD3 = 0x63,
  PAL_NUMPAD4 = 0x64,
  PAL_NUMPAD5 = 0x65,
  PAL_NUMPAD6 = 0x66,
  PAL_NUMPAD7 = 0x67,
  PAL_NUMPAD8 = 0x68,
  PAL_NUMPAD9 = 0x69,
  PAL_MULTIPLY = 0x6A,
  PAL_ADD = 0x6B,
  PAL_SEPARATOR = 0x6C,
  PAL_SUBTRACT = 0x6D,
  PAL_DECIMAL = 0x6E,
  PAL_DIVIDE = 0x6F,
  PAL_F1 = 0x70,
  PAL_F2 = 0x71,
  PAL_F3 = 0x72,
  PAL_F4 = 0x73,
  PAL_F5 = 0x74,
  PAL_F6 = 0x75,
  PAL_F7 = 0x76,
  PAL_F8 = 0x77,
  PAL_F9 = 0x78,
  PAL_F10 = 0x79,
  PAL_F11 = 0x7A,
  PAL_F12 = 0x7B,
  PAL_F13 = 0x7C,
  PAL_F14 = 0x7D,
  PAL_F15 = 0x7E,
  PAL_F16 = 0x7F,
  PAL_F17 = 0x80,
  PAL_F18 = 0x81,
  PAL_F19 = 0x82,
  PAL_F20 = 0x83,
  PAL_F21 = 0x84,
  PAL_F22 = 0x85,
  PAL_F23 = 0x86,
  PAL_F24 = 0x87,
  PAL_NUMLOCK = 0x90,
  PAL_SCRLOCK = 0x91,
  PAL_LSHIFT = 0xA0,
  PAL_RSHIFT = 0xA1,
  PAL_LCONTROL = 0xA2,
  PAL_RCONTROL = 0xA3,
  PAL_LALT = 0xA4,
  PAL_RALT = 0xA5,
  PAL_VOLMUTE = 0xAD,
  PAL_VOLDOWN = 0xAE,
  PAL_VOLUP = 0xAF,
  PAL_MEDIANEXT = 0xB0,
  PAL_MEDIAPREV = 0xB1,
  PAL_MEDIASTOP = 0xB2,
  PAL_MEDIAPLAY = 0xB4,
  PAL_OEM_1 = 0xBA,
  PAL_OEM_PLUS = 0xBB,
  PAL_OEM_COMMA = 0xBC,
  PAL_OEM_MINUS = 0xBD,
  PAL_OEM_PERIOD = 0xBE,
  PAL_OEM_2 = 0xBF,
  PAL_OEM_3 = 0xC0,
  PAL_OEM_4 = 0xDB,
  PAL_OEM_5 = 0xDC,
  PAL_OEM_6 = 0xDD,
  PAL_OEM_7 = 0xDE,
  PAL_OEM_8 = 0xDF,
  PAL_OEM_102 = 0xE2,
  PAL_ALPHANUMERIC = 0xFF
} keyCode;

typedef enum enum_mouseCode {
  PAL_MOUSE_NONE = 0x0000,
  PAL_MOUSE_LEFT = 0x0001,
  PAL_MOUSE_MIDDLE = 0x0010,
  PAL_MOUSE_RIGHT = 0x0100,
  PAL_MOUSE_X1 = 0x1000
} mouseCode;

typedef sflag (*pfPalWinResizeFunc) (palHandle app, int newWidth, int newHeight);

typedef struct tag_palWindowWin32 {
  HWND handle;
  HDC deviceContext;
  HGLRC renderContext;
  BITMAPINFO bitmapInfo;
  RECT windowRect;
  RAWINPUT* rawInputHandle;
  palHandle appHandle;
  pfPalWinResizeFunc eraseFunc;
  sflag shouldQuit;
  sflag resized;
  sflag moved;
  sflag keyPressed;
  sflag mouseMoved;
  sflag focused;
  sflag lockResize;
  int posX;
  int posY;
  int width;
  int height;
  int pixelResolution;
  int lastMousePosX;
  int lastMousePosY;
  int windowMinWidth;
  int windowMinHeight;
  mouseCode lastMouseKey;
  sflag mouseScrolled;
  int keyPressedCount;
  int mouseScrollPos;
  int scrollWheelDelta;
  unsigned int lastKey;
  unsigned int keysPressed[256];
} palWindowWin32;

typedef struct tag_palSoundWin32 {
  int samplesPerSecond;
  long runningSoundBufferIndex;
  int bytesPerSample;
  int secondaryBufferSize;
  int isPlaying;
  LPDIRECTSOUND directSoundHandle;
  LPDIRECTSOUNDBUFFER globalSoundBuffer;
} palSoundWin32;

#define TITLEBUTTONSPAD 48

typedef struct tag_palGlobalsWin32 {
  palWindowWin32* curWinHandle;
  char* clipboardString;
  int captionBarHeight;
  int borderWidth;
  SYSTEMTIME systemTime;
  SYSTEMTIME localTime;
} palGlobalsWin32;

static palGlobalsWin32 globalDataWin32;

/********************************************/
/* @event                                   */
/* PalLib Win32 Event Callback              */
/********************************************/

static sflag global_startMouseTracking = TRUE;

LRESULT CALLBACK pal_win32_event_callback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int x;
	int y;
    int i;
    RECT winRect;
    MINMAXINFO* mmi;
    TRACKMOUSEEVENT trackMouseLeave;
    int BORDERWIDTH = globalDataWin32.borderWidth;
	int TITLEBARWIDTH = globalDataWin32.captionBarHeight;
    palWindowWin32* hwin = globalDataWin32.curWinHandle;
	if(hwin != NULL) {
	  hwin->resized = FALSE;
	  hwin->moved = FALSE;
      hwin->mouseMoved = FALSE;
      hwin->keyPressed = FALSE;
      hwin->mouseScrolled = FALSE;
      trackMouseLeave.dwFlags = TME_LEAVE;
      trackMouseLeave.dwHoverTime = HOVER_DEFAULT;
      trackMouseLeave.cbSize = sizeof(TRACKMOUSEEVENT);
      trackMouseLeave.hwndTrack = hwin->handle;
    }
    
	switch(msg){
		case(WM_CLOSE): {
			if(hwin == NULL) { break; }
			hwin->shouldQuit = TRUE;
			break;
		}
		case(WM_DESTROY): {
			printf("Destroying win32 window\n");
			PostQuitMessage(0);
			break;
		}

		case(WM_KEYDOWN): {
			if(hwin == NULL) { break; }
            hwin->lastKey = wParam;
            hwin->keyPressed = TRUE;
            hwin->keysPressed[(unsigned char)wParam] = TRUE;
			break;
		}

		case(WM_KEYUP): {
			if(hwin == NULL) { break; }
            hwin->keyPressed = FALSE;
            hwin->keysPressed[(unsigned char)wParam] = FALSE;
			break;
		}
 
		case(WM_MOUSELEAVE): {
			if(hwin == NULL) { break; }
            global_startMouseTracking = TRUE;
            hwin->lastMouseKey = 0;
            break;
		}
     
		case(WM_MOUSEMOVE): {
			if(hwin == NULL) { break; }
			hwin->mouseMoved = TRUE;
            hwin->lastMousePosX = GET_X_LPARAM(lParam);
			hwin->lastMousePosY = GET_Y_LPARAM(lParam);
            if(global_startMouseTracking) {
              TrackMouseEvent(&trackMouseLeave);
              global_startMouseTracking = FALSE;
            }
            break;
		}

		case(WM_MOUSEWHEEL): {
			if(hwin == NULL) { break; }
            hwin->mouseScrolled = TRUE;
            hwin->scrollWheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            hwin->mouseScrollPos += hwin->scrollWheelDelta/120;
            break;
		}

		case(WM_LBUTTONDOWN): {
			if(hwin == NULL) { break; }
            hwin->lastMouseKey |= PAL_MOUSE_LEFT;
            break;
		}

		case(WM_MBUTTONDOWN): {
			if(hwin == NULL) { break; }
            hwin->lastMouseKey |= PAL_MOUSE_MIDDLE;
            break;
		}

		case(WM_RBUTTONDOWN): {
			if(hwin == NULL) { break; }
            hwin->lastMouseKey |= PAL_MOUSE_RIGHT;
            break;
		}

		case(WM_LBUTTONUP): {
			if(hwin == NULL) { break; }
            hwin->lastMouseKey ^= PAL_MOUSE_LEFT;
            break;
		}

		case(WM_MBUTTONUP): {
			if(hwin == NULL) { break; }
            hwin->lastMouseKey ^= PAL_MOUSE_MIDDLE;
            break;
		}

		case(WM_RBUTTONUP): {
			if(hwin == NULL) { break; }
            hwin->lastMouseKey ^= PAL_MOUSE_RIGHT;
            break;
		}

		case(WM_SYSKEYDOWN): {
		  if(hwin == NULL) { break; }
          if(wParam == VK_MENU) {
            hwin->keysPressed[PAL_ALT] = TRUE;
            hwin->keysPressed[PAL_LALT] = TRUE;
          }
          break;
		}
		case(WM_SYSKEYUP): {
		  if(hwin == NULL) { break; }
          if(wParam == VK_MENU) {
            hwin->keysPressed[PAL_ALT] = FALSE;
            hwin->keysPressed[PAL_LALT] = FALSE;
          }
          break;
		}

        case(WM_KILLFOCUS): {
			if(hwin == NULL) { break; }
            hwin->focused = FALSE;
            for(i = 0; i < 256; i++) {
              hwin->keysPressed[i] = FALSE;
            }
            hwin->lastMouseKey = PAL_MOUSE_NONE;
            break;
		}
        
        case(WM_SETFOCUS): {
			if(hwin == NULL) { break; }
            hwin->focused = TRUE;
            break;
		}
 
        case(WM_GETMINMAXINFO): {
			if(hwin == NULL) { break; }
            mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = hwin->windowMinWidth;
            mmi->ptMinTrackSize.y = hwin->windowMinHeight;
            break;
		}
 
		case(WM_ERASEBKGND): {
			return 1;
		}

		case(WM_NCHITTEST): {
			if(hwin == NULL) { break; }
            if(hwin->lockResize) { return HTCLIENT; }
            GetWindowRect(hwnd, &winRect);
			x = GET_X_LPARAM(lParam)-winRect.left;
			y = GET_Y_LPARAM(lParam)-winRect.top;
	        BORDERWIDTH = globalDataWin32.borderWidth*hwin->pixelResolution;
			TITLEBARWIDTH = globalDataWin32.captionBarHeight*hwin->pixelResolution;
            if(x >= BORDERWIDTH &&
                x <= winRect.right - winRect.left -
                BORDERWIDTH - TITLEBUTTONSPAD*hwin->pixelResolution &&
				y >= BORDERWIDTH && y <= TITLEBARWIDTH) {
                return HTCAPTION;
			} else if (x > winRect.right - winRect.left - BORDERWIDTH*2 &&
				y > winRect.bottom - winRect.top - BORDERWIDTH*2) {
				return HTBOTTOMRIGHT;
			} else if (x > winRect.right - winRect.left - BORDERWIDTH) {
				return HTRIGHT;
			} else if (y > winRect.bottom - winRect.top - BORDERWIDTH) {
				return HTBOTTOM;
			} else {
        	    return HTCLIENT;
			}
			break;
		}

		case(WM_SIZING): {
			if(hwin == NULL) { break; }
			if(hwin->eraseFunc) {
              hwin->eraseFunc(hwin->appHandle, hwin->width, hwin->height);
            }
            GetWindowRect(hwnd, &(hwin->windowRect));
			hwin->posX = hwin->windowRect.left;
			hwin->posY = hwin->windowRect.top;
			hwin->width = hwin->windowRect.right - hwin->windowRect.left;
			hwin->height = hwin->windowRect.bottom - hwin->windowRect.top;
			hwin->resized = TRUE;
            hwin->focused = TRUE;
            break;
		}

		case(WM_MOVING): {
			if(hwin == NULL) { break; }
            hwin->focused = TRUE;
			if(hwin->eraseFunc) {
              hwin->eraseFunc(hwin->appHandle, hwin->width, hwin->height);
            }
            GetWindowRect(hwnd, &(hwin->windowRect));
			hwin->posX = hwin->windowRect.left;
			hwin->posY = hwin->windowRect.top;
			hwin->width = hwin->windowRect.right - hwin->windowRect.left;
			hwin->height = hwin->windowRect.bottom - hwin->windowRect.top;
			hwin->moved = TRUE;
            hwin->focused = TRUE;
			break;
		}

		default: {
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	return 0;
}

/********************************************/
/* @win                                     */
/* PalLib Win32 Window Creation             */
/********************************************/

sflag pal_win32_init(palHandle* app, palWindowWin32* ret,
    int x, int y, int width, int height, int res, HWND hwnd) {
  int i;
  ret->appHandle = app;
  ret->handle = hwnd;
  ret->width = width;
  ret->height = height;
  ret->posX = x;
  ret->posY = y;
  ret->shouldQuit = FALSE;
  ret->lastMousePosX = 0;
  ret->lastMousePosY = 0;
  ret->deviceContext = GetDC(hwnd);
  ret->keyPressed = FALSE;
  ret->mouseMoved = FALSE;
  ret->mouseScrolled = 0;
  ret->mouseScrollPos = 0;
  ret->pixelResolution = res;
  ret->lastMousePosX = 0;
  ret->lastMousePosY = 0;
  ret->lastMouseKey = 0;
  ret->lockResize = FALSE;
  ret->windowMinWidth = 0;
  ret->windowMinHeight = 0;
  ret->rawInputHandle = NULL;
  for(i = 0; i < 256; i++) {
    ret->keysPressed[i] = FALSE;
  }
  return TRUE;
}

sflag pal_win32_set_style(WNDCLASSEX* winStyle, HINSTANCE hInst, const char* className) {
	ATOM regRet;
	winStyle->lpszClassName = className;
	winStyle->lpfnWndProc = pal_win32_event_callback;
	winStyle->cbSize = sizeof(WNDCLASSEX);
	winStyle->style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winStyle->cbClsExtra = 0;
	winStyle->cbWndExtra = 0;
	winStyle->hInstance = hInst;
	winStyle->hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	winStyle->lpszMenuName = NULL;
	winStyle->hCursor = LoadCursor(NULL, IDC_ARROW);
	winStyle->hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winStyle->hIconSm = LoadIcon(NULL, IDI_APPLICATION);	
	regRet = RegisterClassEx(winStyle);
	if(regRet == 0) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
    return TRUE;
}

sflag pal_win32_create_window(palHandle* app, palWindowWin32* ret,
    int x, int y, int width, int height, int res, const char* title) {
	HWND hwnd = NULL;
	WNDCLASSEX winStyle;
	HINSTANCE hInstance = GetModuleHandle(NULL);
	pal_win32_set_style(&winStyle, hInstance, title);
	if(winStyle.lpszClassName == NULL) { printf("Error hInstance is null\n"); }
	hwnd = CreateWindow(winStyle.lpszClassName,
			title, WS_POPUP | WS_MINIMIZEBOX, x, y, width, height, NULL, NULL, hInstance, NULL);

	if(hwnd == NULL) {
		MessageBox(NULL, "Failed to create Window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
    }
	
    pal_win32_init(app, ret, x, y, width, height, res, hwnd);
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	UpdateWindow(hwnd);
    globalDataWin32.borderWidth = 4;
	globalDataWin32.captionBarHeight = 18;
	globalDataWin32.curWinHandle = NULL;
	globalDataWin32.curWinHandle = ret;
	ret->resized = FALSE;
	ret->moved = FALSE;
	return TRUE;
}

/********************************************/
/* @winGL                                   */
/* PalLib Win32 GL Window Creation          */
/********************************************/

typedef struct tag_palWin32FakeWindow {
  HWND handle;
  HDC deviceContext;
  HGLRC renderContext;
} palWin32FakeWindow;

sflag pal_win32_set_pixel_format(PIXELFORMATDESCRIPTOR* hpf) {
  hpf->nSize = sizeof(PIXELFORMATDESCRIPTOR);
  hpf->nVersion = 1;
  hpf->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  hpf->iPixelType = PFD_TYPE_RGBA;
  hpf->cColorBits = 32;
  hpf->cRedBits = 0;
  hpf->cRedShift = 0;
  hpf->cGreenBits = 0;
  hpf->cGreenShift = 0;
  hpf->cBlueBits = 0;
  hpf->cBlueShift = 0;
  hpf->cAlphaBits = 0;
  hpf->cAlphaShift = 0;
  hpf->cAccumBits = 0;
  hpf->cAccumRedBits = 0;
  hpf->cAccumGreenBits = 0;
  hpf->cAccumBlueBits = 0;
  hpf->cDepthBits = 24;
  hpf->cStencilBits = 8;
  hpf->cAuxBuffers = 0;
  hpf->iLayerType = PFD_MAIN_PLANE;
  hpf->bReserved = 0;
  hpf->dwLayerMask = 0;
  hpf->dwVisibleMask = 0;
  hpf->dwDamageMask = 0;
  return TRUE;
}

sflag pal_win32GL_create_fake_context(palWin32FakeWindow* fw) {
  int pixelFormat;
  PIXELFORMATDESCRIPTOR pf;
  pal_win32_set_pixel_format(&pf);
  if(fw->deviceContext) {
    pixelFormat = ChoosePixelFormat(fw->deviceContext, &pf);  
    if(pixelFormat) {
      if(SetPixelFormat(fw->deviceContext, pixelFormat, &pf)) {
        fw->renderContext = wglCreateContext(fw->deviceContext);
        if(!fw->renderContext) {
          SLOG("Failed to create fake render context\n");
          return FALSE;
        }
      }
    }
  }
  return TRUE;
}

const int pixelAttribs[] = {
  WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
  WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
  WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
  /* WGL_SWAP_EXCHANGE_ARB causes problems with window menu in fullscreen */
  WGL_SWAP_METHOD_ARB, WGL_SWAP_COPY_ARB,
  WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
  WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
  WGL_COLOR_BITS_ARB, 32,
  WGL_ALPHA_BITS_ARB, 8,
  WGL_DEPTH_BITS_ARB, 24,
  0
};

int contextAttributes[] = {
  WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
  WGL_CONTEXT_MINOR_VERSION_ARB, 3,
  WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
  0
};

sflag pal_win32GL_create_real_context(palWindowWin32* ret, palWin32FakeWindow* fw) {
  int wglcret;
  int pixelFormat = 0;
  PIXELFORMATDESCRIPTOR pf;
  unsigned int numFormats = 0;
  pal_win32_set_pixel_format(&pf);
  if(ret->deviceContext) {
    wglMakeCurrent(fw->deviceContext, fw->renderContext);
    wglcret = wglChoosePixelFormatARB(ret->deviceContext, pixelAttribs, NULL, 1, &pixelFormat, &numFormats);
    if(numFormats) {
      DescribePixelFormat(ret->deviceContext, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pf);
      if(SetPixelFormat(ret->deviceContext, pixelFormat, &pf)) {
        ret->renderContext = wglCreateContextAttribsARB(ret->deviceContext, 0, contextAttributes);
        if(!ret->renderContext) {
          SLOG("Error creating render context\n");
        }
      }
      
    }
  }
  return TRUE;
}

sflag pal_win32GL_create_fake_window(palWin32FakeWindow* fw, HINSTANCE hInst, const char* title) {
  fw->handle = CreateWindow(title, title,
      WS_POPUP | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
      0, 0, 1, 1, NULL, NULL, hInst, NULL);
  if(!fw->handle) { SLOG1("Error creating fake window: %ld\n", GetLastError()); return FALSE; }
  fw->deviceContext = GetDC(fw->handle);
  return TRUE;
}

sflag pal_win32GL_create_real_window(palHandle* app, HINSTANCE hInst, palWindowWin32* ret,
    int x, int y, int width, int height, int res, const char* title) {
  ret->handle = CreateWindow(title, title,
      WS_POPUP | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
      x, y, width, height, NULL, NULL, hInst, NULL);
  if(!ret->handle) { SLOG("Error creating real window\n"); return FALSE; }
  ret->deviceContext = GetDC(ret->handle);
  return TRUE;
}

sflag pal_win32GL_destroy_fake_window(palWin32FakeWindow* fw) {
  wglMakeCurrent(fw->deviceContext, NULL);
  wglDeleteContext(fw->renderContext);
  ReleaseDC(fw->handle, fw->deviceContext);
  DestroyWindow(fw->handle);
  fw->renderContext = NULL;
  fw->deviceContext = NULL;
  fw->handle = NULL;
  return TRUE;
}

sflag pal_win32_create_window_gl(palHandle* app, palWindowWin32* ret,
    int x, int y, int width, int height, int res, const char* title) {
  WNDCLASSEX winStyle;
  palWin32FakeWindow fakeWin;
  HINSTANCE hInst = GetModuleHandle(NULL);
  sflag vsync = FALSE;
  sflag result = FALSE;
  char* vendorString;
  char* rendererString;
  pal_win32_set_style(&winStyle, hInst, title);
  SLOG("Creating OpenGL accelerated window\n");
  SLOG("Creating fake window\n");
  pal_win32GL_create_fake_window(&fakeWin, hInst, title);
  SLOG("Creating fake context\n");
  pal_win32GL_create_fake_context(&fakeWin);
  wglMakeCurrent(fakeWin.deviceContext, fakeWin.renderContext);
  if(!wglGetCurrentContext()) { SLOG("Error, current context is NULL\n"); }
  
  SLOG("Loading OpenGL extensions\n");
  if(!pal_gl_load_extension_list()) {
    SLOG1("Error loading OpenGL extensions: %ld\n", GetLastError());
  }
  SLOG("Creating real window\n");
  pal_win32GL_create_real_window(app, hInst, ret, x, y, width, height, res, title);
  SLOG("Creating real context\n");
  pal_win32GL_create_real_context(ret, &fakeWin);
  SLOG("Destroying fake window\n");
  pal_win32GL_destroy_fake_window(&fakeWin);
  SLOG("Making context current\n");
  wglMakeCurrent(ret->deviceContext, ret->renderContext);
  SLOG("Finished creating OpenGL window\n");
  glClearDepth(1.0f);
  glDisable(GL_DEPTH_TEST);
  glFrontFace(GL_CW);
  glDisable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  vendorString = (char*)glGetString(GL_VENDOR);
  rendererString = (char*)glGetString(GL_RENDERER);
  SLOG2("Video card: %s:%s\n", vendorString, rendererString);

  if(vsync == 0 ){
	result = wglSwapIntervalEXT(0);
  } else if (vsync == 1) {
	result = wglSwapIntervalEXT(1);
  }
  if(result != 1) {
	SLOG("OpenGL error!\n");
	return FALSE;
  }
  glViewport(0, 0, width, height);

  pal_win32_init(app, ret, x, y, width, height, res, ret->handle);
  ShowWindow(ret->handle, SW_SHOW);
  SetForegroundWindow(ret->handle);
  SetFocus(ret->handle);
  UpdateWindow(ret->handle);
  globalDataWin32.borderWidth = 4;
  globalDataWin32.captionBarHeight = 18;
  globalDataWin32.curWinHandle = NULL;
  globalDataWin32.curWinHandle = ret;
  ret->resized = FALSE;
  ret->moved = FALSE;
  return TRUE;
}

/********************************************/
/* @memory                                  */
/* PalLib Win32 Memory Allocation           */
/********************************************/

void* pal_win32_alloc(void* ptr, size_t size) {
	void* res = NULL;
	res = VirtualAlloc(ptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	return res;
}

sflag pal_win32_free(void* ptr) {
	if (VirtualFree(ptr, 0, MEM_RELEASE) == 0) {
		return FALSE;
	}
	return TRUE;
}


/* util */

static unsigned char keyState[0xFF];

unsigned char pal_win32_key_to_ascii(int scancode) {
  unsigned short result;
  if(GetKeyboardState(keyState) == FALSE) {
    return FALSE;
  }
  ToAscii(scancode, scancode, keyState, &result, 0);
  return result;
}

unsigned int pal_win32_cstring_len(const char* str) {
	unsigned int index = 0;
	while(index < 50000 && str[index] != '\0') {
		index++;
	}
	if(index == 50000) {
		index = 0;
	}
	return index;
}


/********************************************/
/* @winutils                                */
/* PalLib Win32 Window Utils                */
/********************************************/

sflag pal_win32_poll_events(palWindowWin32* window) {
	MSG msg;
	int i;
    GetWindowRect(window->handle, &window->windowRect);
    GetSystemTime(&globalDataWin32.systemTime);
    GetLocalTime(&globalDataWin32.localTime);
    window->posX = window->windowRect.left;
	window->posY = window->windowRect.top;
	window->width = window->windowRect.right - window->windowRect.left;
	window->height = window->windowRect.bottom - window->windowRect.top;
	window->mouseScrolled = FALSE;
    if(PeekMessage(&msg, window->handle, 0, 0, PM_REMOVE)) {
	  TranslateMessage(&msg);
	  DispatchMessage(&msg);
	}
    window->keyPressedCount = 0;
    for(i = 0; i < 0xFF; i++) {
      if(window->keysPressed[i]) {
        window->keyPressedCount++;
      }
    }
	return TRUE;
}

sflag pal_win32_key_pressed_count(palWindowWin32* window) {
  return window->keyPressedCount;
}

int pal_win32_get_clipboard(char* buffer, unsigned int buffersize) {
  unsigned int i;
  unsigned int len;
  HANDLE h;
  int r = 0;
  if(!OpenClipboard(NULL)) { return FALSE; }
  h = GetClipboardData(CF_TEXT);
  if(h == NULL) { return FALSE; }
  len = pal_win32_cstring_len(((char*)h));
  for(i = 0; i < len; i++) {
    if(i < buffersize) {
      buffer[i] = ((char*)h)[i];
      r++;
    }
  }
  CloseClipboard();
  return r;
}

unsigned int pal_win32_desktop_res_x() {
	return GetSystemMetrics(SM_CXSCREEN);
}

unsigned int pal_win32_desktop_res_y() {
	return GetSystemMetrics(SM_CYSCREEN);
}

sflag pal_win32_minimize(palWindowWin32* win) {
  ShowWindow(win->handle, SW_MINIMIZE);
  return TRUE;
}

sflag pal_win32_set_resolution(palWindowWin32* window, int res) {
  window->pixelResolution = res;
  return TRUE;
}

sflag pal_win32_close(palWindowWin32* window) {
	SendMessage(window->handle, WM_CLOSE, 0, 0);
	return TRUE;
}

sflag pal_win32_should_quit(palWindowWin32* win) {
	return win->shouldQuit;
}

sflag pal_win32_destroy_window(palWindowWin32* win) {
	if(win->handle == NULL) { return FALSE; }
	printf("Destroying window\n");
	DestroyWindow(win->handle);
	if(win->deviceContext) {
		ReleaseDC(win->handle, win->deviceContext);
		win->deviceContext = 0;
	}
	return 0;
}

static int global_prevWinX = 0;
static int global_prevWinY = 0;
static int global_prevWinW = 0;
static int global_prevWinH = 0;

sflag pal_win32_set_window_pos(palWindowWin32* win, int x, int y, int width, int height) {
  global_prevWinX = win->posX;
  global_prevWinY = win->posY;
  global_prevWinW = win->width;
  global_prevWinH = win->height;
  SetWindowPos(win->handle, NULL, x, y, width, height, SWP_SHOWWINDOW);
  return TRUE;
}

sflag pal_win32_set_window_pos_prev(palWindowWin32* win) {
  SetWindowPos(win->handle, NULL, global_prevWinX, global_prevWinY,
      global_prevWinW, global_prevWinH, SWP_SHOWWINDOW);
  return TRUE;
}

/********************************************/
/* @input                                   */
/* PalLib Win32 Input                       */
/********************************************/

int pal_win32_desktop_mouse_x() {
  POINT p;
  GetCursorPos(&p);
  return p.x;
}

int pal_win32_desktop_mouse_y() {
  POINT p;
  GetCursorPos(&p);
  return p.y;
}

int pal_win32_any_key_pressed(palWindowWin32* win) {
  if(win->keyPressed) {
    return win->lastKey;
  }
  return FALSE;
}

sflag pal_win32_key_pressed(palWindowWin32* win, unsigned int key) {
  return win->keysPressed[key];
}

unsigned int pal_win32_last_key(palWindowWin32* win) {
  return win->lastKey;
}

sflag pal_win32_mouse_moved(palWindowWin32* win) {
  return win->mouseMoved;
}

sflag pal_win32_mouse_pressed(palWindowWin32* win, mouseCode mc) {
  return win->lastMouseKey & mc;
}

int pal_win32_mouse_scroll(palWindowWin32* win) {
  return(win->mouseScrollPos);
}

sflag pal_win32_mouse_scrolled(palWindowWin32* win) {
  return (win->mouseScrolled);
}

int pal_win32_mouse_scroll_dir(palWindowWin32* win) {
  return win->scrollWheelDelta/120;
}

sflag pal_win32_async_key_state(unsigned int key) {
  return (GetAsyncKeyState(key) & 0x8000);
}

/********************************************/
/* @graphics                                */
/* PalLib Win32 Graphics                    */
/********************************************/

sflag pal_win32_blit_pixels(palWindowWin32* win, void* memory, int memWidth, int memHeight) {
    win->bitmapInfo.bmiHeader.biSize = sizeof(win->bitmapInfo.bmiHeader);
	win->bitmapInfo.bmiHeader.biWidth = memWidth;
	win->bitmapInfo.bmiHeader.biHeight = -memHeight;
	win->bitmapInfo.bmiHeader.biPlanes = 1;
	win->bitmapInfo.bmiHeader.biBitCount = 32;
	win->bitmapInfo.bmiHeader.biCompression = BI_RGB;
	if(StretchDIBits(win->deviceContext, 0, 0, win->width, win->height,
				0, 0, memWidth, memHeight, memory, &win->bitmapInfo, DIB_RGB_COLORS, SRCCOPY) != 0) {
		return TRUE;
	}
	printf("StretchDIBits error\n");
	return FALSE;
}

/********************************************/
/* @sound                                   */
/* PalLib Win32 Sound                       */
/********************************************/

typedef struct tag_palSampleResult {
  short left;
  short right;
} palSampleResult;

typedef palSampleResult (*pfPalInt16SampleFunc) (void* params);

#define DIRECT_SOUND_CREATE(name) \
HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);


sflag pal_win32_dsound_create(palWindowWin32* win, palSoundWin32* sound, int samplesPerSecond, int bufferSize) {
  /* load direct sound library */
  HMODULE dsLib = LoadLibraryA("dsound.dll");
  DSBUFFERDESC bufferDescPrim;
  DSBUFFERDESC bufferDescSec;
  LPDIRECTSOUNDBUFFER primaryBuffer;
  LPDIRECTSOUND DSound;
  WAVEFORMATEX waveFormat;
  
  sound->samplesPerSecond = samplesPerSecond;
  sound->runningSoundBufferIndex = 0;
  sound->bytesPerSample = sizeof(short)*2;
  sound->secondaryBufferSize = bufferSize;
  sound->directSoundHandle = NULL;
  sound->globalSoundBuffer = NULL;
  sound->isPlaying = 0;
  if(dsLib) {
    /* get direct sound object */
    direct_sound_create* DirectSoundCreate = (direct_sound_create*)GetProcAddress(dsLib, "DirectSoundCreate");
    if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DSound, 0))) {
          waveFormat.wFormatTag = WAVE_FORMAT_PCM;
          waveFormat.nChannels = 2;
          waveFormat.nSamplesPerSec = samplesPerSecond;
          waveFormat.wBitsPerSample = 16;
          waveFormat.nBlockAlign = (waveFormat.nChannels*waveFormat.wBitsPerSample)/8;
          waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec*waveFormat.nBlockAlign;
          waveFormat.cbSize = 0;
      if(SUCCEEDED(DSound->lpVtbl->SetCooperativeLevel(DSound, win->handle, DSSCL_PRIORITY))) {
        ZeroMemory(&bufferDescPrim, sizeof(bufferDescPrim));
        bufferDescPrim.dwSize = sizeof(bufferDescPrim);
        bufferDescPrim.dwFlags = DSBCAPS_PRIMARYBUFFER;

        /* create primary buffer */
        if(SUCCEEDED(DSound->lpVtbl->CreateSoundBuffer(DSound, &bufferDescPrim, &primaryBuffer, 0))) {
          sound->directSoundHandle = DSound;
          if(SUCCEEDED(primaryBuffer->lpVtbl->SetFormat(primaryBuffer, &waveFormat))) {

          } else {
            printf("Direct Sound Error\n");
          }
        } else {
          printf("Direct Sound Error\n");
        }
      } else {
        printf("Direct Sound Error\n");
      }

      /* create secondary buffer */
      ZeroMemory(&bufferDescSec, sizeof(bufferDescSec));
      bufferDescSec.dwSize = sizeof(bufferDescSec);
      bufferDescSec.dwFlags = DSBCAPS_GLOBALFOCUS;
      bufferDescSec.dwBufferBytes = bufferSize;
      bufferDescSec.lpwfxFormat = &waveFormat;
      if(SUCCEEDED(DSound->lpVtbl->CreateSoundBuffer(DSound, &bufferDescSec,
              &sound->globalSoundBuffer, 0))) {
        /* printf("Succesfully created secondary sound buffer\n"); */
      }
    } else {
      printf("Direct Sound Error\n");
    }
  } else {
    printf("Direct Sound Error\n");
  }
  return TRUE;
}

sflag pal_win32_dsound_stop(palSoundWin32* sound) {
  sound->globalSoundBuffer->lpVtbl->Stop(sound->globalSoundBuffer);
  sound->isPlaying = 0;
  return TRUE;
}

sflag pal_win32_dsound_play(palSoundWin32* sound) {
  sound->globalSoundBuffer->lpVtbl->Play(sound->globalSoundBuffer, 0, 0, DSBPLAY_LOOPING);
  sound->isPlaying = 1;
  return TRUE;
}

sflag pal_win32_dsound_fill_buffer(palSoundWin32* sound, DWORD byteToLock, DWORD bytesToWrite,
    pfPalInt16SampleFunc sampleFunc, void* sampleParams) {
  void* region1;
  DWORD region1Size;
  DWORD region2SampleCnt;
  void* region2;
  DWORD region2Size;
  DWORD region1SampleCnt;
  DWORD sampleIndex;
  short* sampleOut;
  palSampleResult sr;
  if(SUCCEEDED(sound->globalSoundBuffer->lpVtbl->Lock(sound->globalSoundBuffer, byteToLock, bytesToWrite,
                                                  &region1, &region1Size,
                                                  &region2, &region2Size,
                                                  0))) {
    region1SampleCnt = region1Size/sound->bytesPerSample;
    sampleOut = (short*)region1;
    for(sampleIndex = 0; sampleIndex < region1SampleCnt; ++sampleIndex) {
      sr = sampleFunc(sampleParams);
      sound->runningSoundBufferIndex++;
      *sampleOut++ = sr.left;
      *sampleOut++ = sr.right;
    }
    region2SampleCnt = region2Size/sound->bytesPerSample;
    sampleOut = (short*)region2;
    for(sampleIndex = 0; sampleIndex < region2SampleCnt; ++sampleIndex) {
      sr = sampleFunc(sampleParams);
      sound->runningSoundBufferIndex++;
      *sampleOut++ = sr.left;
      *sampleOut++ = sr.right;
    }
    sound->globalSoundBuffer->lpVtbl->Unlock(sound->globalSoundBuffer,
        region1, region1Size, region2, region2Size);
  }
  return TRUE;
}

sflag pal_win32_dsound_update(palSoundWin32* sound, pfPalInt16SampleFunc sampleFunc, void* sampleParams) {
  DWORD byteToLock = 0;
  DWORD bytesToWrite = 0;
  DWORD writeCursor = 0;
  DWORD playCursor = 0;
  if(SUCCEEDED(sound->globalSoundBuffer->lpVtbl->GetCurrentPosition(sound->globalSoundBuffer,
          &playCursor, &writeCursor))) {
    byteToLock = (sound->runningSoundBufferIndex*sound->bytesPerSample) % sound->secondaryBufferSize;
    if(byteToLock == playCursor) {
      if(!sound->isPlaying) {
        bytesToWrite = sound->secondaryBufferSize;
      } else {
        bytesToWrite = 0;
      }
    } else if(byteToLock > playCursor) {
      bytesToWrite = (sound->secondaryBufferSize - byteToLock);
      bytesToWrite += playCursor;
    } else {
      bytesToWrite = playCursor - byteToLock;
    }
    pal_win32_dsound_fill_buffer(sound, byteToLock, bytesToWrite, sampleFunc, sampleParams);
  }
  return TRUE;
}

/********************************************/
/* @thread                                  */
/* PalLib Win32 Threads                     */
/********************************************/

typedef struct tag_palThreadWin32 {
  DWORD threadID;
  HANDLE threadHandle;
} palThreadWin32;

typedef DWORD (*pfWin32ThreadFunc) (LPVOID args);

typedef int (*pfThreadFunc) (void* args);

#define THREADFUNC pfThreadFunc

sflag pal_win32_thread_create_internal(palThreadWin32* st, pfThreadFunc func, void** args) {
  pfWin32ThreadFunc tFunc = (pfWin32ThreadFunc)func;
  LPVOID tArgs = (LPVOID)args;
  st->threadHandle = CreateThread(NULL, 0, tFunc, tArgs, 0, &st->threadID);
  if(st->threadHandle == NULL) {
    return FALSE;
  }
  return TRUE;
}

sflag pal_win32_thread_free_internal(palThreadWin32* st) {
  sflag ret = FALSE;
  if(st != NULL) {
    if(CloseHandle(st->threadHandle) == 0) {
      ret = FALSE;
    }
    st->threadHandle = NULL;
    ret = TRUE;
  } else {
    ret = FALSE;
  }
  return ret;
}

sflag pal_win32_thread_create(palThreadWin32* st, pfThreadFunc func, void** args) {
  sflag ret = FALSE;
  if(st == NULL) { return FALSE; }
  ret = pal_win32_thread_create_internal(st, func, args);
  if(ret == FALSE) {
    SLOG1("Error creating win32 thread %p\n", (void*)st);
  }
  return ret;
}

sflag pal_win32_thread_free(palThreadWin32* st) {
  sflag ret = FALSE;
  if(st == NULL) { return FALSE; }
  WaitForSingleObject(st->threadHandle, INFINITE);
  ret = pal_win32_thread_free_internal(st);
  if(ret == FALSE) {
    SLOG1("Error freeing thread %p\n", (void*)st);
  }
  return ret;
}

/********************************************/
/* @dll                                     */
/* PalLib Win32 Dynamic Libs                */
/********************************************/

typedef struct tag_palLibraryWin32 {
  HINSTANCE hInst;
} palLibraryWin32;

sflag pal_win32_load_library(palLibraryWin32* lib, const char* filename) {
  lib->hInst = LoadLibrary(filename);
  if(lib->hInst == NULL) {
     return FALSE;
  }
  return TRUE;
}

typedef sflag (*pfPalFunc)(void* args);

pfPalFunc pal_win32_library_load_function(palLibraryWin32* lib, const char* funcname) {
  return (pfPalFunc)GetProcAddress(lib->hInst, funcname);
}

sflag pal_win32_free_library(palLibraryWin32* lib) {
  FreeLibrary(lib->hInst);
  return TRUE;
}

/********************************************/
/* @time                                    */
/* PalLib Win32 Timing                      */
/********************************************/

typedef enum enum_timeScale {
  PAL_MILISECONDS,
  PAL_SECONDS,
  PAL_MINUTES,
  PAL_HOURS,
  PAL_DAYS,
  PAL_MONTHS,
  PAL_YEARS
} timeScale;

static unsigned long performanceFreq;

unsigned long pal_win32_get_local_time(timeScale ts) {
  unsigned long r;
  switch(ts) {
    case(PAL_MILISECONDS): { r = globalDataWin32.localTime.wMilliseconds; break; }
    case(PAL_SECONDS): { r = globalDataWin32.localTime.wSecond; break; }
    case(PAL_MINUTES): { r = globalDataWin32.localTime.wMinute; break; }
    case(PAL_HOURS): { r = globalDataWin32.localTime.wHour; break; }
    case(PAL_DAYS): { r = globalDataWin32.localTime.wDay; break; }
    case(PAL_MONTHS): { r = globalDataWin32.localTime.wMonth; break; }
    case(PAL_YEARS): { r = globalDataWin32.localTime.wYear; break; }
    default: { r = 0; break; }
  }
  return r;
}

unsigned long pal_win32_get_system_time(timeScale ts) {
  unsigned long r;
  switch(ts) {
    case(PAL_MILISECONDS): { r = globalDataWin32.systemTime.wMilliseconds; break; }
    case(PAL_SECONDS): { r = globalDataWin32.systemTime.wSecond; break; }
    case(PAL_MINUTES): { r = globalDataWin32.systemTime.wMinute; break; }
    case(PAL_HOURS): { r = globalDataWin32.systemTime.wHour; break; }
    case(PAL_DAYS): { r = globalDataWin32.systemTime.wDay; break; }
    case(PAL_MONTHS): { r = globalDataWin32.systemTime.wMonth; break; }
    case(PAL_YEARS): { r = globalDataWin32.systemTime.wYear; break; }
    default: { r = 0; break; }
  }
  return r;
}

sflag pal_win32_time_set_scheduler_ms(int ms) {
	MMRESULT ret = timeBeginPeriod(ms);
	if(ret == TIMERR_NOERROR) {
		return TRUE;
	}
	return FALSE;
}

sflag pal_win32_time_reset_scheduler_ms(int ms) {
	MMRESULT ret = timeEndPeriod(ms);
	if(ret == TIMERR_NOERROR) {
		return TRUE;
	}
	return FALSE;
}

sflag pal_win32_time_init() {
	LARGE_INTEGER queryRes; 
	QueryPerformanceFrequency(&queryRes);
	performanceFreq = queryRes.QuadPart;
	return TRUE;
}

double pal_win32_time_delta(LARGE_INTEGER begin, LARGE_INTEGER end) {
	return ((double)(end.QuadPart - begin.QuadPart) / performanceFreq);
}

LARGE_INTEGER pal_win32_time_get_clock() {
	LARGE_INTEGER res;
	QueryPerformanceCounter(&res);
	return res;
}

sflag pal_win32_sleep(unsigned int ms) {
 Sleep(ms);
 return TRUE;
}

/********************************************/
/* @file                                    */
/* PalLib Win32 Files                       */
/********************************************/


typedef WIN32_FIND_DATA palFindData;

typedef struct tag_palWin32FileData {
  palFindData rawData;
  sflag isDir;
  sflag isNormal;
  sflag isHidden;
  unsigned int indexInDir;
  const char* cName;
  unsigned int ftMil[2];
  unsigned int ftSec[2];
  unsigned int ftMin[2];
  unsigned int ftHrs[2];
  unsigned int ftDay[2];
  unsigned int ftMth[2];
  unsigned int ftYer[2];
} palWin32FileData;

typedef palWin32FileData palFileData;

typedef sflag (*pfPalWin32FileListFunc) (palFileData* data, palHandle args);

sflag pal_win32_file_list_dir(palHandle* hApp, const char* directory,
    const char* format, pfPalWin32FileListFunc listFunc, palHandle args) {
  HANDLE hFind;
  SYSTEMTIME stc;
  SYSTEMTIME stm;
  palFindData findData;
  palFileData data;
  unsigned int dirIndex = 0;
  char buffer[MAX_PATH];
  if(args == NULL || hApp == NULL)  { SLOG("args were NULL\n"); return FALSE; }
  if(pal_win32_cstring_len(directory) > (MAX_PATH - 3)) {
    SLOG("String too long\n");
    return FALSE;
  }
  sprintf(buffer, "%s\\%s", directory, format);
  hFind = FindFirstFile(buffer, &findData);
  if(hFind == INVALID_HANDLE_VALUE) {
    SLOG1("Error finding %s\n", buffer);
    return FALSE;
  }
  dirIndex++;
  do {
    data.isDir = FALSE;
    data.isNormal = FALSE;
    data.isHidden = FALSE;
    if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      data.isDir = TRUE;
    }
    if(findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
      data.isNormal = TRUE;
    }
    if(findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
      data.isHidden = TRUE;
    }
    FileTimeToSystemTime(&findData.ftCreationTime, &stc);
    FileTimeToSystemTime(&findData.ftCreationTime, &stm);
    data.indexInDir = dirIndex;
    data.cName = findData.cFileName;
    data.rawData = findData;
    data.ftMil[0] = stc.wMilliseconds;  data.ftMil[1] = stm.wMilliseconds;
    data.ftSec[0] = stc.wSecond;        data.ftSec[1] = stm.wSecond;
    data.ftMin[0] = stc.wMinute;        data.ftMin[1] = stm.wMinute;
    data.ftHrs[0] = stc.wHour;          data.ftHrs[1] = stm.wHour;
    data.ftDay[0] = stc.wDay;           data.ftDay[1] = stm.wDay;
    data.ftMth[0] = stc.wMonth;         data.ftMth[1] = stm.wMonth;
    data.ftYer[0] = stc.wYear;          data.ftYer[1] = stm.wYear;
    listFunc(&data, args);
    dirIndex++;
  } while(FindNextFile(hFind, &findData) != 0);
  FindClose(hFind);
  return TRUE;
}

sflag pal_win32_get_dir_string (const char* filename, unsigned int len, char* buffer) {
  return GetFullPathName(filename, len, buffer, NULL);
}

sflag pal_win32_get_current_dir(unsigned int bufferLen, char* buffer) {
  return GetCurrentDirectory(bufferLen, buffer);
}

FILETIME pal_win32_file_get_last_write(char* filename) {
	FILETIME lastWriteTime;
	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFileA(filename, &findData);
	lastWriteTime.dwHighDateTime = 0;
	lastWriteTime.dwLowDateTime = 0;
	if(findHandle != INVALID_HANDLE_VALUE) {
		FindClose(findHandle);
		lastWriteTime = findData.ftLastWriteTime;
	}
	return lastWriteTime;
}

sflag pal_win32_file_time_compare_equal(FILETIME a, FILETIME b) {
	return !CompareFileTime(&a,&b);
}

sflag pal_win32_delete_file(const char* file) {
  DeleteFile(file);
  return TRUE;
}

sflag pal_win32_copy_file(const char* from, const char* to, sflag bailIfExists) {
  CopyFile(from, to, bailIfExists);
  return TRUE;
}

sflag pal_win32_set_window_min(palWindowWin32* win, int width, int height) {
  win->windowMinWidth = width;
  win->windowMinHeight = height;
  return TRUE;
}

sflag pal_win32_is_focused(palWindowWin32* win) {
  return win->handle == GetForegroundWindow();
}

#endif
