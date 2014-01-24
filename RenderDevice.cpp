#include "stdafx.h"
#include "RenderDevice.h"
#include "Material.h"
#include "Texture.h"

bool RenderDevice::createDevice(const GUID * const _deviceGUID, LPDIRECT3D7 _dx7, BaseTexture *_backBuffer )
{
   dx7=_dx7;
   memset( renderStateCache, 0xFFFFFFFF, sizeof(DWORD)*RENDER_STATE_CACHE_SIZE);
   for( int i=0;i<8;i++ )
      for( unsigned int j=0;j<TEXTURE_STATE_CACHE_SIZE;j++ )
         textureStateCache[i][j]=0xFFFFFFFF;
   memset(&materialStateCache, 0xFFFFFFFF, sizeof(Material));

   HRESULT hr;
   if( FAILED( hr = dx7->CreateDevice( *_deviceGUID, (LPDIRECTDRAWSURFACE7)_backBuffer, &dx7Device ) ) )
   {
      return false;
   }
   return true;
}

RenderDevice::RenderDevice()
{
   vbInVRAM=false;
   Texture::SetRenderDevice(this);
   memset( renderStateCache, 0xFFFFFFFF, sizeof(DWORD)*RENDER_STATE_CACHE_SIZE);
   for( int i=0;i<8;i++ )
      for( unsigned int j=0;j<TEXTURE_STATE_CACHE_SIZE;j++ )
         textureStateCache[i][j]=0xFFFFFFFF;
   memset(&materialStateCache, 0xFFFFFFFF, sizeof(Material));
}

RenderDevice::~RenderDevice()
{
   dx7Device->Release();
}

void RenderDevice::SetMaterial( const BaseMaterial * const _material )
{
#if !defined(DEBUG_XXX) && !defined(_CRTDBG_MAP_ALLOC)
   // this produces a crash if BaseMaterial isn't proper aligned to 16byte (in vbatest.cpp new/delete is overloaded for that)
   if(_mm_movemask_ps(_mm_and_ps(
	  _mm_and_ps(_mm_cmpeq_ps(_material->d,materialStateCache.d),_mm_cmpeq_ps(_material->a,materialStateCache.a)),
	  _mm_and_ps(_mm_cmpeq_ps(_material->s,materialStateCache.s),_mm_cmpeq_ps(_material->e,materialStateCache.e)))) == 15
	  &&
	  _material->power == materialStateCache.power)
	  return;

   materialStateCache.d = _material->d;
   materialStateCache.a = _material->a;
   materialStateCache.e = _material->e;
   materialStateCache.s = _material->s;
   materialStateCache.power = _material->power;
#endif

   dx7Device->SetMaterial((LPD3DMATERIAL7)_material);
}

void RenderDevice::SetRenderState( const RenderStates p1, const DWORD p2 )
{
   if ( (unsigned int)p1 < RENDER_STATE_CACHE_SIZE) 
   {
      if( renderStateCache[p1]==p2 )
      {
         // this render state is already set -> don't do anything then
         return;
      }
      renderStateCache[p1]=p2;
   }
   dx7Device->SetRenderState((D3DRENDERSTATETYPE)p1,p2);
}

bool RenderDevice::createVertexBuffer( unsigned int _length, DWORD _usage, DWORD _fvf, VertexBuffer **_vBuffer )
{
   D3DVERTEXBUFFERDESC vbd;
   vbd.dwSize = sizeof(vbd);
   if( vbInVRAM )
      vbd.dwCaps = 0; // set nothing to let the driver decide what's best for us ;)
   else
      vbd.dwCaps = D3DVBCAPS_WRITEONLY | D3DVBCAPS_SYSTEMMEMORY; // essential on some setups to enforce this variant

   vbd.dwFVF = _fvf;
   vbd.dwNumVertices = _length;
   dx7->CreateVertexBuffer(&vbd, (LPDIRECT3DVERTEXBUFFER7*)_vBuffer, 0);  //!! Later-on/DX9: CreateIndexBuffer (and release/realloc them??)
   return true;
}

void RenderDevice::renderPrimitive(D3DPRIMITIVETYPE _primType, VertexBuffer* _vbuffer, DWORD _startVertex, DWORD _numVertices, LPWORD _indices, DWORD _numIndices, DWORD _flags)
{
   dx7Device->DrawIndexedPrimitiveVB( _primType, (LPDIRECT3DVERTEXBUFFER7)_vbuffer, _startVertex, _numVertices, _indices, _numIndices, _flags );
}

void RenderDevice::renderPrimitiveListed(D3DPRIMITIVETYPE _primType, VertexBuffer* _vbuffer, DWORD _startVertex, DWORD _numVertices, DWORD _flags)
{
   dx7Device->DrawPrimitiveVB( _primType, (LPDIRECT3DVERTEXBUFFER7)_vbuffer, _startVertex, _numVertices, _flags );
}

//########################## simple wrapper functions (interface for DX7)##################################

HRESULT RenderDevice::QueryInterface( THIS_ REFIID riid, LPVOID * ppvObj )
{
   return dx7Device->QueryInterface(riid,ppvObj);
}

