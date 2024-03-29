#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "SDL.h"
#include "oglfunc.h"

#if !defined(_MSC_VER)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>
#endif

#include "fixer.h"

#include "3dc.h"
#undef No
#include "platform.h"
#include "inline.h"
#include "gamedef.h"
#include "gameplat.h"
#include "ffstdio.h"
#include "vision.h"
#include "comp_shp.h"
#include "avp_envinfo.h"
#include "stratdef.h"
#include "bh_types.h"
#include "avp_userprofile.h"
#include "pldnet.h"
#include "cdtrackselection.h"
#include "gammacontrol.h"
#include "opengl.h"
#include "avp_menus.h"
#include "avp_mp_config.h"
#include "npcsetup.h"
#include "cdplayer.h"
#include "hud.h"
#include "player.h"
#include "mempool.h"
#include "avpview.h"
#include "consbind.hpp"
#include "progress_bar.h"
#include "scrshot.hpp"
#include "version.h"
#include "fmv.h"

#if EMSCRIPTEN
#include <emscripten.h>
#endif

#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__SWITCH__) || defined(VITA)
#define FIXED_WINDOW_SIZE 1
#endif

#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__SWITCH__) || defined(VITA)
#define USE_OPENGL_ES 1
#endif

#ifdef __vita__
#include <vitasdk.h>
#endif

extern uint16_t *gIndices;
extern uint16_t *gIndicesPtr;
extern float *gVertexBuffer;
extern float *gVertexBufferPtr;
uint16_t *fb_pixels = NULL;
uint16_t *soft_fb[2];
SceGxmTexture *old_gxm_texture;
SceGxmTexture *curr_gxm_texture;
SceGxmTexture gxm_tex_storage;

static void main_loop(void);

void RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR(char Ch);
void RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(int wParam);

static int SDLCALL SDLEventFilter(void* userData, SDL_Event* event);

char LevelName[] = {"predbit6\0QuiteALongNameActually"}; /* the real way to load levels */

int DebouncedGotAnyKey;
unsigned char DebouncedKeyboardInput[MAX_NUMBER_OF_INPUT_KEYS];
int GotJoystick;
int GotMouse;
int JoystickEnabled;
int MouseVelX;
int MouseVelY;

extern int ScanDrawMode;
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern unsigned char KeyboardInput[MAX_NUMBER_OF_INPUT_KEYS];
extern unsigned char GotAnyKey;
extern int NormalFrameTime;

SDL_Window *window;

SDL_Joystick *joy;
JOYINFOEX JoystickData;
JOYCAPS JoystickCaps;

static int main_loop_state = 0;

// Window configuration and state
static int WindowWidth;
static int WindowHeight;
static int ViewportWidth;
static int ViewportHeight;
static int DrawableWidth;
static int DrawableHeight;

enum RENDERING_MODE {
	RENDERING_MODE_SOFTWARE,
	RENDERING_MODE_OPENGL
};

enum RENDERING_MODE RenderingMode;

#if defined(FIXED_WINDOW_SIZE)
static int WantFullscreen = 1;
static int WantFullscreenToggle = 0;
static int WantResolutionChange = 0;
static int WantMouseGrab = 1;
#else
static int WantFullscreen = 0;
static int WantFullscreenToggle = 1;
static int WantResolutionChange = 1;
static int WantMouseGrab = 0;
#endif

// Additional configuration
int WantSound = 1;
static int WantCDRom = 0;
static int WantJoystick = 1;

static GLuint FullscreenTexture;
static GLsizei FullscreenTextureWidth;
static GLsizei FullscreenTextureHeight;

static GLuint FullscreenArrayBuffer;
static GLuint FullscreenElementArrayBuffer;

static GLuint FramebufferTexture;
static GLsizei FramebufferTextureWidth;
static GLsizei FramebufferTextureHeight;
static GLuint FramebufferObject;
static GLuint FramebufferDepthObject;

/* originally was "/usr/lib/libGL.so.1:/usr/lib/tls/libGL.so.1:/usr/X11R6/lib/libGL.so" */
static const char * opengl_library = NULL;

/* ** */

static void IngameKeyboardInput_ClearBuffer(void)
{
	// clear the keyboard state
	memset((void*) KeyboardInput, 0, MAX_NUMBER_OF_INPUT_KEYS);
	GotAnyKey = 0;
}

void DirectReadKeyboard()
{
}

void DirectReadMouse()
{
}

void ReadJoysticks()
{
	int axes, balls, hats;
	Uint8 hat;
	
	JoystickData.dwXpos = 0;
	JoystickData.dwYpos = 0;
	JoystickData.dw2Xpos = 0;
	JoystickData.dw2Ypos = 0;
	JoystickData.dwRpos = 0;
	JoystickData.dwUpos = 0;
	JoystickData.dwVpos = 0;
	JoystickData.dwPOV = (DWORD) -1;	
	
	if (joy == NULL || !GotJoystick) {
		return;
	}

	SDL_JoystickUpdate();
	
	axes = SDL_JoystickNumAxes(joy);
	balls = SDL_JoystickNumBalls(joy);
	hats = SDL_JoystickNumHats(joy);
	
	if (axes > 0) {
		JoystickData.dwXpos = SDL_JoystickGetAxis(joy, 0) + 32768;
	}
	if (axes > 1) {
		JoystickData.dwYpos = SDL_JoystickGetAxis(joy, 1) + 32768;
	}
	if (axes > 2) {
		JoystickData.dw2Xpos = SDL_JoystickGetAxis(joy, 2) + 32768;
	}
	if (axes > 3) {
		JoystickData.dw2Ypos = SDL_JoystickGetAxis(joy, 3) + 32768;
	}
	
	if (hats > 0) {
		hat = SDL_JoystickGetHat(joy, 0);
		
		switch (hat) {
			default:
			case SDL_HAT_CENTERED:
				JoystickData.dwPOV = (DWORD) -1;
				break;
			case SDL_HAT_UP:
				JoystickData.dwPOV = 0;
				break;
			case SDL_HAT_RIGHT:
				JoystickData.dwPOV = 9000;
				break;
			case SDL_HAT_DOWN:
				JoystickData.dwPOV = 18000;
				break;
			case SDL_HAT_LEFT:
				JoystickData.dwPOV = 27000;
				break;
			case SDL_HAT_RIGHTUP:
				JoystickData.dwPOV = 4500;
				break;
			case SDL_HAT_RIGHTDOWN:
				JoystickData.dwPOV = 13500;
				break;
			case SDL_HAT_LEFTUP:
				JoystickData.dwPOV = 31500;
				break;
			case SDL_HAT_LEFTDOWN:
				JoystickData.dwPOV = 22500;
				break;
		}
	}
}

