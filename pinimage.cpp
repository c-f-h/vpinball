#include "StdAfx.h"
#include "freeimage.h"
#include "Texture.h"

PinDirectDraw::PinDirectDraw()
{
   m_pDD = NULL;

   int tmp = 1;
   HRESULT hr = GetRegInt("Player", "HardwareRender", &tmp);
   m_fHardwareAccel = (tmp != 0);
}

PinDirectDraw::~PinDirectDraw()
{
   SAFE_RELEASE(m_pDD);
}

typedef int(CALLBACK *DDCreateFunction)(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);

HRESULT PinDirectDraw::InitDD()
{
   HINSTANCE m_DDraw = LoadLibrary("ddraw.dll");
   if ( m_DDraw==NULL )
   {
      return E_FAIL;
   }
   DDCreateFunction DDCreate = (DDCreateFunction)GetProcAddress(m_DDraw,"DirectDrawCreateEx");

   if (DDCreate == NULL)
   {
      FreeLibrary(m_DDraw);

      LocalString ls(IDS_NEED_DD9);
      MessageBox(g_pvp->m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);

      return E_FAIL;
   }

   HRESULT hr = (*DDCreate)(NULL, (VOID **)&m_pDD, IID_IDirectDraw7, NULL);
   if (hr != S_OK)
      ShowError("Could not create Direct Draw.");

   hr = m_pDD->SetCooperativeLevel(NULL, DDSCL_NORMAL | DDSCL_FPUSETUP); // was DDSCL_FPUPRESERVE, which in theory adds lots of overhead, but who knows if this is even supported nowadays by the drivers
   if (hr != S_OK)
      ShowError("Could not set Direct Draw cooperative level.");

   return S_OK;
}


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

