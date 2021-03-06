#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <string>
#include "d3dwin.h"
#include "dt2003\common.h"

D3DWin* L33T;

D3DWin::D3DWin(const char *title, unsigned int width, unsigned int height, D3DFORMAT format, bool fullscreen) : direct3d(0), device(0), window(0) {
	instance = GetModuleHandle(NULL);

	UINT32 style;
	if (!fullscreen) style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	else style = WS_POPUP;

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;

	if(!fullscreen) AdjustWindowRect(&rect, style, false);

	WNDCLASS wc;
	wc.style = CS_VREDRAW|CS_HREDRAW;
	wc.lpfnWndProc = window_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instance;
	wc.hIcon = 0;
	wc.hCursor = LoadCursor(instance,IDC_ARROW);
	wc.hbrBackground = 0;
	wc.lpszMenuName = 0;
	wc.lpszClassName = "d3d9";
	RegisterClass(&wc);
	
	window = CreateWindowEx(0, "d3d9", title, style | WS_VISIBLE, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, 0, instance, 0);
	if (!window) throw std::string("CreateWindowEx failed");

	direct3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!direct3d) throw std::string("Direct3DCreate9() failed");

	D3DPRESENT_PARAMETERS presentparameters;
	memset(&presentparameters, 0, sizeof(D3DPRESENT_PARAMETERS));
	presentparameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentparameters.BackBufferWidth = width;
	presentparameters.BackBufferHeight = height;
	presentparameters.AutoDepthStencilFormat = D3DFMT_D24S8;
	presentparameters.EnableAutoDepthStencil = TRUE;
	presentparameters.Windowed = !fullscreen;
	presentparameters.MultiSampleQuality = D3DMULTISAMPLE_NONE;
//	presentparameters.MultiSampleQuality = D3DMULTISAMPLE_2_SAMPLES;

	D3DDISPLAYMODE displaymode;
	if (!fullscreen) {
		if(FAILED(direct3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displaymode))) throw std::string("GetAdapterDisplayMode() failed");
		presentparameters.BackBufferFormat = displaymode.Format;
	} else {
		presentparameters.BackBufferFormat = format;
		ShowCursor(FALSE);
	}

	if (FAILED(direct3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentparameters, &device)))
		throw std::string("CreateDevice() failed");

	pause = false;
	L33T = this;
}

D3DWin::~D3DWin() {
	ShowCursor(TRUE);

	if (device) {
		device->EvictManagedResources();
		device->Release();
		device = 0;
	}

	if (direct3d) {
		direct3d->Release();
		direct3d = 0;
	}

	if (window) {
		DestroyWindow(window);
		window = 0;
	}

	UnregisterClass("d3d9", instance);
}

LRESULT CALLBACK D3DWin::window_proc(HWND win,UINT message,WPARAM wparam,LPARAM lparam) {
	switch (message) {

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	break;

	case CM_PAUSE:
		L33T->pause = !L33T->pause;
		if(L33T->pause) {
			//FMUSIC_SetPaused(mod,TRUE);
			BASS_Pause();
		} else {
			L33T->sync->load();
			//long long bytes_played = BASS_ChannelGetPosition(music_file);
			//return bytes_played * (1.0 / (44100 * 2 * 2));
			BASS_ChannelSetPosition(L33T->player->music_file,(wparam/8.0f) / ((1.0 / (44100 * 2 * 2))) / (float(BPM) / 60));
			//FSOUND_Stream_SetTime(mod,wParam*secsperrow*1000);
			//FMUSIC_SetPaused(mod,FALSE);
			BASS_Start();
		}
		return 0;
		break;

	case CM_RELOAD:
		L33T->sync->load();
		return 0;
		break;

	case CM_MOVEPOS:
		if(L33T->pause) {
			L33T->sync->load();
			BASS_ChannelSetPosition(L33T->player->music_file,(wparam/8.0f) / ((1.0 / (44100 * 2 * 2))) / (float(BPM) / 60));
			BASS_Pause();
		}
		return 0;
		break;

	case WM_SYSCOMMAND:
		switch (wparam) {
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		}
	break;

	}
    return DefWindowProc(win,message,wparam,lparam);
}
