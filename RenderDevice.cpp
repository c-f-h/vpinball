#include "stdafx.h"
#include "RenderDevice.h"
#include "Material.h"
#include "Texture.h"

bool RenderDevice::CreateDevice(BaseTexture *_backBuffer )
{
    memset( renderStateCache, 0xFFFFFFFF, sizeof(DWORD)*RENDER_STATE_CACHE_SIZE);
    for( int i=0;i<8;i++ )
        for( unsigned int j=0;j<TEXTURE_STATE_CACHE_SIZE;j++ )
            textureStateCache[i][j]=0xFFFFFFFF;
    memset(&materialStateCache, 0xFFFFFFFF, sizeof(Material));

    HRESULT hr;
    const GUID* pDeviceGUID = (g_pvp->m_pdd.m_fHardwareAccel) ? &IID_IDirect3DHALDevice : &IID_IDirect3DRGBDevice;
    if( FAILED( hr = m_pD3D->CreateDevice( *pDeviceGUID, (LPDIRECTDRAWSURFACE7)_backBuffer, &dx7Device ) ) )
    {
        return false;
    }
    return true;
}

bool RenderDevice::InitRenderer(HWND hwnd, int width, int height, bool fullscreen, int screenWidth, int screenHeight, int colordepth, int &refreshrate)
{
    LPDIRECTDRAW7 dd = g_pvp->m_pdd.m_pDD;

    HRESULT hr = dd->QueryInterface( IID_IDirect3D7, (VOID**)&m_pD3D );
    if (hr != S_OK)
    {
        ShowError("Could not create Direct 3D.");
        return false;
    }

    hr = dd->SetCooperativeLevel(hwnd, DDSCL_FPUSETUP); // was DDSCL_FPUPRESERVE, which in theory adds lots of overhead, but who knows if this is even supported nowadays by the drivers

    if (fullscreen)
    {
        //hr = dd->SetCooperativeLevel(hwnd, DDSCL_ALLOWREBOOT|DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_FPUSETUP/*DDSCL_FPUPRESERVE*/);
        hr = dd->SetDisplayMode(screenWidth, screenHeight, colordepth, refreshrate, 0);
        DDSURFACEDESC2 ddsd;
        ddsd.dwSize = sizeof(ddsd);
        hr = dd->GetDisplayMode(&ddsd);
        refreshrate = ddsd.dwRefreshRate;
        if(FAILED(hr) || (refreshrate <= 0))
            refreshrate = 60; // meh, hardcode to 60Hz if fail
    }

    // Create the primary surface
    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize         = sizeof(ddsd);
    ddsd.dwFlags        = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    if( FAILED( hr = dd->CreateSurface( &ddsd, (LPDIRECTDRAWSURFACE7*)&m_pddsFrontBuffer, NULL ) ) )
    {
        ShowError("Could not create front buffer.");
        return false;
    }

    // Update the count.
    NumVideoBytes += width * height * 4;
    //if ( !fullscreen )
    {
        // If in windowed-mode, create a clipper object //!! but it's always needed for whatever reason to get rid of initial rendering artifacts on some systems
        LPDIRECTDRAWCLIPPER pcClipper;
        if( FAILED( hr = dd->CreateClipper( 0, &pcClipper, NULL ) ) )
        {
            ShowError("Could not create clipper.");
            return false;
        }

        if (pcClipper)
        {
            // Associate the clipper with the window.
            pcClipper->SetHWnd( 0, hwnd );
            m_pddsFrontBuffer->SetClipper( pcClipper );
            pcClipper->Release();
        }
    }

    // Define a backbuffer.
    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.dwWidth        = width;
    ddsd.dwHeight       = height;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;

    // Check if we are rendering in hardware.
    if (g_pvp->m_pdd.m_fHardwareAccel)
        ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY; // slower(?): | DDSCAPS_LOCALVIDMEM;
    else
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

    DDSURFACEDESC2 ddsdPrimary; // descriptor for current screen format
    // Check for 8-bit color
    ddsdPrimary.dwSize = sizeof(DDSURFACEDESC2);
    dd->GetDisplayMode( &ddsdPrimary );
    if( ddsdPrimary.ddpfPixelFormat.dwRGBBitCount <= 8 )
    {
        /*ddsd.dwFlags |= DDSD_PIXELFORMAT;
          ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
          ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
          ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
          ddsd.ddpfPixelFormat.dwRBitMask = 0x007c00;
          ddsd.ddpfPixelFormat.dwGBitMask = 0x0003e0;
          ddsd.ddpfPixelFormat.dwBBitMask = 0x00001f;
          ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0;*/

        ShowError("Color depth must be 16 bit or greater.");
        return false;
    }

    // Create the back buffer.
retry2:
    if( FAILED( hr = dd->CreateSurface( &ddsd, (LPDIRECTDRAWSURFACE7*)&m_pddsBackBuffer, NULL ) ) )
    {
        if((ddsd.ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) == 0) {
            ddsd.ddsCaps.dwCaps |= DDSCAPS_NONLOCALVIDMEM;
            goto retry2;
        }
        ShowError("Could not create back buffer.");
        return false;
    }

    // Update the count.
    NumVideoBytes += ddsd.dwWidth * ddsd.dwHeight * (ddsd.ddpfPixelFormat.dwRGBBitCount/8);

    // Create the D3D device.  This device will pre-render everything once...
    // Then it will render only the ball and its shadow in real time.

	if( !CreateDevice(m_pddsBackBuffer) )
	{
		ShowError("Could not create Direct 3D device.");
		return false;
	}

	return true;

}