//HRESULT RenderDevice::GetCaps( THIS_ LPD3DDEVICEDESC7 p1)
//{
//   return dx7Device->GetCaps(p1);
//}

//HRESULT RenderDevice::EnumTextureFormats( THIS_ LPD3DENUMPIXELFORMATSCALLBACK p1,LPVOID p2)
//{
//   return dx7Device->EnumTextureFormats(p1,p2);
//}

HRESULT RenderDevice::BeginScene( THIS )
{
   return dx7Device->BeginScene();
}

HRESULT RenderDevice::EndScene( THIS )
{
   memset( renderStateCache, 0xFFFFFFFF, sizeof(DWORD)*RENDER_STATE_CACHE_SIZE);
   memset(&materialStateCache, 0xFFFFFFFF, sizeof(Material));
   return dx7Device->EndScene();
}

//HRESULT RenderDevice::GetDirect3D( THIS_ LPDIRECT3D7* p1)
//{
//   return dx7Device->GetDirect3D(p1);
//}

HRESULT RenderDevice::SetRenderTarget( THIS_ LPDIRECTDRAWSURFACE7 p1,DWORD p2)
{
   return dx7Device->SetRenderTarget(p1,p2);
}

HRESULT RenderDevice::GetRenderTarget( THIS_ LPDIRECTDRAWSURFACE7 *p1 )
{
   return dx7Device->GetRenderTarget(p1);
}

HRESULT RenderDevice::Clear( THIS_ DWORD p1,LPD3DRECT p2,DWORD p3,D3DCOLOR p4,D3DVALUE p5,DWORD p6)
{
   return dx7Device->Clear(p1,p2,p3,p4,p5,p6);
}

HRESULT RenderDevice::SetTransform( THIS_ D3DTRANSFORMSTATETYPE p1,LPD3DMATRIX p2)
{
   return dx7Device->SetTransform(p1,p2);
}

HRESULT RenderDevice::GetTransform( THIS_ D3DTRANSFORMSTATETYPE p1,LPD3DMATRIX p2)
{
   return dx7Device->GetTransform(p1,p2);
}

HRESULT RenderDevice::SetViewport( THIS_ LPD3DVIEWPORT7 p1)
{
   return dx7Device->SetViewport(p1);
}

HRESULT RenderDevice::MultiplyTransform( THIS_ D3DTRANSFORMSTATETYPE p1,LPD3DMATRIX p2)
{
   return dx7Device->MultiplyTransform(p1,p2);
}

HRESULT RenderDevice::GetViewport( THIS_ LPD3DVIEWPORT7 p1)
{
   return dx7Device->GetViewport(p1);
}

//HRESULT RenderDevice::SetMaterial( THIS_ LPD3DMATERIAL7 p1)
//{
//   return dx7Device->SetMaterial(p1);
//}

//HRESULT RenderDevice::GetMaterial( THIS_ LPD3DMATERIAL7 p1)
//{
//   return dx7Device->GetMaterial(p1);
//}

void RenderDevice::getMaterial( THIS_ BaseMaterial *_material )
{
   dx7Device->GetMaterial((LPD3DMATERIAL7)_material);
}

HRESULT RenderDevice::SetLight( THIS_ DWORD p1,LPD3DLIGHT7 p2)
{
   return dx7Device->SetLight(p1,p2);
}

HRESULT RenderDevice::GetLight( THIS_ DWORD p1,LPD3DLIGHT7 p2 )
{
   return dx7Device->GetLight(p1,p2);
}

HRESULT RenderDevice::SetRenderState( THIS_ D3DRENDERSTATETYPE p1,DWORD p2)
{
   return dx7Device->SetRenderState(p1,p2);
}

HRESULT RenderDevice::GetRenderState( THIS_ D3DRENDERSTATETYPE p1,LPDWORD p2)
{
   return dx7Device->GetRenderState(p1,p2);
}

HRESULT RenderDevice::BeginStateBlock( THIS )
{
   return dx7Device->BeginStateBlock();
}

HRESULT RenderDevice::EndStateBlock( THIS_ LPDWORD p1)
{
   return dx7Device->EndStateBlock(p1);
}

HRESULT RenderDevice::PreLoad( THIS_ LPDIRECTDRAWSURFACE7 p1)
{
   return dx7Device->PreLoad(p1);
}

HRESULT RenderDevice::DrawPrimitive( THIS_ D3DPRIMITIVETYPE p1,DWORD p2,LPVOID p3,DWORD p4,DWORD p5)
{
   return dx7Device->DrawPrimitive(p1,p2,p3,p4,p5);
}

HRESULT RenderDevice::DrawIndexedPrimitive( THIS_ D3DPRIMITIVETYPE p1,DWORD p2,LPVOID p3,DWORD p4,LPWORD p5,DWORD p6,DWORD p7)
{
   return dx7Device->DrawIndexedPrimitive(p1,p2,p3,p4,p5,p6,p7);
}

HRESULT RenderDevice::SetClipStatus( THIS_ LPD3DCLIPSTATUS p1)
{
   return dx7Device->SetClipStatus(p1);
}

