#include <d3d9.h>

typedef IDirect3dTexture9 BaseTexture;
typedef D3DVIEWPORT9 ViewPort;
typedef IDirect3DSurface9 RenderTarget;


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


////////////////////////////////////////////////////////////////////

RenderTarget* RenderDevice::DuplicateRenderTarget(RenderTarget* src)
{
    D3DSURFACE_DESC desc;
    src->GetDesc(&desc);
    IDirect3DSurface9 *dup;
    CHECKD3D(m_pD3DDevice->CreateRenderTarget(desc.Width, desc.Height, desc.Format,
        desc.MultiSample, desc.MultisampleQuality, FALSE /* lockable */, &dup, NULL));
    return dup;
}

void RenderDevice::GetTextureSize(BaseTexture* tex, DWORD *width, DWORD *height)
{
    D3DSURFACE_DESC desc;
    tex->GetLevelDesc(0, &desc);
    *width = desc.Width;
    *height = desc.Height;
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
    CHECKD3D(m_pD3DDevice->CreateVertexBuffer(numVertices * fvfToSize(fvf), D3DUSAGE_WRITEONLY | usage, fvf, D3DPOOL_DEFAULT, vBuffer));
}
