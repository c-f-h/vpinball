#include "StdAfx.h"
#include "freeimage.h"
#include "Texture.h"

PinDirectDraw::PinDirectDraw()
{
   int tmp = 1;
   HRESULT hr = GetRegInt("Player", "HardwareRender", &tmp);
   m_fHardwareAccel = (tmp != 0);
}

PinDirectDraw::~PinDirectDraw()
{
}

typedef int(CALLBACK *DDCreateFunction)(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);


BaseTexture* PinDirectDraw::CreateOffscreenWithCustomTransparency(const int width, const int height, const int color)
{
#ifdef VPINBALL_DX9
    return NULL;
#else
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);

	/*if (width < 1 || height < 1)
	{
	return NULL;
	}*/

	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_CKSRCBLT;
	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = color;//0xffffff;
	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = color;//0xffffff;
	ddsd.dwWidth        = width < 1 ? 1 : width;   // This can happen if an object is completely off screen.  Since that's
	ddsd.dwHeight       = height < 1 ? 1 : height; // rare, it's easier just to create a tiny surface to handle it.
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;// | DDSCAPS_3DDEVICE;

	// Check if we are rendering in hardware.
	if (m_fHardwareAccel)
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY; // slower(?): | DDSCAPS_LOCALVIDMEM;
	else
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

retry0:
	BaseTexture* pdds;
	HRESULT hr;
	if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &pdds, NULL ) ) )
	{
		if((ddsd.ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) == 0) {
			ddsd.ddsCaps.dwCaps |= DDSCAPS_NONLOCALVIDMEM;
			goto retry0;
		}
		ShowError("Could not create offscreen surface.");
		return NULL;
	}

	return pdds;
#endif
}

