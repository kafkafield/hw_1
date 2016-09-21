// app.cpp : Contains generic Windows functions and calls AppInit, AppIdle, and AppTerm
//

#include "stdafx.h"

// background image
#define BKGBMP_FILENAME "background.bmp"
DWORD *pBkgBmp = NULL;
DWORD BkgBmpWidth, BkgBmpHeight, BkgBmpPitch;

// foreground image
#define FGBMP_FILENAME "foreground.bmp"
DWORD *pFgBmp = NULL;
DWORD FgBmpWidth, FgBmpHeight, FgBmpPitch;
POINT FgPos = { 0, 0};
POINT FgDir = {+2, 0};
DWORD *pRotatedImage = NULL;	// same size as foreground
float Angle = 0.0f, AngleDir = -0.04f;

#define PI 3.1415926
#define PI2 6.283185
#define PIm2 1.570796

//---------------------------------------------------------------------------
void UpdatePositionAndRotation (void)
{
	Angle += AngleDir;

	if (Angle > PI)
		Angle -= PI2;
	if (Angle < -PI)
		Angle += PI2;

	FgPos.x += FgDir.x;

	if (FgPos.x < 0)
	{
		FgPos.x = 0;
		FgDir.x = -FgDir.x;
		AngleDir *= -1;
	} else if (FgPos.x + FgBmpWidth >= (int)BkgBmpWidth)
	{
		FgPos.x = BkgBmpWidth - FgBmpWidth;
		FgDir.x = -FgDir.x;
		AngleDir *= -1;
	}
}

inline float sin_simple(float a, char c)
{
	if (c == 'c')
		a += PIm2;
	if (a > PI)
		a -= PI2;
	if (a < 0)
	{
		if (a >= -0.6f)
			return a;
		else
		{
			return 1.27323954 * a + 0.405284735 * a * a;
		}
	}
	else
	{
		if (a <= 0.6f)
			return a;
		else
		{
			return 1.27323954 * a - 0.405284735 * a * a;
		}
	}
}

