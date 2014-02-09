#ifdef VPINBALL_DX9

#include "RenderDevice_dx9.h"

#else

#include "stdafx.h"
#include "Material.h"
#pragma once

// NB: this has the same layout as D3DVIEWPORT7 (and 9)
struct ViewPort {
    DWORD       X;
    DWORD       Y;
    DWORD       Width;
    DWORD       Height;
    D3DVALUE    MinZ;
    D3DVALUE    MaxZ;
};

typedef IDirectDrawSurface7 BaseTexture;
//typedef D3DVIEWPORT7 ViewPort;
typedef IDirectDrawSurface7 RenderTarget;

enum TransformStateType {
    TRANSFORMSTATE_WORLD      = D3DTRANSFORMSTATE_WORLD,
    TRANSFORMSTATE_VIEW       = D3DTRANSFORMSTATE_VIEW,
    TRANSFORMSTATE_PROJECTION = D3DTRANSFORMSTATE_PROJECTION
};

struct BaseLight : public D3DLIGHT7
{
    BaseLight()
    {
		ZeroMemory(this, sizeof(*this));
    }

    D3DLIGHTTYPE getType()          { return dltType; }
    void setType(D3DLIGHTTYPE lt)   { dltType = lt; }

    const D3DCOLORVALUE& getDiffuse()      { return dcvDiffuse; }
    const D3DCOLORVALUE& getSpecular()     { return dcvSpecular; }
    const D3DCOLORVALUE& getAmbient()      { return dcvAmbient; }

    void setDiffuse(float r, float g, float b)
    {
        dcvDiffuse.r = r;
        dcvDiffuse.g = g;
        dcvDiffuse.b = b;
    }
    void setSpecular(float r, float g, float b)
    {
        dcvSpecular.r = r;
        dcvSpecular.g = g;
        dcvSpecular.b = b;
    }
    void setAmbient(float r, float g, float b)
    {
        dcvAmbient.r = r;
        dcvAmbient.g = g;
        dcvAmbient.b = b;
    }
    void setPosition(float x, float y, float z)
    {
        dvPosition.x = x;
        dvPosition.y = y;
        dvPosition.z = z;
    }
    void setDirection(float x, float y, float z)
    {
        dvDirection.x = x;
        dvDirection.y = y;
        dvDirection.z = z;
    }
    void setRange(float r)          { dvRange = r; }
    void setFalloff(float r)        { dvFalloff = r; }
    void setAttenuation0(float r)   { dvAttenuation0 = r; }
    void setAttenuation1(float r)   { dvAttenuation1 = r; }
    void setAttenuation2(float r)   { dvAttenuation2 = r; }
    void setTheta(float r)          { dvTheta = r; }
    void setPhi(float r)            { dvPhi = r; }
};


class VertexBuffer;

class RenderDevice
{
public:

   typedef enum RenderStates
   {
      ALPHABLENDENABLE   = D3DRENDERSTATE_ALPHABLENDENABLE,
      ALPHATESTENABLE    = D3DRENDERSTATE_ALPHATESTENABLE,
      ALPHAREF           = D3DRENDERSTATE_ALPHAREF,
      ALPHAFUNC          = D3DRENDERSTATE_ALPHAFUNC,
      CLIPPING           = D3DRENDERSTATE_CLIPPING,
      CLIPPLANEENABLE    = D3DRENDERSTATE_CLIPPLANEENABLE,
      COLORKEYENABLE     = D3DRENDERSTATE_COLORKEYENABLE,
      CULLMODE           = D3DRENDERSTATE_CULLMODE,
      DITHERENABLE       = D3DRENDERSTATE_DITHERENABLE,
      DESTBLEND          = D3DRENDERSTATE_DESTBLEND,
      LIGHTING           = D3DRENDERSTATE_LIGHTING,
      SPECULARENABLE     = D3DRENDERSTATE_SPECULARENABLE,
      SRCBLEND           = D3DRENDERSTATE_SRCBLEND,
      TEXTUREPERSPECTIVE = D3DRENDERSTATE_TEXTUREPERSPECTIVE,
      ZENABLE            = D3DRENDERSTATE_ZENABLE,
      ZFUNC              = D3DRENDERSTATE_ZFUNC,
      ZWRITEENABLE       = D3DRENDERSTATE_ZWRITEENABLE,
	  NORMALIZENORMALS   = D3DRENDERSTATE_NORMALIZENORMALS,
      TEXTUREFACTOR      = D3DRENDERSTATE_TEXTUREFACTOR
   };

   enum TextureAddressMode {
       TEX_WRAP          = D3DTADDRESS_WRAP,
       TEX_CLAMP         = D3DTADDRESS_CLAMP,
       TEX_MIRROR        = D3DTADDRESS_MIRROR
   };

   RenderDevice();
   ~RenderDevice();

   bool InitRenderer(HWND hwnd, int width, int height, bool fullscreen, int screenWidth, int screenHeight, int colordepth, int &refreshrate);
   void DestroyRenderer();

   void BeginScene();
   void EndScene();

   void Clear(DWORD numRects, D3DRECT* rects, DWORD flags, D3DCOLOR color, D3DVALUE z, DWORD stencil);
   void Flip(int offsetx, int offsety, bool vsync);

