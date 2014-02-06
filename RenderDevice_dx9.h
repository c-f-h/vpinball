#pragma once

#include <map>
#include "stdafx.h"
#include <d3d9.h>
#include "Material.h"
#include "Texture.h"

#define CHECKD3D(s) { HRESULT hr = (s); if (FAILED(hr)) ReportError(hr, __FILE__, __LINE__); }

void ReportError(HRESULT hr, const char *file, int line);

typedef IDirect3DTexture9 D3DTexture;
typedef D3DVIEWPORT9 ViewPort;
typedef IDirect3DSurface9 RenderTarget;

enum TransformStateType {
    TRANSFORMSTATE_WORLD      = D3DTS_WORLD,
    TRANSFORMSTATE_VIEW       = D3DTS_VIEW,
    TRANSFORMSTATE_PROJECTION = D3DTS_PROJECTION
};


class TextureManager
{
public:
    TextureManager(RenderDevice& rd) : m_rd(rd)
    { }

    ~TextureManager()
    {
        UnloadAll();
    }

    D3DTexture* LoadTexture(MemTexture* memtex);
    void UnloadTexture(MemTexture* memtex);

    void UnloadAll();

private:
    struct TexInfo
    {
        D3DTexture* d3dtex;
        int texWidth;
        int texHeight;
    };

    RenderDevice& m_rd;
    std::map<MemTexture*, TexInfo> m_map;
    typedef std::map<MemTexture*, TexInfo>::iterator Iter;
};


// adds simple setters and getters on top of D3DLIGHT9, for compatibility
struct BaseLight : public D3DLIGHT9
{
    BaseLight()
    {
		ZeroMemory(this, sizeof(*this));
    }

    D3DLIGHTTYPE getType()          { return Type; }
    void setType(D3DLIGHTTYPE lt)   { Type = lt; }

    const D3DCOLORVALUE& getDiffuse()      { return Diffuse; }
    const D3DCOLORVALUE& getSpecular()     { return Specular; }
    const D3DCOLORVALUE& getAmbient()      { return Ambient; }

    void setDiffuse(float r, float g, float b)
    {
        Diffuse.r = r;
        Diffuse.g = g;
        Diffuse.b = b;
    }
    void setSpecular(float r, float g, float b)
    {
        Specular.r = r;
        Specular.g = g;
        Specular.b = b;
    }
    void setAmbient(float r, float g, float b)
    {
        Ambient.r = r;
        Ambient.g = g;
        Ambient.b = b;
    }
    void setPosition(float x, float y, float z)
    {
        Position.x = x;
        Position.y = y;
        Position.z = z;
    }
    void setDirection(float x, float y, float z)
    {
        Direction.x = x;
        Direction.y = y;
        Direction.z = z;
    }
    void setRange(float r)          { Range = r; }
    void setFalloff(float r)        { Falloff = r; }
    void setAttenuation0(float r)   { Attenuation0 = r; }
    void setAttenuation1(float r)   { Attenuation1 = r; }
    void setAttenuation2(float r)   { Attenuation2 = r; }
    void setTheta(float r)          { Theta = r; }
    void setPhi(float r)            { Phi = r; }
};



class VertexBuffer : public IDirect3DVertexBuffer9
{
public:
    enum LockFlags
    {
        WRITEONLY = 0,                        // in DX9, this is specified during VB creation
        NOOVERWRITE = D3DLOCK_NOOVERWRITE,    // meaning: no recently drawn vertices are overwritten. only works with dynamic VBs.
                                              // it's only needed for VBs which are locked several times per frame
        DISCARDCONTENTS = D3DLOCK_DISCARD     // discard previous contents; only works with dynamic VBs
    };
    void lock( unsigned int offsetToLock, unsigned int sizeToLock, void **dataBuffer, DWORD flags )
    {
        CHECKD3D(this->Lock(offsetToLock, sizeToLock, dataBuffer, flags));
    }
    void unlock(void)
    {
        CHECKD3D(this->Unlock());
    }
    void release(void)
    {
        while ( this->Release()!=0 );
    }
private:
    VertexBuffer();     // disable default constructor
};


class IndexBuffer : public IDirect3DIndexBuffer9
{
public:
    enum Format {
        FMT_INDEX16 = D3DFMT_INDEX16,
        FMT_INDEX32 = D3DFMT_INDEX32
    };
    enum LockFlags
    {
        WRITEONLY = 0,                      // in DX9, this is specified during VB creation
        NOOVERWRITE = D3DLOCK_NOOVERWRITE,  // meaning: no recently drawn vertices are overwritten. only works with dynamic VBs.
                                            // it's only needed for VBs which are locked several times per frame
        DISCARD = D3DLOCK_DISCARD           // discard previous contents; only works with dynamic VBs
    };
    void lock( unsigned int offsetToLock, unsigned int sizeToLock, void **dataBuffer, DWORD flags )
    {
        CHECKD3D(this->Lock(offsetToLock, sizeToLock, dataBuffer, flags) );
    }
    void unlock(void)
    {
        CHECKD3D(this->Unlock());
    }
    void release(void)
    {
        while ( this->Release()!=0 );
    }
private:
    IndexBuffer();      // disable default constructor
};



