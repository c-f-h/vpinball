#include "RenderDevice_dx9.h"


static void ReportError(HRESULT hr, const char *file, int line)
{
    char msg[128];
    sprintf(msg, "Fatal error: HRESULT %x at %s:%d", hr, file, line);
    ShowError(msg);
    exit(-1);
}

#define CHECKD3D(s) { HRESULT hr = (s); if (FAILED(hr)) ReportError(hr, __FILE__, __LINE__); }


static unsigned int fvfToSize(DWORD fvf)
{
    switch (fvf)
    {
        case MY_D3DFVF_VERTEX:
        case MY_D3DTRANSFORMED_VERTEX:
            return sizeof(Vertex3D);
        case MY_D3DFVF_NOLIGHTING_VERTEX:
            return sizeof(Vertex3D_NoLighting);
        case MY_D3DFVF_NOTEX2_VERTEX:
        case MY_D3DTRANSFORMED_NOTEX2_VERTEX:
            return sizeof(Vertex3D_NoTex2);
        case MY_D3DFVF_NOTEX_VERTEX:
        case MY_D3DTRANSFORMED_NOTEX_VERTEX:
            return sizeof(Vertex3D_NoTex);
        default:
            assert(0 && "Unknown FVF type in fvfToSize");
    }
}

static UINT ComputePrimitiveCount(D3DPRIMITIVETYPE type, DWORD vertexCount)
{
    switch (type)
    {
        case D3DPT_POINTLIST:
            return vertexCount;
        case D3DPT_LINELIST:
            return vertexCount / 2;
        case D3DPT_LINESTRIP:
            return std::max(0, vertexCount - 1);
        case D3DPT_TRIANGLELIST:
            return vertexCount / 3;
        case D3DPT_TRIANGLESTRIP:
        case D3DPT_TRIANGLEFAN:
            return std::max(0, vertexCount - 2);
        default:
            return 0;
    }
}

////////////////////////////////////////////////////////////////////

RenderDevice::RenderDevice()
{
    m_pD3D = NULL;
    m_pD3DDevice = NULL;
    m_pBackBuffer = NULL;

    m_adapter = D3DADAPTER_DEFAULT;     // for now, always use the default adapter
}

bool RenderDevice::InitRenderer(HWND hwnd, int width, int height, bool fullscreen, int screenWidth, int screenHeight, int colordepth, int &refreshrate)
{
    m_pD3D = Direct3DCreate(D3D_SDK_VERSION);
    if (m_pD3D == NULL)
    {
        ShowError("Could not create D3D9 object.");
        return false;
    }

    D3DPRESENT_PARAMETERS params;
    params.BackBufferWidth = fullscreen ? screenWidth : width;
    params.BackBufferHeight = fullscreen ? screenHeight : height;
    params.BackBufferFormat = (colordepth == 32) ? D3DFMT_R8G8B8 : D3DFMT_R5G6B5;
    params.BackBufferCount = 1;
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.MultiSampleQuality = 0;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;  // FLIP ?
    params.hDeviceWindow = hwnd;
    params.Windowed = !fullscreen;
    params.EnableAutoDepthStencil = FALSE;
    params.AutoDepthStencilFormat = 0;      // ignored
    params.Flags = 0;
    params.FullScreen_RefreshRateInHz = fullscreen ? refreshrate : 0;
    params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // D3DPRESENT_INTERVAL_ONE for vsync

    // Create the D3D device. This optionally goes to the proper fullscreen mode.
    // It also creates the default swap chain (front and back buffer).
    CHECKD3D(m_pD3D->CreateDevice(
               m_adapter,
               D3DDEVTYPE_HAL,
               hwnd,
               D3DCREATE_HARDWARE_VERTEXPROCESSING,
               &params
               &m_pD3DDevice));

    // Get the display mode so that we can report back the actual refresh rate.
    D3DDISPLAYMODE mode;
    CHECKD3D(m_pD3DDevice->GetDisplayMode(0, &mode));
    refreshrate = mode.RefreshRate;

    // Retrieve a reference to the back buffer.
    CHECKD3D(m_pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer));
}