   RenderTarget* GetBackBuffer() { return m_pddsBackBuffer; }
   RenderTarget* DuplicateRenderTarget(RenderTarget* src);

   void SetRenderTarget( RenderTarget* );
   void SetZBuffer( RenderTarget* );

   RenderTarget* AttachZBufferTo(RenderTarget* surf);
   void CopySurface(RenderTarget* dest, RenderTarget* src);

   void GetTextureSize(BaseTexture* tex, DWORD *width, DWORD *height);

   void SetRenderState( const RenderStates p1, const DWORD p2 );
   void SetTexture( DWORD, BaseTexture* );
   void SetTextureFilter(DWORD texUnit, DWORD mode);
   void SetTextureAddressMode(DWORD texUnit, TextureAddressMode mode);
   void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);
   void SetMaterial( const BaseMaterial * const material );
   void SetMaterial( const Material & material )        { SetMaterial(&material.getBaseMaterial()); }

   void CreateVertexBuffer( unsigned int numVerts, DWORD usage, DWORD fvf, VertexBuffer **vBuffer );

   void DrawPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, LPVOID vertices, DWORD vertexCount);
   void DrawIndexedPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, LPVOID vertices, DWORD vertexCount, LPWORD indices, DWORD indexCount);
   void DrawPrimitiveVB(D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD numVertices);
   void DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD numVertices, LPWORD indices, DWORD indexCount);

   void GetMaterial( BaseMaterial *_material );

   void LightEnable( DWORD, BOOL );
   void SetLight( DWORD, BaseLight* );
   void GetLight( DWORD, BaseLight* );
   void SetViewport( ViewPort* );
   void GetViewport( ViewPort* );

   void SetTransform( TransformStateType, LPD3DMATRIX );
   void GetTransform( TransformStateType, LPD3DMATRIX );

   void setVBInVRAM( const BOOL state )
   {
      vbInVRAM=(state==1);
   }

public:  //########################## simple wrapper functions (interface for DX7)##################################

   //HRESULT GetRenderTarget( THIS_ RenderTarget* * );
   //HRESULT SetMaterial( THIS_ LPD3DMATERIAL7 );
   //HRESULT GetMaterial( THIS_ LPD3DMATERIAL7 );
   //HRESULT GetRenderState( THIS_ D3DRENDERSTATETYPE,LPDWORD );
   //HRESULT BeginStateBlock( THIS );
   //HRESULT EndStateBlock( THIS_ LPDWORD );
   //HRESULT SetClipStatus( THIS_ LPD3DCLIPSTATUS );
   //HRESULT GetClipStatus( THIS_ LPD3DCLIPSTATUS );
   //HRESULT GetTexture( THIS_ DWORD,LPDIRECTDRAWSURFACE7 * );
   //HRESULT GetTextureStageState( THIS_ DWORD,D3DTEXTURESTAGESTATETYPE,LPDWORD );
   //HRESULT ValidateDevice( THIS_ LPDWORD );
   //HRESULT ApplyStateBlock( THIS_ DWORD );
   //HRESULT CaptureStateBlock( THIS_ DWORD );
   //HRESULT DeleteStateBlock( THIS_ DWORD );
   //HRESULT CreateStateBlock( THIS_ D3DSTATEBLOCKTYPE,LPDWORD );
   //HRESULT GetLightEnable( THIS_ DWORD,BOOL* );
   //HRESULT SetClipPlane( THIS_ DWORD,D3DVALUE* );
   //HRESULT GetClipPlane( THIS_ DWORD,D3DVALUE* );

   // TODO make private
   RECT m_rcUpdate;     // HACK
private:
   LPDIRECT3D7 m_pD3D;

   static const DWORD RENDER_STATE_CACHE_SIZE=256;
   static const DWORD TEXTURE_STATE_CACHE_SIZE=256;

   DWORD renderStateCache[RENDER_STATE_CACHE_SIZE];
   DWORD textureStateCache[8][TEXTURE_STATE_CACHE_SIZE];
   BaseMaterial materialStateCache;
   bool vbInVRAM;

   LPDIRECT3DDEVICE7 dx7Device;
   LPDIRECTDRAWSURFACE7 m_pddsFrontBuffer;
   LPDIRECTDRAWSURFACE7 m_pddsBackBuffer;

};


class VertexBuffer : public IDirect3DVertexBuffer7
{
public:
   enum LockFlags
   {
      WRITEONLY = DDLOCK_WRITEONLY,
      NOOVERWRITE = DDLOCK_NOOVERWRITE,
      DISCARDCONTENTS = DDLOCK_DISCARDCONTENTS
   };
   bool lock( unsigned int _offsetToLock, unsigned int _sizeToLock, void **_dataBuffer, DWORD _flags )
   {
      return ( !FAILED(this->Lock( (DWORD)_flags, _dataBuffer, 0 )) );
   }
   bool unlock(void)
   {
      return ( !FAILED(this->Unlock() ) );
   }
   ULONG release(void)
   {
      while ( this->Release()!=0 );
      return 0;
   }
};

#endif // VPINBALL_DX9
