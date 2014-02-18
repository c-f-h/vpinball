#include "stdafx.h"
#include "RenderDevice.h"
#include "nvapi.h"

#pragma comment(lib, "d3d9.lib")        // TODO: put into build system


void ReportError(const HRESULT hr, const char *file, const int line)
{
    char msg[128];
    sprintf_s(msg, 128, "Fatal error: HRESULT %x at %s:%d", hr, file, line);
    ShowError(msg);
    exit(-1);
}

D3DTexture* TextureManager::LoadTexture(MemTexture* memtex)
{
    Iter it = m_map.find(memtex);
    if (it == m_map.end())
    {
        TexInfo texinfo;
        texinfo.d3dtex = m_rd.UploadTexture(memtex, &texinfo.texWidth, &texinfo.texHeight);
        if (!texinfo.d3dtex)
            return 0;
        m_map[memtex] = texinfo;
        return texinfo.d3dtex;
    }
    else
    {
        return it->second.d3dtex;
    }
}

void TextureManager::UnloadTexture(MemTexture* memtex)
{
    Iter it = m_map.find(memtex);
    if (it != m_map.end())
    {
        it->second.d3dtex->Release();
        m_map.erase(it);
    }
}

void TextureManager::UnloadAll()
{
    for (Iter it = m_map.begin(); it != m_map.end(); ++it)
    {
        it->second.d3dtex->Release();
    }
    m_map.clear();
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
            return 0;
    }
}

static UINT ComputePrimitiveCount(D3DPRIMITIVETYPE type, int vertexCount)
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

void EnumerateDisplayModes(int adapter, std::vector<VideoMode>& modes)
{
    IDirect3D9 *d3d;
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (d3d == NULL)
    {
        ShowError("Could not create D3D9 object.");
        throw 0;
    }

    modes.clear();

    for (int j = 0; j < 2; ++j)
    {
        const D3DFORMAT fmt = (j == 0) ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5;
        const unsigned numModes = d3d->GetAdapterModeCount(adapter, fmt);

        for (unsigned i = 0; i < numModes; ++i)
        {
            D3DDISPLAYMODE d3dmode;
            d3d->EnumAdapterModes(adapter, fmt, i, &d3dmode);

            if (d3dmode.Width >= 640)
            {
                VideoMode mode;
                mode.width = d3dmode.Width;
                mode.height = d3dmode.Height;
                mode.depth = (fmt == D3DFMT_R5G6B5) ? 16 : 32;
                mode.refreshrate = d3dmode.RefreshRate;
                modes.push_back(mode);
            }
        }
    }

    d3d->Release();
}

////////////////////////////////////////////////////////////////////

//#define MY_IDX_BUF_SIZE 8192
#define MY_IDX_BUF_SIZE 65536