/* ** */

unsigned char *GetScreenShot24(int *width, int *height)
{
	unsigned char *buf;

	if (fb_pixels == NULL) {
		return NULL;
	}

	if (RenderingMode == RENDERING_MODE_OPENGL) {
		buf = (unsigned char *)malloc(DrawableWidth * DrawableHeight * 3);

		*width = DrawableWidth;
		*height = DrawableHeight;

		glReadPixels(0, 0, DrawableWidth, DrawableHeight, GL_RGB, GL_UNSIGNED_BYTE, buf);
	} else {
		buf = (unsigned char *)malloc(640 * 480 * 3);

		unsigned char *ptrd;
		unsigned short int *ptrs;
		int x, y;
		
		ptrd = buf;
		for (y = 0; y < 480; y++) {
			ptrs = (unsigned short *)(((unsigned char *)fb_pixels) + (480-y-1)*640);
			for (x = 0; x < 640; x++) {
				unsigned int c;
				
				c = *ptrs;
				ptrd[0] = (c & 0xF800)>>8;
				ptrd[1] = (c & 0x07E0)>>3;
				ptrd[2] = (c & 0x001F)<<3;
				
				ptrs++;
				ptrd += 3;
			}
		}
		
		*width = 640;
		*height = 480;

	}

#if 0
	Uint16 redtable[256], greentable[256], bluetable[256];
	
	if (SDL_GetGammaRamp(redtable, greentable, bluetable) != -1) {
		unsigned char *ptr;
		int i;
		
		ptr = buf;
		for (i = 0; i < 640*480; i++) {
			ptr[i*3+0] = redtable[ptr[i*3+0]]>>8;
			ptr[i*3+1] = greentable[ptr[i*3+1]]>>8;
			ptr[i*3+2] = bluetable[ptr[i*3+2]]>>8;
			ptr += 3;
		}
	}
#endif

	return buf;
}

/* ** */

PROCESSORTYPES ReadProcessorType()
{
	return PType_PentiumMMX;
}

/* ** */

typedef struct VideoModeStruct
{
	int w;
	int h;
	int available;
} VideoModeStruct;
VideoModeStruct VideoModeList[] = {
	{ 	960, 	544,	0	},
	{	960,	544,	0	},
	{	960,	544,	0	},
	{	960,	544,	0	},
	{	960,	544,	0	},
	{	960,    544,	0	},
	{	960,	544,	0	},
	{	960,	544,	0	},
	{	960,	544,	0	},
	{	960,	544,	0	}
};

int CurrentVideoMode;
const int TotalVideoModes = sizeof(VideoModeList) / sizeof(VideoModeList[0]);

void LoadDeviceAndVideoModePreferences()
{
	FILE *fp;
	int mode;
	
	fp = OpenGameFile("AvP_TempVideo.cfg", FILEMODE_READONLY, FILETYPE_CONFIG);
	
	if (fp != NULL) {
		// fullscreen mode (0=window,1=fullscreen,2=fullscreen desktop)
		// window width
		// window height
		// fullscreen width
		// fullscreen height
		// fullscreen desktop aspect ratio n
		// fullscreen desktop aspect ratio d
		// fullscreen desktop scale n
		// fullscreen desktop scale d
		// multisample number of samples (0/2/4)
	 	if (fscanf(fp, "%d", &mode) == 1) {
			fclose(fp);
		
			if (mode >= 0 && mode < TotalVideoModes && VideoModeList[mode].available) {
				CurrentVideoMode = mode;
				return;
			}
		} else {
			fclose(fp);
		}
	}
	
	/* No, or invalid, mode found */
	
	/* Try 640x480 first */
	if (VideoModeList[1].available) {
		CurrentVideoMode = 1;
	} else {
		int i;
		
		for (i = 0; i < TotalVideoModes; i++) {
			if (VideoModeList[i].available) {
				CurrentVideoMode = i;
				break;
			}
		}
	}
}

void SaveDeviceAndVideoModePreferences()
{
	FILE *fp;
	
	fp = OpenGameFile("AvP_TempVideo.cfg", FILEMODE_WRITEONLY, FILETYPE_CONFIG);
	if (fp != NULL) {
		fprintf(fp, "%d\n", CurrentVideoMode);
		fclose(fp);
	}
}

void PreviousVideoMode2()
{
	int cur = CurrentVideoMode;

	do {
		if (cur == 0)
			cur = TotalVideoModes;	
		cur--;
		if (cur == CurrentVideoMode)
			return;
	} while(!VideoModeList[cur].available);
	
	CurrentVideoMode = cur;
}

void NextVideoMode2()
{
	int cur = CurrentVideoMode;

	do {
		cur++;
		if (cur == TotalVideoModes)
			cur = 0;

		if (cur == CurrentVideoMode)
			return;
	} while(!VideoModeList[cur].available);
	
	CurrentVideoMode = cur;
}

char *GetVideoModeDescription2()
{
	return "SDL2";
}

char *GetVideoModeDescription3()
{
	static char buf[64];
	
	_snprintf(buf, 64, "%dx%d", VideoModeList[CurrentVideoMode].w, VideoModeList[CurrentVideoMode].h);

	return buf;
}