HRESULT RenderDevice::GetClipStatus( THIS_ LPD3DCLIPSTATUS p1)
{
   return dx7Device->GetClipStatus(p1);
}

HRESULT RenderDevice::DrawPrimitiveStrided( THIS_ D3DPRIMITIVETYPE p1,DWORD p2,LPD3DDRAWPRIMITIVESTRIDEDDATA p3,DWORD p4,DWORD p5)
{
   return dx7Device->DrawPrimitiveStrided(p1,p2,p3,p4,p5);
}

HRESULT RenderDevice::DrawIndexedPrimitiveStrided( THIS_ D3DPRIMITIVETYPE p1,DWORD p2,LPD3DDRAWPRIMITIVESTRIDEDDATA p3,DWORD p4,LPWORD p5,DWORD p6,DWORD p7)
{
   return dx7Device->DrawIndexedPrimitiveStrided(p1,p2,p3,p4,p5,p6,p7);
}

HRESULT RenderDevice::DrawPrimitiveVB( THIS_ D3DPRIMITIVETYPE p1,LPDIRECT3DVERTEXBUFFER7 p2,DWORD p3,DWORD p4,DWORD p5)
{
   return dx7Device->DrawPrimitiveVB(p1,p2,p3,p4,p5);
}

HRESULT RenderDevice::DrawIndexedPrimitiveVB( THIS_ D3DPRIMITIVETYPE p1,LPDIRECT3DVERTEXBUFFER7 p2,DWORD p3,DWORD p4,LPWORD p5,DWORD p6,DWORD p7)
{
   return dx7Device->DrawIndexedPrimitiveVB(p1,p2,p3,p4,p5,p6,p7);
}

HRESULT RenderDevice::ComputeSphereVisibility( THIS_ LPD3DVECTOR p1,LPD3DVALUE p2,DWORD p3,DWORD p4,LPDWORD p5)
{
   return dx7Device->ComputeSphereVisibility(p1,p2,p3,p4,p5);
}

HRESULT RenderDevice::GetTexture( THIS_ DWORD p1,LPDIRECTDRAWSURFACE7 *p2 )
{
   return dx7Device->GetTexture(p1,p2);
}

HRESULT RenderDevice::SetTexture( THIS_ DWORD p1,LPDIRECTDRAWSURFACE7 p2 )
{
   return dx7Device->SetTexture(p1,p2);
}

HRESULT RenderDevice::GetTextureStageState( THIS_ DWORD p1,D3DTEXTURESTAGESTATETYPE p2,LPDWORD p3)
{
   return dx7Device->GetTextureStageState(p1,p2,p3);
}

HRESULT RenderDevice::SetTextureStageState( THIS_ DWORD p1,D3DTEXTURESTAGESTATETYPE p2,DWORD p3)
{
   if( (unsigned int)p2 < TEXTURE_STATE_CACHE_SIZE && p1<8) 
   {
      if( textureStateCache[p1][p2]==p3 )
      {
         // texture stage state hasn't changed since last call of this function -> do nothing here
         return D3D_OK;
      }
      textureStateCache[p1][p2]=p3;
   }
   return dx7Device->SetTextureStageState(p1,p2,p3);
}

HRESULT RenderDevice::ValidateDevice( THIS_ LPDWORD p1)
{
   return dx7Device->ValidateDevice(p1);
}

HRESULT RenderDevice::ApplyStateBlock( THIS_ DWORD p1)
{
   return dx7Device->ApplyStateBlock(p1);
}

HRESULT RenderDevice::CaptureStateBlock( THIS_ DWORD p1)
{
   return dx7Device->CaptureStateBlock(p1);
}

HRESULT RenderDevice::DeleteStateBlock( THIS_ DWORD p1)
{
   return dx7Device->DeleteStateBlock(p1);
}

HRESULT RenderDevice::CreateStateBlock( THIS_ D3DSTATEBLOCKTYPE p1,LPDWORD p2)
{
   return dx7Device->CaptureStateBlock(*p2);
}

HRESULT RenderDevice::Load( THIS_ LPDIRECTDRAWSURFACE7 p1,LPPOINT p2,LPDIRECTDRAWSURFACE7 p3,LPRECT p4,DWORD p5)
{
   return dx7Device->Load(p1,p2,p3,p4,p5);
}

HRESULT RenderDevice::LightEnable( THIS_ DWORD p1,BOOL p2)
{
   return dx7Device->LightEnable(p1,p2);
}

HRESULT RenderDevice::GetLightEnable( THIS_ DWORD p1,BOOL* p2)
{
   return dx7Device->GetLightEnable(p1,p2);
}

HRESULT RenderDevice::SetClipPlane( THIS_ DWORD p1,D3DVALUE* p2)
{
   return dx7Device->SetClipPlane(p1,p2);
}

HRESULT RenderDevice::GetClipPlane( THIS_ DWORD p1,D3DVALUE* p2)
{
   return dx7Device->GetClipPlane(p1,p2);
}

HRESULT RenderDevice::GetInfo( THIS_ DWORD p1,LPVOID p2,DWORD p3 )
{
   return dx7Device->GetInfo(p1, p2, p3);
}