RenderDevice::RenderDevice(HWND hwnd, int width, int height, bool fullscreen, int screenWidth, int screenHeight, int colordepth, int &refreshrate, bool useAA, bool stereo3DFXAA)
    : m_texMan(*this)
{
    m_adapter = D3DADAPTER_DEFAULT;     // for now, always use the default adapter

    m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (m_pD3D == NULL)
    {
        ShowError("Could not create D3D9 object.");
        throw 0;
    }

    D3DFORMAT format;

    // get the current display format
    if (!fullscreen)
    {
        D3DDISPLAYMODE mode;
        CHECKD3D(m_pD3D->GetAdapterDisplayMode(m_adapter, &mode));
        format = mode.Format;
    }
    else
    {
        format = (colordepth == 16) ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8;
    }

    D3DPRESENT_PARAMETERS params;
    params.BackBufferWidth = fullscreen ? screenWidth : width;
    params.BackBufferHeight = fullscreen ? screenHeight : height;
    params.BackBufferFormat = format;
    params.BackBufferCount = 1;
    params.MultiSampleType = useAA ? D3DMULTISAMPLE_4_SAMPLES : D3DMULTISAMPLE_NONE;
    params.MultiSampleQuality = 0;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;  // FLIP ?
    params.hDeviceWindow = hwnd;
    params.Windowed = !fullscreen;
    params.EnableAutoDepthStencil = FALSE;
    params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;      // ignored
    params.Flags = /*stereo3DFXAA ?*/ 0 /*: D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL*/;
    params.FullScreen_RefreshRateInHz = fullscreen ? refreshrate : 0;
    params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // D3DPRESENT_INTERVAL_ONE for vsync

    D3DDEVTYPE devtype = D3DDEVTYPE_HAL;

    // Look for 'NVIDIA PerfHUD' adapter
    // If it is present, override default settings
    // This only takes effect if run under NVPerfHud, otherwise does nothing
    for (UINT adapter=0; adapter < m_pD3D->GetAdapterCount(); adapter++)
    {
        D3DADAPTER_IDENTIFIER9 Identifier;
        m_pD3D->GetAdapterIdentifier(adapter, 0, &Identifier);
        if (strstr(Identifier.Description, "PerfHUD") != 0)
        {
            m_adapter = adapter;
            devtype = D3DDEVTYPE_REF;
            break;
        }
    }

    // Create the D3D device. This optionally goes to the proper fullscreen mode.
    // It also creates the default swap chain (front and back buffer).
    CHECKD3D(m_pD3D->CreateDevice(
               m_adapter,
               devtype,
               hwnd,
               D3DCREATE_HARDWARE_VERTEXPROCESSING /*| D3DCREATE_PUREDEVICE*/,
               &params,
               &m_pD3DDevice));

    // Get the display mode so that we can report back the actual refresh rate.
    D3DDISPLAYMODE mode;
    CHECKD3D(m_pD3DDevice->GetDisplayMode(m_adapter, &mode));
    refreshrate = mode.RefreshRate;

    // Retrieve a reference to the back buffer.
    CHECKD3D(m_pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer));

    // Set up a dynamic index buffer to cache passed indices in
    CreateIndexBuffer(MY_IDX_BUF_SIZE, D3DUSAGE_DYNAMIC, IndexBuffer::FMT_INDEX16, &m_dynIndexBuffer);

    Texture::SetRenderDevice(this);

    m_curIndexBuffer = 0;
    m_curVertexBuffer = 0;

    // fill state caches with dummy values
    memset( renderStateCache, 0xCC, sizeof(DWORD)*RENDER_STATE_CACHE_SIZE);
    memset( textureStateCache, 0xCC, sizeof(DWORD)*8*TEXTURE_STATE_CACHE_SIZE);
    memset(&materialStateCache, 0xCC, sizeof(Material));
}

#define CHECKNVAPI(s) { NvAPI_Status hr = (s); if (hr != NVAPI_OK) { NvAPI_ShortString ss; NvAPI_GetErrorMessage(hr,ss); MessageBox(NULL, ss, "NVAPI", MB_OK | MB_ICONEXCLAMATION); } }
static bool NVAPIinit = false; //!! meh
static RenderTarget *src_cache = NULL; //!! meh
static D3DTexture* dest_cache = NULL;

RenderDevice::~RenderDevice()
{
	if(src_cache != NULL)
		CHECKNVAPI(NvAPI_D3D9_UnregisterResource(src_cache)); //!! meh
	src_cache = NULL;
	if(dest_cache != NULL)
		CHECKNVAPI(NvAPI_D3D9_UnregisterResource(dest_cache)); //!! meh
	dest_cache = NULL;
	if(NVAPIinit)
		CHECKNVAPI(NvAPI_Unload());
	NVAPIinit = false;

	SAFE_RELEASE(m_pBackBuffer);
    m_pD3DDevice->Release();
    m_pD3D->Release();
}

void RenderDevice::BeginScene()
{
   CHECKD3D(m_pD3DDevice->BeginScene());
}

void RenderDevice::EndScene()
{
   CHECKD3D(m_pD3DDevice->EndScene());
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
        desc.MultiSampleType, desc.MultiSampleQuality, FALSE /* lockable */, &dup, NULL));
    return dup;
}

void RenderDevice::CopySurface(RenderTarget* dest, RenderTarget* src)
{
    CHECKD3D(m_pD3DDevice->StretchRect(src, NULL, dest, NULL, D3DTEXF_NONE));
}

D3DTexture* RenderDevice::DuplicateTexture(RenderTarget* src)
{
    D3DSURFACE_DESC desc;
    src->GetDesc(&desc);
	D3DTexture* dup;
	CHECKD3D(m_pD3DDevice->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &dup, NULL));
	return dup;
}