int InitSDL()
{
#if EMSCRIPTEN
	printf("Setting main loop...\n");
	emscripten_set_main_loop(main_loop, 0, 0);
#endif

	SDL_AddEventWatch(SDLEventFilter, NULL);

{
	int i;
	
	for (i = 0; i < TotalVideoModes; i++) {
		//if (SDL_VideoModeOK(VideoModeList[i].w, VideoModeList[i].h, 16, SDL_FULLSCREEN | SDL_OPENGL)) {
			/* assume SDL isn't lying to us */
			VideoModeList[i].available = 1;
			
			//foundit = 1;
		//}
	}
	
	soft_fb[0] = (uint16_t*)malloc(640*480*2);
	soft_fb[1] = (uint16_t*)malloc(640*480*2);
	fb_pixels = soft_fb[0];
}

	LoadDeviceAndVideoModePreferences();

	if (WantJoystick) {
		SDL_InitSubSystem(SDL_INIT_JOYSTICK);
		
		if (SDL_NumJoysticks() > 0) {
			/* TODO: make joystick number a configuration parameter */
			
			joy = SDL_JoystickOpen(0);
			if (joy) {
				GotJoystick = 1;
			}
			
			JoystickCaps.wCaps = 0; /* no rudder... ? */
			
			JoystickData.dwXpos = 0;
			JoystickData.dw2Xpos = 0;
			JoystickData.dwYpos = 0;
			JoystickData.dw2Ypos = 0;
			JoystickData.dwRpos = 0;
			JoystickData.dwUpos = 0;
			JoystickData.dwVpos = 0;
			JoystickData.dwPOV = (DWORD) -1;
		}
	}

	return 0;
}

static void SetWindowSize(int PhysicalWidth, int PhysicalHeight, int VirtualWidth, int VirtualHeight)
{
	ViewportWidth = PhysicalWidth;
	ViewportHeight = PhysicalHeight;
	DrawableWidth = ViewportWidth;
	DrawableHeight = ViewportHeight;

	ScreenDescriptorBlock.SDB_Width     = VirtualWidth;
	ScreenDescriptorBlock.SDB_Height    = VirtualHeight;
	ScreenDescriptorBlock.SDB_CentreX   = VirtualWidth/2;
	ScreenDescriptorBlock.SDB_CentreY   = VirtualHeight/2;
	ScreenDescriptorBlock.SDB_ProjX     = VirtualWidth/2;
	ScreenDescriptorBlock.SDB_ProjY     = VirtualHeight/2;
	ScreenDescriptorBlock.SDB_ClipLeft  = 0;
	ScreenDescriptorBlock.SDB_ClipRight = VirtualWidth;
	ScreenDescriptorBlock.SDB_ClipUp    = 0;
	ScreenDescriptorBlock.SDB_ClipDown  = VirtualHeight;

	glViewport(0, 0, DrawableWidth, DrawableHeight);
}

static int SetSoftVideoMode(int Width, int Height, int Depth)
{
	//TODO: clear surface

	RenderingMode = RENDERING_MODE_SOFTWARE;
	ScanDrawMode = ScanDrawD3DHardwareRGB;
	GotMouse = 1;

	// reset input
	IngameKeyboardInput_ClearBuffer();

	SetWindowSize(ViewportWidth, ViewportHeight, Width, Height);

	return 0;
}

/* ** */
static void load_opengl_library(const char *lib)
{
}

/* ** */
static int SDLCALL SDLEventFilter(void* userData, SDL_Event* event) {
	(void) userData;

	//printf("SDLEventFilter: %d\n", event->type);
	
	switch (event->type) {
		case SDL_APP_TERMINATING:
			AvP.MainLoopRunning = 0; /* TODO */
			break;
	}
	
	return 1;
}

static int InitSDLVideo(void) {
	return 0;
}

static int SetOGLVideoMode(int Width, int Height)
{
	GLenum status;
	int firsttime = 0;
	int oldflags;
	int flags;
	
	RenderingMode = RENDERING_MODE_OPENGL;
	ScanDrawMode = ScanDrawD3DHardwareRGB;
	GotMouse = 1;
	
	Width = 960;
	Height = 544;

	if (window == NULL) {
		firsttime = 1;

		load_ogl_functions(0);

		// reset input
		IngameKeyboardInput_ClearBuffer();

		load_opengl_library(opengl_library);

		load_ogl_functions(1);

		glViewport(0, 0, Width, Height);

		// create fullscreen window texture
		glGenTextures(1, &FullscreenTexture);
		glBindTexture(GL_TEXTURE_2D, FullscreenTexture);
		
		// Hijacking GXM texture for faster access and callbacks skipping
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FullscreenTextureWidth, FullscreenTextureHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
		curr_gxm_texture = vglGetGxmTexture(GL_TEXTURE_2D);
		vglFree(vglGetTexDataPointer(GL_TEXTURE_2D));
		sceGxmTextureInitLinear(curr_gxm_texture, soft_fb[0], SCE_GXM_TEXTURE_FORMAT_R5G6B5, 640, 480, 0);
		sceGxmTextureInitLinear(&gxm_tex_storage, soft_fb[1], SCE_GXM_TEXTURE_FORMAT_R5G6B5, 640, 480, 0);
		old_gxm_texture = &gxm_tex_storage;
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		FullscreenTextureWidth = 640;
		FullscreenTextureHeight = 480;
		
		window = 0xDEADBEEF;
	}

	SetWindowSize(Width, Height, Width, Height);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glDepthRange(0.0, 1.0);

	glDisable(GL_CULL_FACE);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	InitOpenGL(firsttime);

	return 0;
}

int InitialiseWindowsSystem(HANDLE hInstance, int nCmdShow, int WinInitMode)
{
	return 0;
}