// --------------------------------------------------------------------------
void Rotate (DWORD *pDst, long DPitch, DWORD *pSrc, long SPitch, long width, long  height, float angle)
{
	DPitch /= sizeof(DWORD);
	SPitch /= sizeof(DWORD);

	int halfHeight = height >> 1;
	int halfWidth = width >> 1;

	for (int y = -halfHeight; y<halfHeight; ++y) //++y
	{
		for (int x = -halfWidth; x<halfWidth; ++x)
		{
			//float co = cos(angle);
			//float si = sin(angle);
			//float fSrcX = (float)(width / 2.0 + x*co - y*si);
			//float fSrcY = (float)(height / 2.0 + x*si + y*co);
			float co = sin_simple(angle, 'c');
			float si = sin_simple(angle, 's');

			//float fSrcX = (float)(width / 2.0 + x*co - y*si);
			//float fSrcY = (float)(height / 2.0 + x*si + y*co);

			float ub = x + y;
			float uc = y - x;
			float vc = co + si;
			float uavc = x * vc;
			float fSrcX = uc * co + uavc + halfWidth;
			float fSrcY = uavc - ub * si + halfHeight;

			long dwSrcX = (long)fSrcX;
			long dwSrcY = (long)fSrcY;
			
			if (dwSrcX > 0 && dwSrcY > 0 && dwSrcX < width-1 && dwSrcY < height-1)
			{
				DWORD *pTopLeft, *pTopRight, *pBottomLeft, *pBottomRight;
				pTopLeft		= pSrc + dwSrcY * SPitch + dwSrcX;
				pTopRight		= pTopLeft + 1;
				pBottomLeft		= pTopLeft + SPitch;
				pBottomRight	= pBottomLeft + 1;

				float fx = fSrcX - dwSrcX;
				float fy = fSrcY - dwSrcY;

				float fxfy = fx * fy;
				//byte c1 = (1 - fx - fy + fxfy) ;
				//byte c2 = fx - fxfy;
				//byte c3 = fy - fxfy;
				float c1 = (1 - fx - fy + fxfy);
				float c2 = fx - fxfy;
				float c3 = fy - fxfy;

				// alpha interpolation
				byte TopLeftAlpha     = (*pTopLeft)    >> 24;
				byte TopRightAlpha    = (*pTopRight)   >> 24;
				byte BottomLeftAlpha  = (*pBottomLeft) >> 24;
				byte BottomRightAlpha = (*pBottomRight)>> 24;
				byte alpha_value = TopLeftAlpha	* c1 +
					//float alphaFP = TopLeftAlpha	* c1 +
					TopRightAlpha	* c2 +
					BottomLeftAlpha	* c3 +
					BottomRightAlpha* fxfy +0.5;
				//DWORD  alpha_value = (DWORD)(alphaFP + 0.5);
				//DWORD  alpha_value = alphaFP;

				// red interpolation
				byte TopLeftRed     = ((*pTopLeft)    >> 16) & 0xff;
				byte TopRightRed    = ((*pTopRight)   >> 16) & 0xff;
				byte BottomLeftRed  = ((*pBottomLeft) >> 16) & 0xff;
				byte BottomRightRed = ((*pBottomRight)>> 16) & 0xff;
				byte red_value = TopLeftRed		* c1 +
					//float redFP = TopLeftRed		* c1 +
					TopRightRed		* c2 +
					BottomLeftRed	* c3 +
					BottomRightRed	* fxfy +0.5;
				//DWORD  red_value = (DWORD)(redFP + 0.5);
				
				// green interpolation
				byte TopLeftGreen     = ((*pTopLeft)    >> 8) & 0xff;
				byte TopRightGreen    = ((*pTopRight)   >> 8) & 0xff;
				byte BottomLeftGreen  = ((*pBottomLeft) >> 8) & 0xff;
				byte BottomRightGreen = ((*pBottomRight)>> 8) & 0xff;
				byte green_value = TopLeftGreen	  * c1 +
					//float greenFP = TopLeftGreen	  * c1 +
					TopRightGreen	  * c2 +
					BottomLeftGreen  * c3 +
					BottomRightGreen * fxfy +0.5;
				//DWORD  green_value = (DWORD)(greenFP + 0.5);
				//DWORD  green_value = greenFP;

				// blue interpolation
				byte TopLeftBlue     = *pTopLeft & 0xff;
				byte TopRightBlue    = *pTopRight & 0xff;
				byte BottomLeftBlue  = *pBottomLeft & 0xff;
				byte BottomRightBlue = *pBottomRight & 0xff;
				byte blue_value = TopLeftBlue		* c1 +
					//float blueFP = TopLeftBlue		* c1 +
					TopRightBlue	* c2 +
					BottomLeftBlue	* c3 +
					BottomRightBlue	* fxfy +0.5;
				//DWORD  blue_value = (DWORD)(blueFP + 0.5);
				//DWORD  blue_value = blueFP;

				pDst[x+halfWidth] = (alpha_value << 24) | (red_value << 16) | 
								  (green_value << 8)  | (blue_value);
				//byte * temp = (byte*)&pDst[x + halfWidth];
				//*(temp + 3) = alpha_value;
				//*(temp + 2) = red_value;
				//*(temp + 1) = green_value;
				//*(temp + 0) = blue_value;
			} else {
				pDst[x + halfWidth] = 0;
			}
		}
		pDst += DPitch;
	}
}
// --------------------------------------------------------------------------
void AlphaBlend32to32(DWORD *pDst, long dPitch,
				DWORD *pSrc, long sPitch,
				DWORD width, DWORD height)
{
	for (DWORD row=0; row < height; row++)
	{
		for (DWORD col=0; col < width; col++)
		{
			DWORD sColor = pSrc[col];
			DWORD sAlpha = sColor >> 24;
			if (sAlpha != 0)
			{
				if (sAlpha == 255)
					pDst[col] = sColor;
				else
				{
					DWORD dColor = pDst[col];
					DWORD dAlpha = 255 - sAlpha;

					DWORD sBlue = sColor & 0xff;
					DWORD dBlue = dColor & 0xff;
					DWORD Blue  = (sBlue * sAlpha + dBlue * dAlpha) / 256;

					DWORD sGreen = (sColor >> 8) & 0xff;
					DWORD dGreen = (dColor >> 8) & 0xff;
					DWORD Green  = (sGreen * sAlpha + dGreen * dAlpha) / 256;

					DWORD sRed = (sColor >> 16) & 0xff;
					DWORD dRed = (sColor >> 16) & 0xff;
					DWORD Red  = (sRed * sAlpha + dRed * dAlpha) / 256;

					pDst[col] = (Red << 16) + (Green << 8) + Blue;
				}
			}
		}
		pSrc += sPitch / 4;
		pDst += dPitch / 4;
	}
}

