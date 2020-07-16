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
	log2file("PlayBinkedFMV(%s)\n", filenamePtr);
*/
//}

//void StartMenuBackgroundBink()
//{
/*
	log2file("StartMenuBackgroundBink()\n");
*/
//}

//int PlayMenuBackgroundBink()
//{
/*
	log2file("PlayMenuBackgroundBink()\n");
*/	
//	return 0;
//}

//void EndMenuBackgroundBink()
//{
/*
	log2file("EndMenuBackgroundBink()\n");
*/
//}

/* alt_tab.cpp */
void ATIncludeSurface(void * pSurface, void * hBackup)
{
	log2file("ATIncludeSurface(%p, %p)\n", pSurface, hBackup);
}

void ATRemoveSurface(void * pSurface)
{
	log2file("ATRemoveSurface(%p)\n", pSurface);
}

void ATRemoveTexture(void * pTexture)
{
	log2file("ATRemoveTexture(%p)\n", pTexture);
}


/* d3_func.cpp */
int GetTextureHandle(IMAGEHEADER *imageHeaderPtr)
{
/*
	log2file("GetTextureHandle(%p)\n", imageHeaderPtr);
*/	
	return 1;
}

void ReleaseDirect3DNotDDOrImages()
{
/*
	log2file("ReleaseDirect3DNotDDOrImages()\n");
*/	
}

void ReleaseDirect3DNotDD()
{
/*
	log2file("ReleaseDirect3DNotDD()\n");
*/	
}

void ReleaseDirect3D()
{
/*
	log2file("ReleaseDirect3D()\n");
*/
}

void ReloadImageIntoD3DImmediateSurface(IMAGEHEADER* iheader)
{
	log2file("ReloadImageIntoD3DImmediateSurface(%p)\n", iheader);
}


/* d3d_render.cpp */
int NumberOfLandscapePolygons;
int FMVParticleColour;
int WireFrameMode;

void InitDrawTest()
{
/*
	log2file("InitDrawTest()\n");
*/
}

void CheckWireFrameMode(int shouldBeOn)
{
	if (shouldBeOn)
		log2file("CheckWireFrameMode(%d)\n", shouldBeOn);
}


/* ddplat.cpp */
void MinimizeAllDDGraphics()
{
/*
	log2file("MinimizeAllDDGraphics()\n");
*/	
}

        
/* dd_func.cpp */
long BackBufferPitch;
int VideoModeColourDepth;

void BlitWin95Char(int x, int y, unsigned char toprint)
{
	log2file("BlitWin95Char(%d, %d, %d)\n", x, y, toprint);
}

void LockSurfaceAndGetBufferPointer()
{
	log2file("LockSurfaceAndGetBufferPointer()\n");
}

void finiObjectsExceptDD()
{
/*
	log2file("finiObjectsExceptDD()\n");
*/	
}

void finiObjects()
{
/*
	log2file("finiObjects()\n");
*/	
}

void UnlockSurface()
{
	log2file("UnlockSurface()\n");
}


BOOL ChangeDirectDrawObject()
{
/*
	log2file("ChangeDirectDrawObject()\n");
*/	
	return FALSE;
}

int SelectDirectDrawObject(void *pGUID)
{
/*
	log2file("SelectDirectDrawObject(%p)\n", pGUID);
*/
	return 0;
}

void GenerateDirectDrawSurface()
{
/*
	log2file("GenerateDirectDrawSurface()\n");
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