class RenderDevice
{
public:
   typedef enum RenderStates
   {
      ALPHABLENDENABLE   = D3DRS_ALPHABLENDENABLE,
      ALPHATESTENABLE    = D3DRS_ALPHATESTENABLE,
      ALPHAREF           = D3DRS_ALPHAREF,
      ALPHAFUNC          = D3DRS_ALPHAFUNC,
      CLIPPING           = D3DRS_CLIPPING,
      CLIPPLANEENABLE    = D3DRS_CLIPPLANEENABLE,
      COLORKEYENABLE     = 0, //D3DRS_COLORKEYENABLE,   // TODO: not supported, remove
      CULLMODE           = D3DRS_CULLMODE,
      DITHERENABLE       = D3DRS_DITHERENABLE,
      DESTBLEND          = D3DRS_DESTBLEND,
      LIGHTING           = D3DRS_LIGHTING,
      SPECULARENABLE     = D3DRS_SPECULARENABLE,
      SRCBLEND           = D3DRS_SRCBLEND,
      ZENABLE            = D3DRS_ZENABLE,
      ZFUNC              = D3DRS_ZFUNC,
      ZWRITEENABLE       = D3DRS_ZWRITEENABLE,
	  NORMALIZENORMALS   = D3DRS_NORMALIZENORMALS,
      TEXTUREFACTOR      = D3DRS_TEXTUREFACTOR
   };

   enum TextureAddressMode {
       TEX_WRAP          = D3DTADDRESS_WRAP,
       TEX_CLAMP         = D3DTADDRESS_CLAMP,
       TEX_MIRROR        = D3DTADDRESS_MIRROR
   };

   RenderDevice(HWND hwnd, int width, int height, bool fullscreen, int screenWidth, int screenHeight, int colordepth, int &refreshrate);
   ~RenderDevice();

   void BeginScene();
   void EndScene();

   void Clear(DWORD numRects, D3DRECT* rects, DWORD flags, D3DCOLOR color, D3DVALUE z, DWORD stencil);
   void Flip(int offsetx, int offsety, bool vsync);

   RenderTarget* GetBackBuffer() { return m_pBackBuffer; }
   RenderTarget* DuplicateRenderTarget(RenderTarget* src);

   void SetRenderTarget( RenderTarget* );
   void SetZBuffer( RenderTarget* );

   RenderTarget* AttachZBufferTo(RenderTarget* surf);
   void CopySurface(RenderTarget* dest, RenderTarget* src);

   D3DTexture* RenderDevice::UploadTexture(MemTexture* surf, int *pTexWidth=NULL, int *pTexHeight=NULL);
   void SetRenderState( const RenderStates p1, const DWORD p2 );
   void SetTexture( DWORD, D3DTexture* );
   void SetTextureFilter(DWORD texUnit, DWORD mode);
   void SetTextureAddressMode(DWORD texUnit, TextureAddressMode mode);
   void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);
   void SetMaterial( const BaseMaterial * const material );
   void SetMaterial( const Material & material )        { SetMaterial(&material.getBaseMaterial()); }

   void CreateVertexBuffer( unsigned int numVerts, DWORD usage, DWORD fvf, VertexBuffer **vBuffer );
   void CreateIndexBuffer(unsigned int numIndices, DWORD usage, IndexBuffer::Format format, IndexBuffer **idxBuffer);

   IndexBuffer* CreateAndFillIndexBuffer(unsigned int numIndices, const WORD * indices);

   void DrawPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, LPVOID vertices, DWORD vertexCount);
   void DrawIndexedPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, LPVOID vertices, DWORD vertexCount, LPWORD indices, DWORD indexCount);
   void DrawPrimitiveVB(D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount);
   void DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount, LPWORD indices, DWORD indexCount);
   void DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount, IndexBuffer* ib, DWORD startIndex, DWORD indexCount);

   void GetMaterial( BaseMaterial *_material );

   void LightEnable( DWORD, BOOL );
   void SetLight( DWORD, BaseLight* );
   void GetLight( DWORD, BaseLight* );
   void SetViewport( ViewPort* );
   void GetViewport( ViewPort* );

   void SetTransform( TransformStateType, D3DMATRIX* );
   void GetTransform( TransformStateType, D3DMATRIX* );

   void SetColorKeyEnabled(bool enable)
   {
       // TODO remove SetRenderState(RenderDevice::COLORKEYENABLE, enable);
   }

   void setVBInVRAM( const BOOL state )
   {
      // TODO remove vbInVRAM=(state==1);
   }

private:
   CComPtr<IDirect3D9> m_pD3D;
   CComPtr<IDirect3DDevice9> m_pD3DDevice;

   IDirect3DSurface9* m_pBackBuffer;
   CComPtr<IndexBuffer> m_dynIndexBuffer;      // workaround for DrawIndexedPrimitiveVB

   UINT m_adapter;      // index of the display adapter to use

   static const DWORD RENDER_STATE_CACHE_SIZE=256;
   static const DWORD TEXTURE_STATE_CACHE_SIZE=256;

   DWORD renderStateCache[RENDER_STATE_CACHE_SIZE];
   DWORD textureStateCache[8][TEXTURE_STATE_CACHE_SIZE];
   BaseMaterial materialStateCache;

public:
   TextureManager m_texMan;
};