D3DTexture* RenderDevice::DuplicateDepthTexture(RenderTarget* src)
{
    D3DSURFACE_DESC desc;
    src->GetDesc(&desc);
	D3DTexture* dup;
	CHECKD3D(m_pD3DDevice->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_DEPTHSTENCIL, (D3DFORMAT)MAKEFOURCC('I','N','T','Z'), D3DPOOL_DEFAULT, &dup, NULL));
	return dup;
}

void RenderDevice::CopySurface(D3DTexture* dest, RenderTarget* src)
{
	IDirect3DSurface9 *textureSurface;
    CHECKD3D(dest->GetSurfaceLevel(0, &textureSurface));
    CHECKD3D(m_pD3DDevice->StretchRect(src, NULL, textureSurface, NULL, D3DTEXF_NONE));
}

void RenderDevice::CopyDepth(D3DTexture* dest, RenderTarget* src)
{
	if(!NVAPIinit)
	{
		 CHECKNVAPI(NvAPI_Initialize()); //!! meh
		 NVAPIinit = true;
	}
	if(src != src_cache)
	{
		if(src_cache != NULL)
			CHECKNVAPI(NvAPI_D3D9_UnregisterResource(src_cache)); //!! meh
		CHECKNVAPI(NvAPI_D3D9_RegisterResource(src)); //!! meh
		src_cache = src;
	}
	if(dest != dest_cache)
	{
		if(dest_cache != NULL)
			CHECKNVAPI(NvAPI_D3D9_UnregisterResource(dest_cache)); //!! meh
		CHECKNVAPI(NvAPI_D3D9_RegisterResource(dest)); //!! meh
		dest_cache = dest;
	}

	//CHECKNVAPI(NvAPI_D3D9_AliasSurfaceAsTexture(m_pD3DDevice,src,dest,0));
	CHECKNVAPI(NvAPI_D3D9_StretchRectEx(m_pD3DDevice, src, NULL, dest, NULL, D3DTEXF_NONE));
}

D3DTexture* RenderDevice::UploadTexture(MemTexture* surf, int *pTexWidth, int *pTexHeight)
{
    IDirect3DTexture9 *sysTex, *tex;

    int texwidth = surf->width();
    int texheight = surf->height();

    if (pTexWidth) *pTexWidth = texwidth;
    if (pTexHeight) *pTexHeight = texheight;

    CHECKD3D(m_pD3DDevice->CreateTexture(texwidth, texheight, 1, 0, D3DFMT_A8R8G8B8,
                D3DPOOL_SYSTEMMEM, &sysTex, NULL));

    // copy data into system memory texture
    D3DLOCKED_RECT locked;
    CHECKD3D(sysTex->LockRect(0, &locked, NULL, 0));
    BYTE *pdest = (BYTE*)locked.pBits;
    for (int y = 0; y < surf->height(); ++y)
    {
        memcpy(pdest + y*locked.Pitch, surf->data() + y*surf->pitch(), 4 * surf->width());
    }
    CHECKD3D(sysTex->UnlockRect(0));

    CHECKD3D(m_pD3DDevice->CreateTexture(texwidth, texheight, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8,
                D3DPOOL_DEFAULT, &tex, NULL));

    CHECKD3D(m_pD3DDevice->UpdateTexture(sysTex, tex));
    CHECKD3D(sysTex->Release());

    return tex;
}

void RenderDevice::SetTexture(DWORD p1, D3DTexture* p2 )
{
    CHECKD3D(m_pD3DDevice->SetTexture(p1, p2));
}

void RenderDevice::SetTextureFilter(DWORD texUnit, DWORD mode)
{
	switch ( mode )
	{
	default:
	case TEXTURE_MODE_POINT:
		// Don't filter textures.
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_POINT));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
		break;

	case TEXTURE_MODE_BILINEAR:
		// Filter textures (average of 2x2 texels).
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
		break;

	case TEXTURE_MODE_TRILINEAR:
		// Filter textures on 2 mip levels (average of 2x2 texels).  And filter between the 2 mip levels.
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
		break;

	case TEXTURE_MODE_ANISOTROPIC:
		// Full HQ anisotropic Filter. Should lead to driver doing whatever it thinks is best.
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC));
		CHECKD3D(m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_MAXANISOTROPY,16));
		break;
	}
}