void RenderDevice::Flip(int offsetx, int offsety, bool vsync)
{
	RECT rcNew;
	// Set the region to the entire screen dimensions.
	rcNew.left = m_rcUpdate.left + offsetx;
	rcNew.right = m_rcUpdate.right + offsetx;
	rcNew.top = m_rcUpdate.top + offsety;
	rcNew.bottom = m_rcUpdate.bottom + offsety;

	// Set blt effects
	DDBLTFX ddbltfx;
	ZeroMemory(&ddbltfx, sizeof(DDBLTFX));
	ddbltfx.dwSize = sizeof(DDBLTFX);
	//if(vsync)
	//    ddbltfx.dwDDFX = DDBLTFX_NOTEARING; // deprecated for win2000 and above?!

	// Check if we are mirrored.
	if ( g_pplayer->m_ptable->m_tblMirrorEnabled )
		// Mirror the table.
		ddbltfx.dwDDFX |= DDBLTFX_MIRRORUPDOWN;

	if(vsync)
         g_pvp->m_pdd.m_pDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);

	// Copy the back buffer to the front buffer.
	HRESULT hr;
	hr = m_pddsFrontBuffer->Blt(&rcNew,
		// TODO: (DX9) Stereo3D disabled // ((((g_pplayer->m_fStereo3D != 0) && g_pplayer->m_fStereo3Denabled) || (g_pplayer->m_fFXAA)) && (m_maxSeparation > 0.0f) && (m_maxSeparation < 1.0f) && (m_ZPD > 0.0f) && (m_ZPD < 1.0f) && m_pdds3Dbuffercopy && m_pdds3DBackBuffer) ? m_pdds3DBackBuffer : 
		m_pddsBackBuffer, NULL, ddbltfx.dwDDFX ? DDBLT_DDFX : 0, &ddbltfx);

	if (hr == DDERR_SURFACELOST)
		hr = g_pvp->m_pdd.m_pDD->RestoreAllSurfaces();
}

void RenderDevice::DestroyRenderer()
{
	g_pvp->m_pdd.m_pDD->RestoreDisplayMode();

	SAFE_RELEASE(m_pddsFrontBuffer);
	SAFE_RELEASE(m_pddsBackBuffer);
	SAFE_RELEASE(m_pD3D);
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

void RenderDevice::SetTextureAddressMode(DWORD texUnit, TextureAddressMode mode)
{
    dx7Device->SetTextureStageState(texUnit, D3DTSS_ADDRESS, mode);
}


void RenderDevice::createVertexBuffer( unsigned int _length, DWORD _usage, DWORD _fvf, VertexBuffer **_vBuffer )
{
   D3DVERTEXBUFFERDESC vbd;
   vbd.dwSize = sizeof(vbd);
   if( vbInVRAM )
      vbd.dwCaps = 0; // set nothing to let the driver decide what's best for us ;)
   else
      vbd.dwCaps = D3DVBCAPS_WRITEONLY | D3DVBCAPS_SYSTEMMEMORY; // essential on some setups to enforce this variant

   vbd.dwFVF = _fvf;
   vbd.dwNumVertices = _length;
   m_pD3D->CreateVertexBuffer(&vbd, (LPDIRECT3DVERTEXBUFFER7*)_vBuffer, 0);  //!! Later-on/DX9: CreateIndexBuffer (and release/realloc them??)
}

void RenderDevice::renderPrimitive(D3DPRIMITIVETYPE _primType, VertexBuffer* _vbuffer, DWORD _startVertex, DWORD _numVertices, LPWORD _indices, DWORD _numIndices, DWORD _flags)
{
   dx7Device->DrawIndexedPrimitiveVB( _primType, (LPDIRECT3DVERTEXBUFFER7)_vbuffer, _startVertex, _numVertices, _indices, _numIndices, _flags );
}

void RenderDevice::renderPrimitiveListed(D3DPRIMITIVETYPE _primType, VertexBuffer* _vbuffer, DWORD _startVertex, DWORD _numVertices, DWORD _flags)
{
   dx7Device->DrawPrimitiveVB( _primType, (LPDIRECT3DVERTEXBUFFER7)_vbuffer, _startVertex, _numVertices, _flags );
}


static HRESULT WINAPI EnumZBufferFormatsCallback( DDPIXELFORMAT * pddpf,
	VOID * pContext )
{
	DDPIXELFORMAT * const pddpfOut = (DDPIXELFORMAT*)pContext;

	if((pddpf->dwRGBBitCount > 0) && (pddpfOut->dwRGBBitCount == pddpf->dwRGBBitCount))
	{
		(*pddpfOut) = (*pddpf);
		return D3DENUMRET_CANCEL;
	}

	return D3DENUMRET_OK;
}

RenderTarget* RenderDevice::AttachZBufferTo(RenderTarget* surf)
{
	// Get z-buffer dimensions from the render target
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	surf->GetSurfaceDesc( &ddsd );

	// Setup the surface desc for the z-buffer.
	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
	ddsd.ddpfPixelFormat.dwSize = 0;  // Tag the pixel format as unitialized

	// Check if we are rendering in software.
	if (!g_pvp->m_pdd.m_fHardwareAccel)
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	else
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY; // slower(?): | DDSCAPS_LOCALVIDMEM;

	const GUID* pDeviceGUID = (g_pvp->m_pdd.m_fHardwareAccel) ? &IID_IDirect3DHALDevice : &IID_IDirect3DRGBDevice;

	// Get an appropriate pixel format from enumeration of the formats. On the
	// first pass, we look for a zbuffer depth which is equal to the frame
	// buffer depth (as some cards unfornately require this).
	m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback,
		(VOID*)&ddsd.ddpfPixelFormat );
	if( 0 == ddsd.ddpfPixelFormat.dwSize )
	{
		// Try again, just accepting any 16-bit zbuffer
		ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
		m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback,
			(VOID*)&ddsd.ddpfPixelFormat );

		if( 0 == ddsd.ddpfPixelFormat.dwSize )
		{
			ShowError("Could not find Z-Buffer format.");
			return NULL;
		}
	}

	// Create the z buffer.
    LPDIRECTDRAWSURFACE7 pZBuf;
