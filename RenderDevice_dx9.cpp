#include <d3d9.h>

typedef IDirect3dTexture9 BaseTexture;
typedef D3DVIEWPORT9 ViewPort;
typedef IDirect3DSurface9 RenderTarget;


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


private:
   IDirect3D9* m_pD3D;
   IDirect3DDevice9* m_pD3DDevice;
   IDirect3DSurface9* m_pBackBuffer;

   UINT m_adapter;      // index of the display adapter to use
};

#define CHECKD3D(s) { HRESULT hr = (s); if (FAILED(hr)) ReportError(hr, __FILE__, __LINE__); }

unsigned int fvfToSize(DWORD fvf)
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

void ReportError(HRESULT hr, const char *file, int line)
{
    char msg[128];
    sprintf(msg, "Fatal error: HRESULT %x at %s:%d", hr, file, line);
    ShowError(msg);
    exit(-1);
}


class VertexBuffer : public IDirect3DVertexBuffer9
{
public:
   enum LockFlags
   {
      WRITEONLY = 0,                        // in DX9, this is specified during VB creation
      NOOVERWRITE = D3DLOCK_NOOVERWRITE,    // meaning: no recently drawn vertices are overwritten
      DISCARDCONTENTS = D3DLOCK_DISCARD     // WARNING: this only works with dynamic VBs
   };
   bool lock( unsigned int offsetToLock, unsigned int sizeToLock, void **dataBuffer, DWORD flags )
   {
      return !FAILED(this->Lock(offsetToLock, sizeToLock, dataBuffer, flags) );
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
    m_pD3DDevice->Present(NULL, NULL, NULL, NULL);
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
    m_pD3DDevice->StretchRect(src, NULL, dest, NULL, D3DTEXF_NONE);
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

    m_pD3DDevice->SetMaterial((LPD3DMATERIAL9)_material);
}


void RenderDevice::SetRenderTarget( RenderTarget* surf)
{
    m_pD3DDevice->SetRenderTarget(0, surf);
}

void RenderDevice::SetZBuffer( RenderTarget* surf)
{
    dx7Device->SetDepthStencilSurface(surf);
}

void RenderDevice::SetTextureAddressMode(DWORD texUnit, TextureAddressMode mode)
{
    m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_ADDRESSU, mode);
    m_pD3DDevice->SetSamplerState(texUnit, D3DSAMP_ADDRESSV, mode);
}

void RenderDevice::createVertexBuffer( unsigned int numVertices, DWORD usage, DWORD fvf, VertexBuffer **vBuffer )
{
    // NB: We always specify WRITEONLY since MSDN states,
    // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
    // This means we cannot read from vertex buffers, but I don't think we need to.
    CHECKD3D(m_pD3DDevice->CreateVertexBuffer(numVertices * fvfToSize(fvf), D3DUSAGE_WRITEONLY | usage, fvf, D3DPOOL_DEFAULT, vBuffer));
}