void RenderDevice::DestroyRenderer()
{
    SAFE_RELEASE(m_pBackBuffer);
    SAFE_RELEASE(m_pD3DDevice);
    SAFE_RELEASE(m_pD3D);
}

RenderTarget* RenderDevice::GetBackBuffer()
{
    return m_pBackBuffer;
}

void RenderDevice::Flip(int offsetx, int offsety, bool vsync)
{
    // TODO: we can't handle shake or vsync here
    // (vsync should be set when creating the device)
    CHECKD3D(m_pD3DDevice->Present(NULL, NULL, NULL, NULL));
}

RenderTarget* RenderDevice::DuplicateRenderTarget(RenderTarget* src)
{
    D3DSURFACE_DESC desc;
    src->GetDesc(&desc);
    IDirect3DSurface9 *dup;
    CHECKD3D(m_pD3DDevice->CreateRenderTarget(desc.Width, desc.Height, desc.Format,
        desc.MultiSample, desc.MultisampleQuality, FALSE /* lockable */, &dup, NULL));
    return dup;
}

void RenderDevice::CopySurface(RenderTarget* dest, RenderTarget* src)
{
    CHECKD3D(m_pD3DDevice->StretchRect(src, NULL, dest, NULL, D3DTEXF_NONE));
}

void RenderDevice::GetTextureSize(BaseTexture* tex, DWORD *width, DWORD *height)
{
    D3DSURFACE_DESC desc;
    tex->GetLevelDesc(0, &desc);
    *width = desc.Width;
    *height = desc.Height;
}

void RenderDevice::SetTextureFilter(DWORD texUnit, DWORD mode)
{
	switch ( mode )
	{
	case TEXTURE_MODE_POINT: 
		// Don't filter textures.  Don't filter between mip levels.
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_POINT);		
		break;

	case TEXTURE_MODE_BILINEAR:
		// Filter textures when magnified or reduced (average of 2x2 texels).  Don't filter between mip levels.
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		break;

	case TEXTURE_MODE_TRILINEAR:
		// Filter textures when magnified or reduced (average of 2x2 texels).  And filter between the 2 mip levels.
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		break;

	case TEXTURE_MODE_ANISOTROPIC:
		// Filter textures when magnified or reduced (filter to account for perspective distortion).  And filter between the 2 mip levels.
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		break;

	default:
		// Don't filter textures.  Don't filter between mip levels.
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		break;
	}
}

void RenderDevice::SetMaterial( const BaseMaterial * const _material )
{
#if !defined(DEBUG_XXX) && !defined(_CRTDBG_MAP_ALLOC)
    // this produces a crash if BaseMaterial isn't proper aligned to 16byte (in vbatest.cpp new/delete is overloaded for that)
    if((_mm_movemask_ps(_mm_and_ps(
        _mm_and_ps(_mm_cmpeq_ps(_material->d,materialStateCache.d),_mm_cmpeq_ps(_material->a,materialStateCache.a)),
        _mm_and_ps(_mm_cmpeq_ps(_material->s,materialStateCache.s),_mm_cmpeq_ps(_material->e,materialStateCache.e)))) == 15)
            &&
            (_material->power == materialStateCache.power))
        return;

    materialStateCache.d = _material->d;
    materialStateCache.a = _material->a;
    materialStateCache.e = _material->e;
    materialStateCache.s = _material->s;
    materialStateCache.power = _material->power;
#endif

    CHECKD3D(m_pD3DDevice->SetMaterial((LPD3DMATERIAL9)_material));
}


void RenderDevice::SetRenderTarget( RenderTarget* surf)
{
    CHECKD3D(m_pD3DDevice->SetRenderTarget(0, surf));
}

void RenderDevice::SetZBuffer( RenderTarget* surf)
{
    CHECKD3D(m_pD3DDevice->SetDepthStencilSurface(surf));
}

