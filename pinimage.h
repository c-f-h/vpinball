#include "Texture.h"
#pragma once
#if !defined(AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_)
#define AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_

#ifdef RGB
#undef RGB
#endif
#define BGR(b,g,r) ((COLORREF)(((DWORD)(b)) | (((DWORD)(g))<<8) | (((DWORD)(r))<<16)))
#define RGB(r,g,b) ((COLORREF)(((DWORD)(r)) | (((DWORD)(g))<<8) | (((DWORD)(b))<<16)))

#define NOTRANSCOLOR  RGB(123,123,123)


class PinDirectDraw
{
public:
	PinDirectDraw();
	~PinDirectDraw();

	HRESULT InitDD();

	LPDIRECTDRAW7 m_pDD;

	BaseTexture* CreateTextureOffscreen(const int width, const int height, DWORD* texWidth = NULL, DWORD* texHeight = NULL);
	BaseTexture* CreateFromFile(char *szfile, int * const pwidth, int * const pheight, int& originalWidth, int& originalHeight);
	BaseTexture* CreateFromResource(const int id, int * const pwidth, int * const pheight);
	BaseTexture* CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight);
    BaseTexture* CreateOffscreenWithCustomTransparency(const int width, const int height, const int color);
    BaseTexture* CreateOffscreenPlain(const int width, const int height);   // this is only used by a few elements, maybe can be removed

	BOOL m_fHardwareAccel;
	BOOL m_fAlternateRender;
};

#endif // !defined(AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_)
