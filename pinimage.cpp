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
   if( FAILED( hr = m_pDD->CreateSurface( &ddsd, (LPDIRECTDRAWSURFACE7*)&pdds, NULL ) ) )
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
}

BaseTexture* PinDirectDraw::CreateFromFile(char *szfile, int * const pwidth, int * const pheight, int& originalWidth, int& originalHeight)
{
   FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

   // check the file signature and deduce its format
   // (the second argument is currently not used by FreeImage)
   fif = FreeImage_GetFileType(szfile, 0);
   if(fif == FIF_UNKNOWN) {
      // no signature ?
      // try to guess the file format from the file extension
      fif = FreeImage_GetFIFFromFilename(szfile);
   }
   // check that the plugin has reading capabilities ...
   if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
      // ok, let's load the file
      FIBITMAP *dib = FreeImage_Load(fif, szfile, 0);

      // check if Textures exceed the maximum texture diemension
      int maxTexDim;
      HRESULT hrMaxTex = GetRegInt("Player", "MaxTexDimension", &maxTexDim);
      if (hrMaxTex != S_OK)
      {
         maxTexDim = 0; // default: Don't resize textures
      }
      if((maxTexDim <= 0) || (maxTexDim > MAX_TEXTURE_SIZE))
      {
         maxTexDim = MAX_TEXTURE_SIZE;
      }
      int pictureWidth = FreeImage_GetWidth(dib);
      int pictureHeight = FreeImage_GetHeight(dib);
      // save original width and height, if the texture is rescaled
      originalWidth = pictureWidth;
      originalHeight = pictureHeight;
      if ((pictureHeight > maxTexDim) || (pictureWidth > maxTexDim))
      {
         dib = FreeImage_Rescale(dib, min(pictureWidth,maxTexDim), min(pictureHeight,maxTexDim), FILTER_BILINEAR);
      }

      HDC hDC = GetDC(NULL);
      HBITMAP hbm = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(dib),CBM_INIT, FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);
      int bitsPerPixel = FreeImage_GetBPP(dib);
      int dibWidth = FreeImage_GetWidth(dib);
      int dibHeight = FreeImage_GetHeight(dib);

      FreeImage_Unload(dib);

      BaseTexture* mySurface = CreateFromHBitmap(hbm, pwidth, pheight);

      if (bitsPerPixel == 24)
         Texture::SetOpaque(mySurface, dibWidth, dibHeight);

      return (hbm == NULL) ? NULL : mySurface;
   }
   else
      return NULL;
}

BaseTexture* PinDirectDraw::CreateFromResource(const int id, int * const pwidth, int * const pheight)
{
   HBITMAP hbm = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

   if (hbm == NULL)
   {
      return NULL;
   }

   return CreateFromHBitmap(hbm, pwidth, pheight);
}

BaseTexture* PinDirectDraw::CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight)
{
   BITMAP bm;
   GetObject(hbm, sizeof(bm), &bm);

   if (pwidth)
   {
      *pwidth = bm.bmWidth;
   }

   if (pheight)
   {
      *pheight = bm.bmHeight;
   }

   if (bm.bmWidth > MAX_TEXTURE_SIZE || bm.bmHeight > MAX_TEXTURE_SIZE)
   {
      return NULL; // MAX_TEXTURE_SIZE is the limit for directx7 textures
   }

   BaseTexture* pdds = CreateTextureOffscreen(bm.bmWidth, bm.bmHeight);

#ifndef VPINBALL_DX9
   HDC hdc;
   pdds->GetDC(&hdc);

   HDC hdcFoo = CreateCompatibleDC(hdc);
   HBITMAP hbmOld = (HBITMAP)SelectObject(hdcFoo, hbm);

   BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcFoo, 0, 0, SRCCOPY);

   SelectObject(hdcFoo, hbmOld);
   DeleteDC(hdcFoo);
   DeleteObject(hbm);
   pdds->ReleaseDC(hdc); 

   if (bm.bmBitsPixel != 32) 
      Texture::SetOpaque(pdds, bm.bmWidth, bm.bmHeight);
#endif

   return pdds;
}

BaseTexture* PinDirectDraw::CreateOffscreenPlain(const int width, const int height)
{
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);

	if (width < 1 || height < 1)
		return NULL;

	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_CKSRCBLT;
	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = 0;//0xffffff;
	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = 0;//0xffffff;
	ddsd.dwWidth        = width;
	ddsd.dwHeight       = height;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

	// Check if we are rendering in hardware.
	ddsd.ddsCaps.dwCaps |= (m_fHardwareAccel) ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY;

retry1:
	HRESULT hr;
	BaseTexture* pdds;
	if( FAILED( hr = m_pDD->CreateSurface( &ddsd, (LPDIRECTDRAWSURFACE7*)&pdds, NULL ) ) )
	{
		if((ddsd.ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) == 0) {
			ddsd.ddsCaps.dwCaps |= DDSCAPS_NONLOCALVIDMEM;
			goto retry1;
		}
		ShowError("Could not create offscreen surface.");
		exit(-1400);
		return NULL;
	}

	// Update the count.
	NumVideoBytes += ddsd.dwWidth * ddsd.dwHeight * (ddsd.ddpfPixelFormat.dwRGBBitCount/8);

	return pdds;
}

BaseTexture* PinDirectDraw::CreateOffscreenWithCustomTransparency(const int width, const int height, const int color)
{
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
	if( FAILED( hr = m_pDD->CreateSurface( &ddsd, (LPDIRECTDRAWSURFACE7*)&pdds, NULL ) ) )
	{
		if((ddsd.ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) == 0) {
			ddsd.ddsCaps.dwCaps |= DDSCAPS_NONLOCALVIDMEM;
			goto retry0;
		}
		ShowError("Could not create offscreen surface.");
		return NULL;
	}

	return pdds;
}