void RenderDevice::SetTextureAddressMode(DWORD texUnit, TextureAddressMode mode)
{
    CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_ADDRESSU, mode));
    CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_ADDRESSV, mode));
}

void RenderDevice::CreateVertexBuffer( unsigned int vertexCount, DWORD usage, DWORD fvf, VertexBuffer **vBuffer )
{
    // NB: We always specify WRITEONLY since MSDN states,
    // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
    // This means we cannot read from vertex buffers, but I don't think we need to.
    CHECKD3D(m_pD3DDevice->CreateVertexBuffer(vertexCount * fvfToSize(fvf), D3DUSAGE_WRITEONLY | usage, fvf, D3DPOOL_DEFAULT, vBuffer));
}


RenderTarget* RenderDevice::AttachZBufferTo(RenderTarget* surf)
{
    D3DSURFACE_DESC desc;
    surf->GetDesc(&desc);

    IDirect3DSurface9 *pZBuf;
    CHECKD3D(m_pD3DDevice->CreateDepthStencilSurface(desc.Width, desc.Height, D3DFORMAT_D16,
            desc.MultiSampleType, desc.MultiSampleQuality, FALSE, &pZBuf, NULL));

    return pZBuf;
}


void RenderDevice::DrawPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, LPVOID vertices, DWORD vertexCount)
{
    m_pd3dDevice->SetFVF(fvf);
    CHECKD3D(m_pD3DDevice->DrawPrimitiveUP(type, ComputePrimitiveCount(type, vertexCount), vertices, fvfToSize(fvf)));
}

void RenderDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, LPVOID vertices, DWORD vertexCount, LPWORD indices, DWORD indexCount)
{
    m_pD3DDevice->SetFVF(fvf);
    CHECKD3D(m_pD3DDevice->DrawIndexedPrimitiveUP(type, 0, vertexCount, ComputePrimitiveCount(type, vertexCount),
                indices, D3DFMT_INDEX32, vertices, fvfToSize(fvf)));
}

void RenderDevice::DrawPrimitiveVB(D3DPRIMITIVETYPE type, LPDIRECT3DVERTEXBUFFER7 vb, DWORD startVertex, DWORD vertexCount)
{
    D3DVERTEXBUFFER_DESC desc;
    vb->GetDesc(&desc);     // let's hope this is not very slow

    const unsigned int vsize = fvfToSize(desc.FVF);

    m_pD3DDevice->SetFVF(desc.FVF);
    m_pD3DDevice->SetStreamSource(0, vb, 0, vsize);
    CHECKD3D(m_pD3DDevice->DrawPrimitive(type, startVertex, ComputePrimitiveCount(type, vertexCount)));
}

void RenderDevice::DrawIndexedPrimitiveVB( D3DPRIMITIVETYPE type, LPDIRECT3DVERTEXBUFFER7 vb, DWORD startVertex, DWORD vertexCount, LPWORD indices, DWORD indexCount)
{
    // TODO: no such draw call in DX9, we need an index buffer
}

void RenderDevice::SetTransform( TransformStateType p1, LPD3DMATRIX p2)
{
   CHECKD3D(m_pD3DDevice->SetTransform((D3DTRANSFORMSTATETYPE)p1, p2));
}

void RenderDevice::GetTransform( TransformStateType p1, LPD3DMATRIX p2)
{
   CHECKD3D(m_pD3DDevice->GetTransform((D3DTRANSFORMSTATETYPE)p1, p2));
}


void RenderDevice::GetMaterial( BaseMaterial *_material )
{
   m_pD3DDevice->GetMaterial((LPD3DMATERIAL7)_material);
}

void RenderDevice::SetLight( DWORD p1, BaseLight* p2)
{
   m_pD3DDevice->SetLight(p1,p2);
}

void RenderDevice::GetLight( DWORD p1, BaseLight* p2 )
{
   m_pD3DDevice->GetLight(p1,p2);
}

