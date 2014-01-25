#include "stdafx.h"
#include "Material.h"
#pragma once


typedef IDirectDrawSurface7 BaseTexture;
typedef D3DVIEWPORT7 ViewPort;
typedef IDirectDrawSurface7 RenderTarget;

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

   RenderDevice();
   ~RenderDevice();

   bool createDevice(const GUID * const _deviceGUID, LPDIRECT3D7 _dx7, BaseTexture *_backBuffer );

   void SetMaterial( const BaseMaterial * const _material );
   void SetMaterial( const Material & material )        { SetMaterial(&material.getBaseMaterial()); }
   void SetRenderState( const RenderStates p1, const DWORD p2 );
   bool createVertexBuffer( unsigned int _length, DWORD _usage, DWORD _fvf, VertexBuffer **_vBuffer );
   void renderPrimitive(D3DPRIMITIVETYPE _primType, VertexBuffer* _vbuffer, DWORD _startVertex, DWORD _numVertices, LPWORD _indices, DWORD _numIndices, DWORD _flags);
   void renderPrimitiveListed(D3DPRIMITIVETYPE _primType, VertexBuffer* _vbuffer, DWORD _startVertex, DWORD _numVertices, DWORD _flags);

   void SetColorKeyEnabled(bool enable)
   {
       SetRenderState(RenderDevice::COLORKEYENABLE, enable);
   }

   inline void setVBInVRAM( const BOOL _state )
   {
      vbInVRAM=(_state==1);
   }

   //########################## simple wrapper functions (interface for DX7)##################################

   HRESULT QueryInterface( THIS_ REFIID riid, LPVOID * ppvObj );

   //HRESULT GetCaps( THIS_ LPD3DDEVICEDESC7 );

   //HRESULT EnumTextureFormats( THIS_ LPD3DENUMPIXELFORMATSCALLBACK,LPVOID );

   HRESULT BeginScene( THIS );

   HRESULT EndScene( THIS );

   //HRESULT GetDirect3D( THIS_ LPDIRECT3D7* );

   HRESULT SetRenderTarget( THIS_ RenderTarget*,DWORD );

   HRESULT GetRenderTarget( THIS_ RenderTarget* * );

   HRESULT Clear( THIS_ DWORD,LPD3DRECT,DWORD,D3DCOLOR,D3DVALUE,DWORD );

   HRESULT SetTransform( THIS_ D3DTRANSFORMSTATETYPE,LPD3DMATRIX );

   HRESULT GetTransform( THIS_ D3DTRANSFORMSTATETYPE,LPD3DMATRIX );

   HRESULT SetViewport( THIS_ ViewPort* );

   HRESULT MultiplyTransform( THIS_ D3DTRANSFORMSTATETYPE,LPD3DMATRIX );

   HRESULT GetViewport( THIS_ ViewPort* );

   //HRESULT SetMaterial( THIS_ LPD3DMATERIAL7 );

   //HRESULT GetMaterial( THIS_ LPD3DMATERIAL7 );

   void getMaterial( THIS_ BaseMaterial *_material );

   HRESULT SetLight( THIS_ DWORD,LPD3DLIGHT7 );

   HRESULT GetLight( THIS_ DWORD,LPD3DLIGHT7 );

   HRESULT SetRenderState( THIS_ D3DRENDERSTATETYPE,DWORD );

   HRESULT GetRenderState( THIS_ D3DRENDERSTATETYPE,LPDWORD );

   HRESULT BeginStateBlock( THIS );

   HRESULT EndStateBlock( THIS_ LPDWORD );

   HRESULT PreLoad( THIS_ LPDIRECTDRAWSURFACE7 );

   HRESULT DrawPrimitive( THIS_ D3DPRIMITIVETYPE,DWORD,LPVOID,DWORD,DWORD );

   HRESULT DrawIndexedPrimitive( THIS_ D3DPRIMITIVETYPE,DWORD,LPVOID,DWORD,LPWORD,DWORD,DWORD );

   HRESULT SetClipStatus( THIS_ LPD3DCLIPSTATUS );

   HRESULT GetClipStatus( THIS_ LPD3DCLIPSTATUS );

   HRESULT DrawPrimitiveStrided( THIS_ D3DPRIMITIVETYPE,DWORD,LPD3DDRAWPRIMITIVESTRIDEDDATA,DWORD,DWORD );

   HRESULT DrawIndexedPrimitiveStrided( THIS_ D3DPRIMITIVETYPE,DWORD,LPD3DDRAWPRIMITIVESTRIDEDDATA,DWORD,LPWORD,DWORD,DWORD );

   HRESULT DrawPrimitiveVB( THIS_ D3DPRIMITIVETYPE,LPDIRECT3DVERTEXBUFFER7,DWORD,DWORD,DWORD );

   HRESULT DrawIndexedPrimitiveVB( THIS_ D3DPRIMITIVETYPE,LPDIRECT3DVERTEXBUFFER7,DWORD,DWORD,LPWORD,DWORD,DWORD );

   HRESULT ComputeSphereVisibility( THIS_ LPD3DVECTOR,LPD3DVALUE,DWORD,DWORD,LPDWORD );

   HRESULT GetTexture( THIS_ DWORD,LPDIRECTDRAWSURFACE7 * );

   HRESULT SetTexture( THIS_ DWORD,LPDIRECTDRAWSURFACE7 );

   HRESULT GetTextureStageState( THIS_ DWORD,D3DTEXTURESTAGESTATETYPE,LPDWORD );

   HRESULT SetTextureStageState( THIS_ DWORD,D3DTEXTURESTAGESTATETYPE,DWORD );

   HRESULT ValidateDevice( THIS_ LPDWORD );

   HRESULT ApplyStateBlock( THIS_ DWORD );

   HRESULT CaptureStateBlock( THIS_ DWORD );

   HRESULT DeleteStateBlock( THIS_ DWORD );

   HRESULT CreateStateBlock( THIS_ D3DSTATEBLOCKTYPE,LPDWORD );

   HRESULT Load( THIS_ LPDIRECTDRAWSURFACE7,LPPOINT,LPDIRECTDRAWSURFACE7,LPRECT,DWORD );

   HRESULT LightEnable( THIS_ DWORD,BOOL );

   HRESULT GetLightEnable( THIS_ DWORD,BOOL* );

   HRESULT SetClipPlane( THIS_ DWORD,D3DVALUE* );

   HRESULT GetClipPlane( THIS_ DWORD,D3DVALUE* );

   HRESULT GetInfo( THIS_ DWORD,LPVOID,DWORD );

private:
   static const DWORD RENDER_STATE_CACHE_SIZE=256;
   static const DWORD TEXTURE_STATE_CACHE_SIZE=256;

   DWORD renderStateCache[RENDER_STATE_CACHE_SIZE];
   DWORD textureStateCache[8][TEXTURE_STATE_CACHE_SIZE];
   BaseMaterial materialStateCache;
   bool vbInVRAM;

   LPDIRECT3DDEVICE7 dx7Device;
   LPDIRECT3D7 dx7;
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
   inline bool lock( unsigned int _offsetToLock, unsigned int _sizeToLock, void **_dataBuffer, DWORD _flags )
   {
      return ( !FAILED(this->Lock( (DWORD)_flags, _dataBuffer, 0 )) );
   }
   inline bool unlock(void)
   {
      return ( !FAILED(this->Unlock() ) );
   }
   inline ULONG release(void)
   {
      while ( this->Release()!=0 );
      return 0;
   }
};