void RenderDevice::SetTextureStageState( DWORD p1, D3DTEXTURESTAGESTATETYPE p2, DWORD p3)
{
    if( (unsigned int)p2 < TEXTURE_STATE_CACHE_SIZE && p1 < 8)
    {
        if(textureStateCache[p1][p2] == p3)
        {
            // texture stage state hasn't changed since last call of this function -> do nothing here
            return;
        }
        textureStateCache[p1][p2] = p3;
    }
    CHECKD3D(m_pD3DDevice->SetTextureStageState(p1, p2, p3));
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

    CHECKD3D(m_pD3DDevice->SetMaterial((D3DMATERIAL9*)_material));
}


void RenderDevice::SetRenderTarget( RenderTarget* surf)
{
    CHECKD3D(m_pD3DDevice->SetRenderTarget(0, surf));
}

void RenderDevice::SetZBuffer( RenderTarget* surf)
{
    CHECKD3D(m_pD3DDevice->SetDepthStencilSurface(surf));
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
   CHECKD3D(m_pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)p1, p2));
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
    CHECKD3D(m_pD3DDevice->CreateVertexBuffer(vertexCount * fvfToSize(fvf), D3DUSAGE_WRITEONLY | usage, fvf,
                D3DPOOL_DEFAULT, (IDirect3DVertexBuffer9**)vBuffer, NULL));
}

void RenderDevice::CreateIndexBuffer(unsigned int numIndices, DWORD usage, IndexBuffer::Format format, IndexBuffer **idxBuffer)
{
    // NB: We always specify WRITEONLY since MSDN states,
    // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
    const unsigned idxSize = (format == IndexBuffer::FMT_INDEX16) ? 2 : 4;
    CHECKD3D(m_pD3DDevice->CreateIndexBuffer(idxSize * numIndices, usage | D3DUSAGE_WRITEONLY, (D3DFORMAT)format,
                D3DPOOL_DEFAULT, (IDirect3DIndexBuffer9**)idxBuffer, NULL));
}

IndexBuffer* RenderDevice::CreateAndFillIndexBuffer(unsigned int numIndices, const WORD * indices)
{
    IndexBuffer* ib;
    CreateIndexBuffer(numIndices, 0, IndexBuffer::FMT_INDEX16, &ib);

    void* buf;
    ib->lock(0, 0, &buf, 0);
    memcpy(buf, indices, numIndices * sizeof(indices[0]));
    ib->unlock();

    return ib;
}

IndexBuffer* RenderDevice::CreateAndFillIndexBuffer(const std::vector<WORD>& indices)
{
    return CreateAndFillIndexBuffer(indices.size(), &indices[0]);
}

RenderTarget* RenderDevice::AttachZBufferTo(RenderTarget* surf)
{
    D3DSURFACE_DESC desc;
    surf->GetDesc(&desc);

    IDirect3DSurface9 *pZBuf;
    CHECKD3D(m_pD3DDevice->CreateDepthStencilSurface(desc.Width, desc.Height, D3DFMT_D16 /*D3DFMT_D24X8*/,
            desc.MultiSampleType, desc.MultiSampleQuality, FALSE, &pZBuf, NULL));

    return pZBuf;
}


void RenderDevice::DrawPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, LPVOID vertices, DWORD vertexCount)
{
    m_pD3DDevice->SetFVF(fvf);
    CHECKD3D(m_pD3DDevice->DrawPrimitiveUP(type, ComputePrimitiveCount(type, vertexCount), vertices, fvfToSize(fvf)));
    m_curVertexBuffer = 0;      // DrawPrimitiveUP sets the VB to NULL
}

void RenderDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, LPVOID vertices, DWORD vertexCount, LPWORD indices, DWORD indexCount)
{
    m_pD3DDevice->SetFVF(fvf);
    CHECKD3D(m_pD3DDevice->DrawIndexedPrimitiveUP(type, 0, vertexCount, ComputePrimitiveCount(type, indexCount),
                indices, D3DFMT_INDEX16, vertices, fvfToSize(fvf)));
    m_curVertexBuffer = 0;      // DrawIndexedPrimitiveUP sets the VB to NULL
    m_curIndexBuffer = 0;       // DrawIndexedPrimitiveUP sets the IB to NULL
}