int ExitWindowsSystem()
{
	if (joy != NULL) {
		SDL_JoystickClose(joy);
	}

	load_ogl_functions(0);
	
	free(fb_pixels);
	fb_pixels = NULL;
	window = NULL;

	return 0;
}

static int GotPrintScn, HavePrintScn;

static int KeySymToKey(int keysym)
{
	switch(keysym) {
		case SDLK_ESCAPE:
			return KEY_ESCAPE;
			
		case SDLK_0:
			return KEY_0;
		case SDLK_1:
			return KEY_1;
		case SDLK_2:
			return KEY_2;
		case SDLK_3:
			return KEY_3;
		case SDLK_4:
			return KEY_4;
		case SDLK_5:
			return KEY_5;
		case SDLK_6:
			return KEY_6;
		case SDLK_7:
			return KEY_7;
		case SDLK_8:
			return KEY_8;
		case SDLK_9:
			return KEY_9;
		
		case SDLK_a:
			return KEY_A;
		case SDLK_b:
			return KEY_B;
		case SDLK_c:
			return KEY_C;
		case SDLK_d:
			return KEY_D;
		case SDLK_e:
			return KEY_E;
		case SDLK_f:
			return KEY_F;
		case SDLK_g:
			return KEY_G;
		case SDLK_h:
			return KEY_H;
		case SDLK_i:
			return KEY_I;
		case SDLK_j:
			return KEY_J;
		case SDLK_k:
			return KEY_K;
		case SDLK_l:
			return KEY_L;
		case SDLK_m:
			return KEY_M;
		case SDLK_n:
			return KEY_N;
		case SDLK_o:
			return KEY_O;
		case SDLK_p:
			return KEY_P;
		case SDLK_q:
			return KEY_Q;
		case SDLK_r:
			return KEY_R;
		case SDLK_s:
			return KEY_S;
		case SDLK_t:
			return KEY_T;
		case SDLK_u:
			return KEY_U;
		case SDLK_v:
			return KEY_V;
		case SDLK_w:
			return KEY_W;
		case SDLK_x:
			return KEY_X;
		case SDLK_y:
			return KEY_Y;
		case SDLK_z:
			return KEY_Z;
				
		case SDLK_LEFT:
			return KEY_LEFT;
		case SDLK_RIGHT:
			return KEY_RIGHT;
		case SDLK_UP:
			return KEY_UP;
		case SDLK_DOWN:
			return KEY_DOWN;		
		case SDLK_RETURN:
			return KEY_CR;
		case SDLK_TAB:
			return KEY_TAB;
		case SDLK_INSERT:
			return KEY_INS;
		case SDLK_DELETE:
			return KEY_DEL;
		case SDLK_END:
			return KEY_END;
		case SDLK_HOME:
			return KEY_HOME;
		case SDLK_PAGEUP:
			return KEY_PAGEUP;
		case SDLK_PAGEDOWN:
			return KEY_PAGEDOWN;
		case SDLK_BACKSPACE:
			return KEY_BACKSPACE;
		case SDLK_COMMA:
			return KEY_COMMA;
		case SDLK_PERIOD:
			return KEY_FSTOP;
		case SDLK_SPACE:
			return KEY_SPACE;
			
		case SDLK_LSHIFT:
			return KEY_LEFTSHIFT;
		case SDLK_RSHIFT:
			return KEY_RIGHTSHIFT;
		case SDLK_LALT:
			return KEY_LEFTALT;
		case SDLK_RALT:
			return KEY_RIGHTALT;
		case SDLK_LCTRL:
			return KEY_LEFTCTRL;
		case SDLK_RCTRL:
			return KEY_RIGHTCTRL;

		case SDLK_CAPSLOCK:
			return KEY_CAPS;
		case SDLK_NUMLOCKCLEAR:
			return KEY_NUMLOCK;
		case SDLK_SCROLLLOCK:
			return KEY_SCROLLOK;
			
		case SDLK_KP_0:
			return KEY_NUMPAD0;
		case SDLK_KP_1:
			return KEY_NUMPAD1;
		case SDLK_KP_2:
			return KEY_NUMPAD2;
		case SDLK_KP_3:
			return KEY_NUMPAD3;
		case SDLK_KP_4:
			return KEY_NUMPAD4;
		case SDLK_KP_5:
			return KEY_NUMPAD5;
		case SDLK_KP_6:
			return KEY_NUMPAD6;
		case SDLK_KP_7:
			return KEY_NUMPAD7;
		case SDLK_KP_8:
			return KEY_NUMPAD8;
		case SDLK_KP_9:
			return KEY_NUMPAD9;
		case SDLK_KP_MINUS:
			return KEY_NUMPADSUB;
		case SDLK_KP_PLUS:
			return KEY_NUMPADADD;
		case SDLK_KP_PERIOD:
			return KEY_NUMPADDEL;
		case SDLK_KP_ENTER:
			return KEY_NUMPADENTER;
		case SDLK_KP_DIVIDE:
			return KEY_NUMPADDIVIDE;
		case SDLK_KP_MULTIPLY:
			return KEY_NUMPADMULTIPLY;
	
		case SDLK_LEFTBRACKET:
			return KEY_LBRACKET;
		case SDLK_RIGHTBRACKET:
			return KEY_RBRACKET;
		case SDLK_SEMICOLON:
			return KEY_SEMICOLON;
		case SDLK_QUOTE:
			return KEY_APOSTROPHE;
		case SDLK_BACKQUOTE:
			return KEY_GRAVE;
		case SDLK_BACKSLASH:
			return KEY_BACKSLASH;
		case SDLK_SLASH:
			return KEY_SLASH;
/*		case SDLK_
			return KEY_CAPITAL; */
		case SDLK_MINUS:
			return KEY_MINUS;
		case SDLK_EQUALS:
			return KEY_EQUALS;
		case SDLK_LGUI:
			return KEY_LWIN;
		case SDLK_RGUI:
			return KEY_RWIN;
/*		case SDLK_
			return KEY_APPS; */
		
		case SDLK_F1:
			return KEY_F1;
		case SDLK_F2:
			return KEY_F2;
		case SDLK_F3:
			return KEY_F3;
		case SDLK_F4:
			return KEY_F4;
		case SDLK_F5:
			return KEY_F5;
		case SDLK_F6:
			return KEY_F6;
		case SDLK_F7:
			return KEY_F7;
		case SDLK_F8:
			return KEY_F8;
		case SDLK_F9:
			return KEY_F9;
		case SDLK_F10:
			return KEY_F10;
		case SDLK_F11:
			return KEY_F11;
		case SDLK_F12:
			return KEY_F12;

/* finish foreign keys */

		default:
			return -1;
	}
}

