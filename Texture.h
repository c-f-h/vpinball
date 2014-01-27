#include "stdafx.h"
#include "RenderDevice.h"
#pragma once

#ifdef RGB
#undef RGB
#endif
#define BGR(b,g,r) ((COLORREF)(((DWORD)(b)) | (((DWORD)(g))<<8) | (((DWORD)(r))<<16)))
#define RGB(r,g,b) ((COLORREF)(((DWORD)(r)) | (((DWORD)(g))<<8) | (((DWORD)(b))<<16)))

#define NOTRANSCOLOR  RGB(123,123,123)


class Texture : public ILoadable
{
public:
   Texture();
   virtual ~Texture();

   // ILoadable callback
   virtual BOOL LoadToken(int id, BiffReader *pbr);

   HRESULT SaveToStream(IStream *pstream, PinTable *pt);
   HRESULT LoadFromStream(IStream *pstream, int version, PinTable *pt);

   void CreateAlphaChannel();
   void EnsureBackdrop(const COLORREF color);
   void FreeStuff();
   void SetTransparentColor(const COLORREF color);
   void EnsureMaxTextureCoordinates();

   static void SetRenderDevice( RenderDevice *_device );
   void SetBackDrop( DWORD textureChannel );
   inline void Set( DWORD textureChannel )
   {
      renderDevice->SetTexture( textureChannel, m_pdsBufferColorKey);
   }

   void Release();
   void EnsureHBitmap();
   void CreateGDIVersion();
   void CreateFromResource(const int id, int * const pwidth, int * const pheight);

   BaseTexture *CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight);
   BaseTexture *CreateBaseTexture(const int width, const int height);

   void CreateTextureOffscreen(const int width, const int height);
   void CreateMipMap();
   BOOL SetAlpha(const COLORREF rgbTransparent, const int width, const int height);

   void Lock();
   void Unlock();

   static void CreateNextMipMapLevel(BaseTexture* pdds);
   static void SetOpaque(BaseTexture* pdds, const int width, const int height);
   static void SetOpaqueBackdrop(BaseTexture* pdds, const COLORREF rgbTransparent, const COLORREF rgbBackdrop, const int width, const int height);
   static BOOL SetAlpha(BaseTexture* pdds, const COLORREF rgbTransparent, const int width, const int height);
   static void Blur(BaseTexture* pdds, const BYTE * const pbits, const int shadwidth, const int shadheight);

   void Unset( const DWORD textureChannel );

   // create/release a DC which contains a (read-only) copy of the texture; for editor use
   void GetTextureDC(HDC *pdc);
   void ReleaseTextureDC(HDC dc);

private:
   bool LoadFromMemory(BYTE *data, DWORD size);

public:

   // width and height of texture can be different than width and height
   // of dd surface, since the surface has to be in powers of 2
   int m_width, m_height;
   COLORREF m_rgbTransparent;
   BOOL m_fTransparent; // Whether this picture actually contains transparent bits

   // Filled at runtime, accounts for buffer space to meet the power of 2
   // requirement
   float m_maxtu, m_maxtv;

   BaseTexture* m_pdsBuffer;
   HBITMAP m_hbmGDIVersion; // HBitmap at screen depth so GDI draws it fast
   PinBinary *m_ppb;  // if this image should be saved as a binary stream, otherwise just LZW compressed from the live bitmap

   char m_szName[MAXTOKEN];
   char m_szInternalName[MAXTOKEN];
   char m_szPath[MAX_PATH];

   int m_originalWidth, m_originalHeight;


private:
   COLORREF m_rgbBackdropCur;

   BaseTexture* m_pdsBufferColorKey;
   BaseTexture* m_pdsBufferBackdrop;

   static RenderDevice *renderDevice;

   HBITMAP m_oldHBM;        // this is to cache the result of SelectObject()
};
