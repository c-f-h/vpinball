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

BaseTexture* PinDirectDraw::CreateTextureOffscreen(const int width, const int height, DWORD* out_texWidth, DWORD* out_texHeight)
{
#ifdef VPINBALL_DX9
    return NULL;
#else
   DDSURFACEDESC2 ddsd;
   ZeroMemory( &ddsd, sizeof(ddsd) );
   ddsd.dwSize = sizeof(ddsd);

   //!! power of two only necessary because of mipmaps?
   int texwidth = 8; // Minimum size 8
   while(texwidth < width)
      texwidth <<= 1;

   int texheight = 8;
   while(texheight < height)
      texheight <<= 1;

   // D3D7 does not support textures greater than 4096 in either dimension
   if (texwidth > MAX_TEXTURE_SIZE)
   {
      texwidth = MAX_TEXTURE_SIZE;
   }

   if (texheight > MAX_TEXTURE_SIZE)
   {
      texheight = MAX_TEXTURE_SIZE;
   }

   ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | /*DDSD_CKSRCBLT |*/ DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
   ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = 0;
   ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = 0;
   ddsd.dwWidth        = texwidth;
   ddsd.dwHeight       = texheight;
   ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP; 
   ddsd.dwMipMapCount	= 4;

   if (m_fHardwareAccel)
   {
#if 1
      // Create the texture and let D3D driver decide where it store it.
      ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
#else
      // Create the texture in video memory.
      ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
#endif
   }
   else
   {
      // Create the texture in system memory.
      ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
   }

   ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
   ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
   ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
   ddsd.ddpfPixelFormat.dwRBitMask        = 0x00ff0000;
   ddsd.ddpfPixelFormat.dwGBitMask        = 0x0000ff00;
   ddsd.ddpfPixelFormat.dwBBitMask        = 0x000000ff;
   ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;

   BaseTexture* pdds;
   HRESULT hr;
   bool retryflag = (m_fHardwareAccel != 0);
retryimage:
   if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &pdds, NULL ) ) )
   {
      if(retryflag)
      {
         ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP | DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM;
         ddsd.ddsCaps.dwCaps2 = 0;
         retryflag = false;
         goto retryimage;
      }
      ShowError("Could not create texture offscreen surface.");
      return NULL;
   }

   if (out_texWidth)  *out_texWidth = ddsd.dwWidth;
   if (out_texHeight) *out_texHeight = ddsd.dwHeight;

   // Update the count (including mipmaps).
   NumVideoBytes += (unsigned int)((float)(ddsd.dwWidth * ddsd.dwHeight * 4) * (float)(4.0/3.0));

   pdds->SetLOD(0);

   return pdds;
#endif
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