static void handle_keypress(int key, int unicode, int press)
{	
	if (key == -1)
		return;

	if (press) {
		switch(key) {
			case KEY_CR:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR('\r');
				break;
			case KEY_BACKSPACE:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_BACK);
				break;
			case KEY_END:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_END);
				break;
			case KEY_HOME:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_HOME);
				break;
			case KEY_LEFT:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_LEFT);
				break;
			case KEY_UP:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_UP);
				break;
			case KEY_RIGHT:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_RIGHT);
				break;
			case KEY_DOWN:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_DOWN);
				break;
			case KEY_INS:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_INSERT);
				break;
			case KEY_DEL:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_DELETE);
				break;
			case KEY_TAB:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_TAB);
				break;
			default:
				break;
		}
	}
	
	if (press && !KeyboardInput[key]) {
		DebouncedKeyboardInput[key] = 1;
		DebouncedGotAnyKey = 1;
	}
	
	if (press)
		GotAnyKey = 1;
	KeyboardInput[key] = press;
}

void CheckForWindowsMessages()
{
	SDL_Event event;
	int x, y, buttons, wantmouse;
	
	GotAnyKey = 0;
	DebouncedGotAnyKey = 0;
	memset(DebouncedKeyboardInput, 0, sizeof(DebouncedKeyboardInput));
	
	wantmouse =	(SDL_GetRelativeMouseMode() == SDL_TRUE);

	// "keyboard" events that don't have an up event
	KeyboardInput[KEY_MOUSEWHEELUP] = 0;
	KeyboardInput[KEY_MOUSEWHEELDOWN] = 0;
	
	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_MOUSEBUTTONDOWN:
				break;
			case SDL_MOUSEBUTTONUP:
				break;
			case SDL_MOUSEWHEEL:
				if (wantmouse) {
					if (event.wheel.y < 0) {
						handle_keypress(KEY_MOUSEWHEELDOWN, 0, 1);
					} else if (event.wheel.y > 0) {
						handle_keypress(KEY_MOUSEWHEELUP, 0, 1);
					}
				}
				break;
			case SDL_TEXTINPUT: {
					int unicode = event.text.text[0]; //TODO convert to utf-32
					if (unicode && !(unicode & 0xFF80)) {
						RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR(unicode);
						KeyboardEntryQueue_Add(unicode);
					}
				}
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_PRINTSCREEN) {
					if (HavePrintScn == 0)
						GotPrintScn = 1;
					HavePrintScn = 1;
				} else {
					handle_keypress(KeySymToKey(event.key.keysym.sym), 0, 1);
				}
				break;
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_PRINTSCREEN) {
					GotPrintScn = 0;
					HavePrintScn = 0;
				} else {
					handle_keypress(KeySymToKey(event.key.keysym.sym), 0, 0);
				}
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_FOCUS_LOST:
						// disable mouse grab?
						break;
					case SDL_WINDOWEVENT_RESIZED:
						WindowWidth = event.window.data1;
						WindowHeight = event.window.data2;
						if (RenderingMode == RENDERING_MODE_SOFTWARE) {
							SetWindowSize(WindowWidth, WindowHeight, 640, 480);
						} else {
							SetWindowSize(WindowWidth, WindowHeight, WindowWidth, WindowHeight);
						}
						if (glViewport != NULL) {
							glViewport(0, 0, WindowWidth, WindowHeight);
						}
						break;
				}
				break;
			case SDL_QUIT:
				AvP.MainLoopRunning = 0; /* TODO */
				exit(0); //TODO
				break;
		}
	}
	
	buttons = SDL_GetRelativeMouseState(&x, &y);
	
	if (wantmouse) {
		if (buttons & SDL_BUTTON(1))
			handle_keypress(KEY_LMOUSE, 0, 1);
		else
			handle_keypress(KEY_LMOUSE, 0, 0);
		if (buttons & SDL_BUTTON(2))
			handle_keypress(KEY_MMOUSE, 0, 1);
		else
			handle_keypress(KEY_MMOUSE, 0, 0);
		if (buttons & SDL_BUTTON(3))
			handle_keypress(KEY_RMOUSE, 0, 1);
		else
			handle_keypress(KEY_RMOUSE, 0, 0);

		MouseVelX = DIV_FIXED(x, NormalFrameTime);
		MouseVelY = DIV_FIXED(y, NormalFrameTime);
	} else {
		KeyboardInput[KEY_LMOUSE] = 0;
		KeyboardInput[KEY_MMOUSE] = 0;
		KeyboardInput[KEY_RMOUSE] = 0;
		MouseVelX = 0;
		MouseVelY = 0;
	}

	if (GotJoystick) {
		int numbuttons;
		
		SDL_JoystickUpdate();
		
		numbuttons = 16;
		
		int button_mapping[] = { 1, 2, 0, 3, 0xff, 0xff, 4, 5, 0xff, 0xff, 11, 10, 7, 8, 9, 6 }; 
		
		for (x = 0; x < numbuttons; x++) {
			if (SDL_JoystickGetButton(joy, button_mapping[x])) {
				GotAnyKey = 1;
				if(KEY_JOYSTICK_BUTTON_1+x == KEY_JOYSTICK_BUTTON_12)
				{
					if (!KeyboardInput[KEY_CR]) {
						KeyboardInput[KEY_CR] = 1;
						DebouncedKeyboardInput[KEY_CR] = 1;
						DebouncedGotAnyKey = 1;
					}
					continue;
				}
				else if(KEY_JOYSTICK_BUTTON_1+x == KEY_JOYSTICK_BUTTON_11)
				{
					if (!KeyboardInput[KEY_ESCAPE]) {
						KeyboardInput[KEY_ESCAPE] = 1;
						DebouncedKeyboardInput[KEY_ESCAPE] = 1;
						DebouncedGotAnyKey = 1;
					}
					continue;
				}
				
				if (!KeyboardInput[KEY_JOYSTICK_BUTTON_1+x]) {
					KeyboardInput[KEY_JOYSTICK_BUTTON_1+x] = 1;
					DebouncedKeyboardInput[KEY_JOYSTICK_BUTTON_1+x] = 1;
					DebouncedGotAnyKey = 1;
				}
			} else {
				if(KEY_JOYSTICK_BUTTON_1+x == KEY_JOYSTICK_BUTTON_12)
				{
					KeyboardInput[KEY_CR] = 0;
				}
				else if(KEY_JOYSTICK_BUTTON_1+x == KEY_JOYSTICK_BUTTON_11)
				{
					KeyboardInput[KEY_ESCAPE] = 0;
				}
				KeyboardInput[KEY_JOYSTICK_BUTTON_1+x] = 0;
			}	
		}

		if(main_loop_state != 5 || InGameMenusAreRunning()) { /* Not Ingame */
			#ifndef JOYSTICK_DEAD_ZONE
			#define JOYSTICK_DEAD_ZONE 6000
			#endif
			int axes = SDL_JoystickNumAxes(joy);
			int xPos = 0, yPos = 0;
			
			if (axes > 0) {
				xPos = SDL_JoystickGetAxis(joy, 0) + 32768;
			}

			if (axes > 1) {
				yPos = SDL_JoystickGetAxis(joy, 1) + 32768;
			}
			
			int yAxis = (32768-yPos)*2;
			if(yAxis>JOYSTICK_DEAD_ZONE)
			{
				KeyboardInput[KEY_UP] = 1;
				DebouncedKeyboardInput[KEY_UP] = 1;
			}	
			else if(yAxis<-JOYSTICK_DEAD_ZONE)
			{
				KeyboardInput[KEY_DOWN] = 1;
				DebouncedKeyboardInput[KEY_DOWN] = 1;
			}
			else
			{
				KeyboardInput[KEY_UP] = 0;
				KeyboardInput[KEY_DOWN] = 0;
			}

			int xAxis = (xPos-32768)*2;
			if(xAxis>JOYSTICK_DEAD_ZONE)
			{
				KeyboardInput[KEY_RIGHT] = 1;
				DebouncedKeyboardInput[KEY_RIGHT] = 1;
			}	
			else if(xAxis<-JOYSTICK_DEAD_ZONE)
			{
				KeyboardInput[KEY_LEFT] = 1;
				DebouncedKeyboardInput[KEY_LEFT] = 1;
			}
			else
			{
				KeyboardInput[KEY_LEFT] = 0;
				KeyboardInput[KEY_RIGHT] = 0;
			}
		}
	}

	// a second reset of relative mouse state because
	// enabling relative mouse mode moves the mouse
	SDL_GetRelativeMouseState(NULL, NULL);

	if (GotPrintScn) {
		GotPrintScn = 0;
		
		ScreenShot();
	}
}
  
