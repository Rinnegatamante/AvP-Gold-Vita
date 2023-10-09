#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fixer.h"

#include "3dc.h"
#include "platform.h"
#include "psndplat.h"
#include "module.h"
#include "stratdef.h"
#include "avp_userprofile.h"
#include "projfont.h"
#include "savegame.h"
#include "pldnet.h"
#include "kshape.h"
#include "d3d_hud.h"


/* winmain.c */
BOOL KeepMainRifFile = FALSE;
int HWAccel = 1;
int VideoModeNotAvailable=0;

/* bink.c */
//void PlayBinkedFMV(char *filenamePtr)
//{
/*
	printf("PlayBinkedFMV(%s)\n", filenamePtr);
*/
//}

//void StartMenuBackgroundBink()
//{
/*
	printf("StartMenuBackgroundBink()\n");
*/
//}

//int PlayMenuBackgroundBink()
//{
/*
	printf("PlayMenuBackgroundBink()\n");
*/	
//	return 0;
//}

//void EndMenuBackgroundBink()
//{
/*
	printf("EndMenuBackgroundBink()\n");
*/
//}

/* alt_tab.cpp */
void ATIncludeSurface(void * pSurface, void * hBackup)
{
	printf("ATIncludeSurface(%p, %p)\n", pSurface, hBackup);
}

void ATRemoveSurface(void * pSurface)
{
	printf("ATRemoveSurface(%p)\n", pSurface);
}

void ATRemoveTexture(void * pTexture)
{
	printf("ATRemoveTexture(%p)\n", pTexture);
}


/* d3_func.cpp */
int GetTextureHandle(IMAGEHEADER *imageHeaderPtr)
{
/*
	printf("GetTextureHandle(%p)\n", imageHeaderPtr);
*/	
	return 1;
}

void ReleaseDirect3DNotDDOrImages()
{
/*
	printf("ReleaseDirect3DNotDDOrImages()\n");
*/	
}

void ReleaseDirect3DNotDD()
{
/*
	printf("ReleaseDirect3DNotDD()\n");
*/	
}

void ReleaseDirect3D()
{
/*
	printf("ReleaseDirect3D()\n");
*/
}

void ReloadImageIntoD3DImmediateSurface(IMAGEHEADER* iheader)
{
	printf("ReloadImageIntoD3DImmediateSurface(%p)\n", iheader);
}


/* d3d_render.cpp */
int NumberOfLandscapePolygons;
int FMVParticleColour;
int WireFrameMode;

void InitDrawTest()
{
/*
	printf("InitDrawTest()\n");
*/
}

void CheckWireFrameMode(int shouldBeOn)
{
	if (shouldBeOn)
		printf("CheckWireFrameMode(%d)\n", shouldBeOn);
}


/* ddplat.cpp */
void MinimizeAllDDGraphics()
{
/*
	printf("MinimizeAllDDGraphics()\n");
*/	
}

        
/* dd_func.cpp */
long BackBufferPitch;
int VideoModeColourDepth;

void BlitWin95Char(int x, int y, unsigned char toprint)
{
	printf("BlitWin95Char(%d, %d, %d)\n", x, y, toprint);
}

void LockSurfaceAndGetBufferPointer()
{
	printf("LockSurfaceAndGetBufferPointer()\n");
}

void finiObjectsExceptDD()
{
/*
	printf("finiObjectsExceptDD()\n");
*/	
}

void finiObjects()
{
/*
	printf("finiObjects()\n");
*/	
}

void UnlockSurface()
{
	printf("UnlockSurface()\n");
}


BOOL ChangeDirectDrawObject()
{
/*
	printf("ChangeDirectDrawObject()\n");
*/	
	return FALSE;
}

int SelectDirectDrawObject(void *pGUID)
{
/*
	printf("SelectDirectDrawObject(%p)\n", pGUID);
*/
	return 0;
}

void GenerateDirectDrawSurface()
{
/*
	printf("GenerateDirectDrawSurface()\n");
*/	
}


/* dxlog.c */
void dx_str_log(char const * str, int line, char const * file)
{
	FILE *fp;

	fp = OpenGameFile("dx_error.log", FILEMODE_APPEND, FILETYPE_CONFIG);	
	if (fp == NULL)
		fp = stderr;
		
	fprintf(fp, "dx_str_log: %s/%d: %s\n", file, line, str);
	
	if (fp != stderr) fclose(fp);
}

void dx_strf_log(char const * fmt, ... )
{
	va_list ap;
	FILE *fp;
	
	fp = OpenGameFile("dx_error.log", FILEMODE_APPEND, FILETYPE_CONFIG);
	if (fp == NULL)
		fp = stderr;
        
        va_start(ap, fmt);
        fprintf(fp, "dx_strf_log: ");
	vfprintf(fp, fmt,ap);
	fprintf(fp, "\n");
        va_end(ap);
        
        if (fp != stderr) fclose(fp);
}

void dx_line_log(int line, char const * file)
{
	FILE *fp;
	
	fp = OpenGameFile("dx_error.log", FILEMODE_APPEND, FILETYPE_CONFIG);
	if (fp == NULL)
		fp = stderr;	
	
	fprintf(fp, "dx_line_log: %s/%d\n", file, line);
	
	if (fp != stderr) fclose(fp);
}