// --------------------------------------------------------------------------
void MemCopyRect(BYTE *pDst, long dPitch, BYTE *pSrc, long sPitch, 
		DWORD width, DWORD height)
{
	for (DWORD row=0; row < height; row ++)
	{
		CopyMemory(pDst, pSrc, width);
		pDst += dPitch;
		pSrc += sPitch;
	}
}

// ------------------------------------------------------------------------
int AppInit (HWND hWnd)
{
	int RetVal;

	// load the bitmap for the background
	BITMAPINFOHEADER BIH;
	RetVal = MyLoadBitmap32((void**)&pBkgBmp, &BIH, BKGBMP_FILENAME);
	if (RetVal != 0)
		return -1;
	BkgBmpWidth	 = BIH.biWidth;
	BkgBmpHeight = BIH.biHeight;
	BkgBmpPitch	 = BIH.biWidth*4;

	// adjust the window to match the size of the bitmap
	RECT ClientRect;
	SetRect(&ClientRect, 0, 0, BIH.biWidth, BIH.biHeight);
	AdjustWindowRect(&ClientRect, WS_CAPTION | WS_SYSMENU, FALSE);
	SetWindowPos(hWnd, 0, 0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top, SWP_NOMOVE | SWP_NOZORDER);

	// load the bitmap for the foreground
	RetVal = MyLoadBitmap32((void **)&pFgBmp, &BIH, FGBMP_FILENAME);
	if (RetVal != 0)
	{
		free (pBkgBmp);
		return -1;
	}
	FgBmpWidth  = BIH.biWidth;
	FgBmpHeight = BIH.biHeight;
	FgBmpPitch  = BIH.biWidth*4;

	// create the bitmap for the rotated foreground image
	pRotatedImage = (DWORD *)malloc(FgBmpWidth * FgBmpHeight * sizeof(DWORD));
	if (pRotatedImage == NULL)
	{
		free (pBkgBmp);
		return -1;
	}

	// place the ball vertically centered 
	FgPos.y = BkgBmpHeight / 2 - FgBmpHeight / 2;

	RetVal = dd_Init(hWnd, BkgBmpWidth, BkgBmpHeight);


	return RetVal;
}


// ------------------------------------------------------------------------
void AppIdle (HWND hWnd)
{
	BYTE *pBackBuf;
	DWORD BackBufWidth, BackBufHeight;
	long  BackBufPitch;

	DWORD StartTime, ElapTime;
	_asm {
		RDTSC
		mov StartTime, eax
	}

	UpdatePositionAndRotation();   // updates FgPos.x and Angle

	// rotates pFgBmp by Angle radians and places result in pRotatedImage
	Rotate(pRotatedImage, FgBmpPitch, pFgBmp, FgBmpPitch, FgBmpWidth, FgBmpHeight, Angle);

	// retrieve a write pointer to the off-screen surface from DirectDraw
	HRESULT dd_RetVal = dd_LockSurface(&pBackBuf, &BackBufWidth, &BackBufHeight, &BackBufPitch);
	if(dd_RetVal != DD_OK) 
		return;
	// erase the old image by copying the pBkgBmp to the off-screen surface
	MemCopyRect(pBackBuf, BackBufPitch, (BYTE*)pBkgBmp, BkgBmpPitch, BkgBmpWidth*4, BkgBmpHeight);

	// calculate the offset of the upper left pixel of the foreground image on the off-screen surface
	DWORD *pBallPosOnSurface = (DWORD*)(pBackBuf + FgPos.x*4 + FgPos.y*BackBufPitch);

	// blend the rotated foreground image onto the background image
	// note: the blend process reads from the destination
	AlphaBlend32to32((DWORD *)pBallPosOnSurface, BackBufPitch, pRotatedImage, FgBmpPitch, FgBmpWidth, FgBmpHeight);

	dd_UnlockSurface(pBackBuf);

	// copy the off-screen surface to the visible client window
	dd_BltBackToPrimaryFull(hWnd);

	_asm {
		RDTSC
		sub eax, StartTime
		mov ElapTime, eax
	}
	static DWORD MinElapsedTime = 0xffffffff;
	if (MinElapsedTime > ElapTime)
	{
		MinElapsedTime = ElapTime;
		char WinTitle[80];
		wsprintf(WinTitle, "%d ", MinElapsedTime);
		SetWindowText(hWnd, WinTitle);
	}
}


// ------------------------------------------------------------------------
void AppTerm (void)
{
	if (pBkgBmp != NULL) free (pBkgBmp);
	if (pFgBmp != NULL) free (pFgBmp);
	if (pRotatedImage != NULL) free (pRotatedImage);
	
	dd_Term();
}