void RenderDevice::DrawPrimitiveVB(D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount)
{
    D3DVERTEXBUFFER_DESC desc;
    vb->GetDesc(&desc);     // let's hope this is not very slow

    const unsigned int vsize = fvfToSize(desc.FVF);

    CHECKD3D(m_pD3DDevice->SetFVF(desc.FVF));

    if (m_curVertexBuffer != vb)
    {
        CHECKD3D(m_pD3DDevice->SetStreamSource(0, vb, 0, vsize));
        m_curVertexBuffer = vb;
    }

    CHECKD3D(m_pD3DDevice->DrawPrimitive(type, startVertex, ComputePrimitiveCount(type, vertexCount)));
}

void RenderDevice::DrawIndexedPrimitiveVB( D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount, LPWORD indices, DWORD indexCount)
{
    if (indexCount > MY_IDX_BUF_SIZE)
    {
        ShowError("Call to DrawIndexedPrimitiveVB has too many indices. Use an index buffer.");
        return;
    }

    // copy the indices to the dynamic index buffer
    WORD *buffer;
    m_dynIndexBuffer->lock(0, indexCount * sizeof(WORD), (void**)&buffer, IndexBuffer::DISCARD);
    memcpy(buffer, indices, indexCount * sizeof(WORD));
    m_dynIndexBuffer->unlock();

    DrawIndexedPrimitiveVB(type, vb, startVertex, vertexCount, m_dynIndexBuffer, 0, indexCount);
}

void RenderDevice::DrawIndexedPrimitiveVB( D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount, IndexBuffer* ib, DWORD startIndex, DWORD indexCount)
{
    // get VB description (for FVF)
    D3DVERTEXBUFFER_DESC desc;
    vb->GetDesc(&desc);     // let's hope this is not very slow

    const unsigned int vsize = fvfToSize(desc.FVF);

    // bind the vertex and index buffers
    CHECKD3D(m_pD3DDevice->SetFVF(desc.FVF));

    if (m_curVertexBuffer != vb)
    {
        CHECKD3D(m_pD3DDevice->SetStreamSource(0, vb, 0, vsize));
        m_curVertexBuffer = vb;
    }

    if (m_curIndexBuffer != ib)
    {
        CHECKD3D(m_pD3DDevice->SetIndices(ib));
        m_curIndexBuffer = ib;
    }

    // render
    CHECKD3D(m_pD3DDevice->DrawIndexedPrimitive(type, startVertex, 0, vertexCount, startIndex, ComputePrimitiveCount(type, indexCount)));
}

void RenderDevice::SetTransform( TransformStateType p1, D3DMATRIX* p2)
{
   CHECKD3D(m_pD3DDevice->SetTransform((D3DTRANSFORMSTATETYPE)p1, p2));
}

void RenderDevice::GetTransform( TransformStateType p1, D3DMATRIX* p2)
{
   CHECKD3D(m_pD3DDevice->GetTransform((D3DTRANSFORMSTATETYPE)p1, p2));
}


void RenderDevice::GetMaterial( BaseMaterial *_material )
{
   m_pD3DDevice->GetMaterial((D3DMATERIAL9*)_material);
}

void RenderDevice::SetLight( DWORD p1, BaseLight* p2)
{
   m_pD3DDevice->SetLight(p1,p2);
}

void RenderDevice::GetLight( DWORD p1, BaseLight* p2 )
{
   m_pD3DDevice->GetLight(p1,p2);
}

void RenderDevice::LightEnable( DWORD p1, BOOL p2)
{
   m_pD3DDevice->LightEnable(p1,p2);
}

void RenderDevice::Clear(DWORD numRects, D3DRECT* rects, DWORD flags, D3DCOLOR color, D3DVALUE z, DWORD stencil)
{
   m_pD3DDevice->Clear(numRects, rects, flags, color, z, stencil);
}

void RenderDevice::SetViewport( ViewPort* p1)
{
   m_pD3DDevice->SetViewport((D3DVIEWPORT9*)p1);
}

void RenderDevice::GetViewport( ViewPort* p1)
{
   m_pD3DDevice->GetViewport((D3DVIEWPORT9*)p1);
}