void InGameFlipBuffers()
{
	glViewport(0, 0, 960, 544);
	
	vglSwapBuffers(GL_FALSE);
	gVertexBuffer = gVertexBufferPtr;
	gIndices = gIndicesPtr;
}

int soft_fb_idx = 0;

void FlipBuffers()
{
	glViewport(0, 0, 960, 544);
	glBindTexture(GL_TEXTURE_2D, FullscreenTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	
	GLfloat x0;
	GLfloat x1;
	GLfloat y0;
	GLfloat y1;
	
	// figure out the best way to fit the 640x480 virtual window
	GLfloat a = DrawableHeight * 640.0f / 480.0f;
	GLfloat b = DrawableWidth * 480.0f / 640.0f;

	if (a <= DrawableWidth) {
		// a x DrawableHeight window
		y0 = -1.0f;
		y1 =  1.0f;

		x1 = 1.0 - (DrawableWidth - a) / DrawableWidth;
		x0 = -x1;
	} else {
		// DrawableWidth x b window
		x0 = -1.0f;
		x1 =  1.0f;

		y1 = 1.0 - (DrawableHeight - b) / DrawableHeight;
		y0 = -y1;
	}

	GLfloat s0 = 0.0f;
	GLfloat s1 = 1.0f;
	GLfloat t0 = 0.0f;
	GLfloat t1 = 1.0f;
	
	gIndices[0] = 0;
	gIndices[1] = 1;
	gIndices[2] = 2;
	gIndices[3] = 0;
	gIndices[4] = 2;
	gIndices[5] = 3;
	
	gVertexBuffer[0] = x0;
	gVertexBuffer[1] = y0;
	gVertexBuffer[2] =-1.0f;
	gVertexBuffer[3] = 1.0f;
	gVertexBuffer[4] = s0;
	gVertexBuffer[5] = t1;
	
	gVertexBuffer[8]  = x1;
	gVertexBuffer[9]  = y0;
	gVertexBuffer[10] =-1.0f;
	gVertexBuffer[11] = 1.0f;
	gVertexBuffer[12] = s1;
	gVertexBuffer[13] = t1;
	
	gVertexBuffer[16] = x1;
	gVertexBuffer[17] = y1;
	gVertexBuffer[18] =-1.0f;
	gVertexBuffer[19] = 1.0f;
	gVertexBuffer[20] = s1;
	gVertexBuffer[21] = t0;
	
	gVertexBuffer[24] = x0;
	gVertexBuffer[25] = y1;
	gVertexBuffer[26] =-1.0f;
	gVertexBuffer[27] = 1.0f;
	gVertexBuffer[28] = s0;
	gVertexBuffer[29] = t0;
	
	SelectProgram(AVP_SHADER_PROGRAM_NO_COLOR_NO_DISCARD);
	
	vglIndexPointerMapped(gIndices);
	vglVertexAttribPointerMapped(0, gVertexBuffer);
	
	vglDrawObjects(GL_TRIANGLES, 6, GL_FALSE);
	
	InGameFlipBuffers();
	
	glFinish();
	SceGxmTexture *temp = curr_gxm_texture;
	curr_gxm_texture = old_gxm_texture;
	old_gxm_texture = temp;
	soft_fb_idx = (soft_fb_idx + 1) % 2;
	fb_pixels = soft_fb[soft_fb_idx];
	memset(fb_pixels, 0, 640*480*2);
}

char *AvpCDPath = 0;

#if !defined(_MSC_VER)
static const struct option getopt_long_options[] = {
{ "help",	0,	NULL,	'h' },
{ "version",	0,	NULL,	'v' },
{ "fullscreen",	0,	NULL,	'f' },
{ "windowed",	0,	NULL,	'w' },
{ "nosound",	0,	NULL,	's' },
{ "nocdrom",	0,	NULL,	'c' },
{ "nojoy",	0,	NULL,	'j' },
{ "debug",	0,	NULL,	'd' },
{ "withgl",	1,	NULL,	'g' },
/*
{ "loadrifs",	1,	NULL,	'l' },
{ "server",	0,	someval,	1 },
{ "client",	1,	someval,	2 },
*/
{ NULL,		0,	NULL,	0 },
};
#endif

static const char *usage_string =
"Aliens vs Predator Linux - http://www.icculus.org/avp/\n"
"Based on Rebellion Developments AvP Gold source\n"
"      [-h | --help]           Display this help message\n"
"      [-v | --version]        Display the game version\n"
"      [-f | --fullscreen]     Run the game fullscreen\n"
"      [-w | --windowed]       Run the game in a window\n"
"      [-s | --nosound]        Do not access the soundcard\n"
"      [-c | --nocdrom]        Do not access the CD-ROM\n"
"      [-j | --nojoy]          Do not access the joystick\n"
"      [-g | --withgl] [x]     Use [x] instead of /usr/lib/libGL.so.1 for OpenGL\n"
;

static int menusActive = 0;
static int thisLevelHasBeenCompleted = 0;

int main(int argc, char *argv[])
{			
#if !defined(_MSC_VER)
	int c;
	
	opterr = 0;
	while ((c = getopt_long(argc, argv, "hvfwscdg:", getopt_long_options, NULL)) != -1) {
		switch(c) {
			case 'h':
				printf("%s", usage_string);
				exit(EXIT_SUCCESS);
			case 'v':
				printf("%s", AvPVersionString);
				exit(EXIT_SUCCESS);
			case 'f':
				WantFullscreen = 1;
				break;
			case 'w':
				WantFullscreen = 0;
				break;
			case 's':
				WantSound = 0;
				break;
			case 'c':
				WantCDRom = 0;
				break;
			case 'j':
				WantJoystick = 0;
				break;			
			case 'd': {
				extern int DebuggingCommandsActive;
				DebuggingCommandsActive = 1;
				}
				break;
			case 'g':
				opengl_library = optarg;
				break;
			default:
				printf("%s", usage_string);
				exit(EXIT_FAILURE);	
		}
	}
#endif

	InitGameDirectories(argv[0]);
	
	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	scePowerSetGpuClockFrequency(222);
	scePowerSetGpuXbarClockFrequency(166);
	vglInitExtended(0, 960, 544, 0x100000, SCE_GXM_MULTISAMPLE_4X);
		
	gVertexBufferPtr = (float*)malloc(0x1800000);
	gIndicesPtr = (uint16_t*)malloc(0x600000);
	gVertexBuffer = gVertexBufferPtr;
	gIndices = gIndicesPtr;
	
	if (InitSDL() == -1) {
		printf("Could not find a sutable resolution!\n");
		printf("At least 512x384 is needed.  Does OpenGL work?\n");
		exit(EXIT_FAILURE);
	}
		
	LoadCDTrackList();
	
	SetFastRandom();

#if MARINE_DEMO
	ffInit("fastfile/mffinfo.txt","fastfile/");
#elif ALIEN_DEMO
	ffInit("alienfastfile/ffinfo.txt","alienfastfile/");
#else
	ffInit("fastfile/ffinfo.txt","fastfile/");
#endif
	InitGame();

	WindowWidth = VideoModeList[CurrentVideoMode].w;
	WindowHeight = VideoModeList[CurrentVideoMode].h;
	SetOGLVideoMode(0, 0);
	SetSoftVideoMode(640, 480, 16);
	
	InitialVideoMode();

	/* Env_List can probably be removed */
	Env_List[0]->main = LevelName;

	InitialiseSystem();
	InitialiseRenderer();
	
	LoadKeyConfiguration();
	
	SoundSys_Start();
	//BinkSys_Init();

	if (WantCDRom) CDDA_Start();
	
	InitTextStrings();
	
	BuildMultiplayerLevelNameArray();
	
	ChangeDirectDrawObject();
	AvP.LevelCompleted = 0;
	LoadSounds("PLAYER");

	/* is this still neccessary? */
	AvP.CurrentEnv = AvP.StartingEnv = 0;

#if ALIEN_DEMO
	AvP.PlayerType = I_Alien;
	SetLevelToLoad(AVP_ENVIRONMENT_INVASION_A);
#elif PREDATOR_DEMO
	AvP.PlayerType = I_Predator;
	SetLevelToLoad(AVP_ENVIRONMENT_INVASION_P);
#elif MARINE_DEMO
	AvP.PlayerType = I_Marine;
	SetLevelToLoad(AVP_ENVIRONMENT_INVASION);
#endif

	main_loop_state = 1;
	
	while(1)
		main_loop();
		
	return 0;
}

int AvP_MainMenus_Init(void);
int AvP_MainMenus_Update(void);
int AvP_MainMenus_Deinit(void);

static int MainGame_Init(void);
static int MainGame_Update(void);
static int MainGame_Deinit(void);

static void main_loop(void) {
	switch (main_loop_state) {
		case 0:
			return;

		case -1:
			printf("You can't exit the game!\n");
			main_loop_state = 0;
			return;

		case -999:
			printf("Unimplemented!\n");
			main_loop_state = 0;
			return;	

		case 1:
			AvP_MainMenus_Init();
			main_loop_state = 2;
			// fallthrough
		case 2: {
			int cont = AvP_MainMenus_Update();
			if (cont != 0) {
				return;
			}
		}
			// fallthrough

		case 3: {
			int cont = AvP_MainMenus_Deinit();
			if (cont != 0) {
				main_loop_state = 4;
				return;
			}
		}
			main_loop_state = -1;
			return;
		
		case 4: {
			MainGame_Init();
			main_loop_state = 5;
			// fallthrough
		}
		
		case 5: {
			int cont = MainGame_Update();
			if (cont != 0) {
				return;
			}
			// fallthrough
		}
		
		case 6: {
			MainGame_Deinit();
			main_loop_state = 1;
		}
		
	}
}

static int MainGame_Init(void) {
	menusActive = 0;
	thisLevelHasBeenCompleted = 0;
	
	/* turn off any special effects */
	d3d_light_ctrl.ctrl = LCCM_NORMAL;

	SetOGLVideoMode(0, 0);

	InitialiseGammaSettings(RequestedGammaSetting);
	
	start_of_loaded_shapes = load_precompiled_shapes();
	
	InitCharacter();
	
	LoadRifFile(); /* sets up a map */
	
	AssignAllSBNames();
	
	StartGame();
	
	ffcloseall();
	
	AvP.MainLoopRunning = 1;
	
	ScanImagesForFMVs();
	
	ResetFrameCounter();

	Game_Has_Loaded();
	
	ResetFrameCounter();
	
	if(AvP.Network!=I_No_Network)
	{
		/*Need to choose a starting position for the player , but first we must look
		through the network messages to find out which generator spots are currently clear*/
		netGameData.myGameState = NGS_Playing;
		MinimalNetCollectMessages();
		TeleportNetPlayerToAStartingPosition(Player->ObStrategyBlock,1);
	}

	IngameKeyboardInput_ClearBuffer();

	return 0;
}

static int MainGame_Update(void) {	
	if(AvP.MainLoopRunning) {
		CheckForWindowsMessages();
		
		switch(AvP.GameMode) {
		case I_GM_Playing:
			if ((!menusActive || (AvP.Network!=I_No_Network && !netGameData.skirmishMode)) && !AvP.LevelCompleted) {
				/* TODO: print some debugging stuff */
				
				DoAllShapeAnimations();
				
				UpdateGame();
				
				AvpShowViews();
				
				MaintainHUD();
				
				CheckCDAndChooseTrackIfNeeded();
				
				if(InGameMenusAreRunning() && ( (AvP.Network!=I_No_Network && netGameData.skirmishMode) || (AvP.Network==I_No_Network)) ) {
					SoundSys_StopAll();
				}
			} else {
				ReadUserInput();
				
				SoundSys_Management();
				
				FlushD3DZBuffer();
				
				ThisFramesRenderingHasBegun();
			}

			menusActive = AvP_InGameMenus();
			if (AvP.RestartLevel) menusActive=0;
			
			if (AvP.LevelCompleted) {
				SoundSys_FadeOutFast();
				DoCompletedLevelStatisticsScreen();
				thisLevelHasBeenCompleted = 1;
			}

			ThisFramesRenderingHasFinished();

			InGameFlipBuffers();
			
			FrameCounterHandler();
			{
				PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
				
				if (!menusActive && playerStatusPtr->IsAlive && !AvP.LevelCompleted) {
					DealWithElapsedTime();
				}
			}
			break;
			
		case I_GM_Menus:
			AvP.GameMode = I_GM_Playing;
			break;
		default:
			printf("AvP.MainLoopRunning: gamemode = %d\n", AvP.GameMode);
			exit(EXIT_FAILURE);
		}
		
		if (AvP.RestartLevel) {
			AvP.RestartLevel = 0;
			AvP.LevelCompleted = 0;

			FixCheatModesInUserProfile(UserProfilePtr);

			RestartLevel();
		}
	}

	return AvP.MainLoopRunning;
}

static int MainGame_Deinit(void) {
	
	AvP.LevelCompleted = thisLevelHasBeenCompleted;

	FixCheatModesInUserProfile(UserProfilePtr);

	ReleaseAllFMVTextures();

	CONSBIND_WriteKeyBindingsToConfigFile();
	
	DeInitialisePlayer();
	
	DeallocatePlayersMirrorImage();
	
	KillHUD();
	
	Destroy_CurrentEnvironment();
	
	DeallocateAllImages();
	
	EndNPCs();
	
	ExitGame();

	//BinkSys_Release();
	
	SoundSys_StopAll();
	
	SoundSys_ResetFadeLevel();
	
	CDDA_Stop();
	
	if (AvP.Network != I_No_Network) {
		EndAVPNetGame();
	}
	
	ClearMemoryPool();

/* go back to menu mode */
#if !(ALIEN_DEMO|PREDATOR_DEMO|MARINE_DEMO)
	SetSoftVideoMode(640, 480, 16);
#endif
	
	return 0;
}