retry6:
	HRESULT hr;
	if( FAILED( hr = g_pvp->m_pdd.m_pDD->CreateSurface( &ddsd, &pZBuf, NULL ) ) )
	{
		if((ddsd.ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) == 0) {
			ddsd.ddsCaps.dwCaps |= DDSCAPS_NONLOCALVIDMEM;
			goto retry6;
		}
		ShowError("Could not create Z-Buffer.");
		return NULL;
	}

	// Attach the z buffer to the back buffer.
	if( FAILED( hr = surf->AddAttachedSurface( pZBuf ) ) )
	{
		ShowError("Could not attach Z-Buffer.");
		return NULL;
	}

    return pZBuf;
}

void RenderDevice::SetRenderTarget( RenderTarget* surf)
{
    dx7Device->SetRenderTarget(surf, 0);
}

void RenderDevice::SetZBuffer( RenderTarget* surf)
{
    dx7Device->SetRenderTarget(surf, 0);
}



//########################## simple wrapper functions (interface for DX7)##################################

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

   void SetZBuffer( RenderTarget* );

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

HRESULT RenderDevice::SetLight( THIS_ DWORD p1, BaseLight* p2)
{
   return dx7Device->SetLight(p1,p2);
}

HRESULT RenderDevice::GetLight( THIS_ DWORD p1, BaseLight* p2 )
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

//HRESULT RenderDevice::BeginStateBlock( THIS )
//{
//   return dx7Device->BeginStateBlock();
//}

//HRESULT RenderDevice::EndStateBlock( THIS_ LPDWORD p1)
//{
//   return dx7Device->EndStateBlock(p1);
//}

HRESULT RenderDevice::DrawPrimitive( THIS_ D3DPRIMITIVETYPE type,DWORD fvf,LPVOID vertices,DWORD vertexCount,DWORD flags)
{
   return dx7Device->DrawPrimitive(type, fvf, vertices, vertexCount, flags);
}

HRESULT RenderDevice::DrawIndexedPrimitive( THIS_ D3DPRIMITIVETYPE type, DWORD fvf, LPVOID vertices, DWORD vertexCount, LPWORD indices, DWORD indexCount, DWORD flags)
{
   return dx7Device->DrawIndexedPrimitive(type, fvf, vertices, vertexCount, indices, indexCount, flags);
}

//HRESULT RenderDevice::SetClipStatus( THIS_ LPD3DCLIPSTATUS p1)
//{
//   return dx7Device->SetClipStatus(p1);
//}

//HRESULT RenderDevice::GetClipStatus( THIS_ LPD3DCLIPSTATUS p1)
//{
//   return dx7Device->GetClipStatus(p1);
//}

HRESULT RenderDevice::DrawPrimitiveVB( THIS_ D3DPRIMITIVETYPE type, LPDIRECT3DVERTEXBUFFER7 vb, DWORD startVertex, DWORD numVertices, DWORD flags)
{
   return dx7Device->DrawPrimitiveVB(type, vb, startVertex, numVertices, flags);
}

HRESULT RenderDevice::DrawIndexedPrimitiveVB( THIS_ D3DPRIMITIVETYPE type,LPDIRECT3DVERTEXBUFFER7 vb,DWORD startVertex,DWORD numVertices,LPWORD indices,DWORD indexCount,DWORD flags)
{
   return dx7Device->DrawIndexedPrimitiveVB(type,vb,startVertex,numVertices,indices,indexCount,flags);
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

