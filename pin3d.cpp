#include "StdAfx.h"
#include "RenderDevice.h"

int NumVideoBytes = 0;

Pin3D::Pin3D()
{
	m_scalex = m_scaley = 1.0f;
	m_xlatex = m_xlatey = 0.0f;
	m_pddsBackBuffer = NULL;
	m_pdds3DBackBuffer = NULL;
	m_pdds3Dbuffercopy = NULL;
	m_pdds3Dbufferzcopy = NULL;
	m_pdds3Dbuffermask = NULL;
	m_pddsZBuffer = NULL;
	m_pd3dDevice = NULL;
	m_pddsStatic = NULL;
	m_pddsStaticZ = NULL;
    ballShadowTexture = NULL;
	backgroundVBuffer = NULL;
    tableVBuffer = NULL;
    tableIBuffer = NULL;
}

Pin3D::~Pin3D()
{
	SAFE_RELEASE(m_pdds3DBackBuffer);
	if(m_pdds3Dbuffercopy) {
		_aligned_free((void*)m_pdds3Dbuffercopy);
		m_pdds3Dbuffercopy = NULL;
	}
	if(m_pdds3Dbufferzcopy) {
		_aligned_free((void*)m_pdds3Dbufferzcopy);
		m_pdds3Dbufferzcopy = NULL;
	}
	if(m_pdds3Dbuffermask) {
		free((void*)m_pdds3Dbuffermask);
		m_pdds3Dbuffermask = NULL;
	}

	SAFE_RELEASE(m_pddsZBuffer);

	SAFE_RELEASE(m_pddsStatic);

	SAFE_RELEASE(m_pddsStaticZ);

	for (int i=0; i<m_xvShadowMap.AbsoluteSize(); ++i)
		delete (BaseTexture*)m_xvShadowMap.AbsoluteElementAt(i);

   delete ballShadowTexture;
   ballTexture.FreeStuff();
   lightTexture[0].FreeStuff();
   lightTexture[1].FreeStuff();

	if(backgroundVBuffer)
		backgroundVBuffer->release();
    if (tableVBuffer)
        tableVBuffer->release();
    if (tableIBuffer)
        tableIBuffer->release();

	delete m_pd3dDevice;
}

void Pin3D::ClipRectToVisibleArea(RECT * const prc) const
{
	prc->top = max(prc->top, 0);
	prc->left = max(prc->left, 0);
	prc->right = min(prc->right, m_dwRenderWidth);
	prc->bottom = min(prc->bottom, m_dwRenderHeight);
}

void Pin3D::TransformVertices(const Vertex3D * rgv, const WORD * rgi, int count, Vertex3D * rgvout) const
{
	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	const float rClipWidth  = vp.Width*0.5f;
	const float rClipHeight = vp.Height*0.5f;
	const int xoffset = vp.X;
	const int yoffset = vp.Y;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; ++i)
	{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const float x = rgv[l].x;
		const float y = rgv[l].y;
		const float z = rgv[l].z;

		// Transform it through the current matrix set
		const float xp = m_proj.m_matrixTotal._11*x + m_proj.m_matrixTotal._21*y + m_proj.m_matrixTotal._31*z + m_proj.m_matrixTotal._41;
		const float yp = m_proj.m_matrixTotal._12*x + m_proj.m_matrixTotal._22*y + m_proj.m_matrixTotal._32*z + m_proj.m_matrixTotal._42;
		const float wp = m_proj.m_matrixTotal._14*x + m_proj.m_matrixTotal._24*y + m_proj.m_matrixTotal._34*z + m_proj.m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const float inv_wp = 1.0f/wp;
		const float vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const float vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		const float zp = m_proj.m_matrixTotal._13*x + m_proj.m_matrixTotal._23*y + m_proj.m_matrixTotal._33*z + m_proj.m_matrixTotal._43;
		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
		rgvout[l].z = zp * inv_wp;
		rgvout[l].rhw = wp;
	}
}

//copy pasted from above
void Pin3D::TransformVertices(const Vertex3D_NoTex2 * rgv, const WORD * rgi, int count, Vertex3D_NoTex2 * rgvout) const
{
	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	const float rClipWidth  = vp.Width*0.5f;
	const float rClipHeight = vp.Height*0.5f;
	const int xoffset = vp.X;
	const int yoffset = vp.Y;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; ++i)
	{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const float x = rgv[l].x;
		const float y = rgv[l].y;
		const float z = rgv[l].z;

		// Transform it through the current matrix set
		const float xp = m_proj.m_matrixTotal._11*x + m_proj.m_matrixTotal._21*y + m_proj.m_matrixTotal._31*z + m_proj.m_matrixTotal._41;
		const float yp = m_proj.m_matrixTotal._12*x + m_proj.m_matrixTotal._22*y + m_proj.m_matrixTotal._32*z + m_proj.m_matrixTotal._42;
		const float wp = m_proj.m_matrixTotal._14*x + m_proj.m_matrixTotal._24*y + m_proj.m_matrixTotal._34*z + m_proj.m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const float inv_wp = 1.0f/wp;
		const float vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const float vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		const float zp = m_proj.m_matrixTotal._13*x + m_proj.m_matrixTotal._23*y + m_proj.m_matrixTotal._33*z + m_proj.m_matrixTotal._43;
		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
		rgvout[l].z = zp * inv_wp;
		rgvout[l].rhw = wp;
	}
}

//copy pasted from above
void Pin3D::TransformVertices(const Vertex3D_NoLighting * rgv, const WORD * rgi, int count, Vertex3D_NoLighting * rgvout) const
{
	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	const float rClipWidth  = vp.Width*0.5f;
	const float rClipHeight = vp.Height*0.5f;
	const int xoffset = vp.X;
	const int yoffset = vp.Y;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; ++i)
	{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const float x = rgv[l].x;
		const float y = rgv[l].y;
		const float z = rgv[l].z;

		// Transform it through the current matrix set
		const float xp = m_proj.m_matrixTotal._11*x + m_proj.m_matrixTotal._21*y + m_proj.m_matrixTotal._31*z + m_proj.m_matrixTotal._41;
		const float yp = m_proj.m_matrixTotal._12*x + m_proj.m_matrixTotal._22*y + m_proj.m_matrixTotal._32*z + m_proj.m_matrixTotal._42;
		const float wp = m_proj.m_matrixTotal._14*x + m_proj.m_matrixTotal._24*y + m_proj.m_matrixTotal._34*z + m_proj.m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const float inv_wp = 1.0f/wp;
		const float vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const float vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		const float zp = m_proj.m_matrixTotal._13*x + m_proj.m_matrixTotal._23*y + m_proj.m_matrixTotal._33*z + m_proj.m_matrixTotal._43;
		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
		rgvout[l].z = zp * inv_wp;
	}
}

//copy pasted from above
void Pin3D::TransformVertices(const Vertex3D * rgv, const WORD * rgi, int count, Vertex2D * rgvout) const
{
	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	const float rClipWidth  = vp.Width*0.5f;
	const float rClipHeight = vp.Height*0.5f;
	const int xoffset = vp.X;
	const int yoffset = vp.Y;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; ++i)
	{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const float x = rgv[l].x;
		const float y = rgv[l].y;
		const float z = rgv[l].z;

		// Transform it through the current matrix set
		const float xp = m_proj.m_matrixTotal._11*x + m_proj.m_matrixTotal._21*y + m_proj.m_matrixTotal._31*z + m_proj.m_matrixTotal._41;
		const float yp = m_proj.m_matrixTotal._12*x + m_proj.m_matrixTotal._22*y + m_proj.m_matrixTotal._32*z + m_proj.m_matrixTotal._42;
		const float wp = m_proj.m_matrixTotal._14*x + m_proj.m_matrixTotal._24*y + m_proj.m_matrixTotal._34*z + m_proj.m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const float inv_wp = 1.0f/wp;
		const float vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const float vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
	}
}

//copy pasted from above
void Pin3D::TransformVertices(const Vertex3D_NoTex2 * rgv, const WORD * rgi, int count, Vertex2D * rgvout) const
{
	// Get the width and height of the viewport. This is needed to scale the
	// transformed vertices to fit the render window.
	const float rClipWidth  = vp.Width*0.5f;
	const float rClipHeight = vp.Height*0.5f;
	const int xoffset = vp.X;
	const int yoffset = vp.Y;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; ++i)
	{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const float x = rgv[l].x;
		const float y = rgv[l].y;
		const float z = rgv[l].z;

		// Transform it through the current matrix set
		const float xp = m_proj.m_matrixTotal._11*x + m_proj.m_matrixTotal._21*y + m_proj.m_matrixTotal._31*z + m_proj.m_matrixTotal._41;
		const float yp = m_proj.m_matrixTotal._12*x + m_proj.m_matrixTotal._22*y + m_proj.m_matrixTotal._32*z + m_proj.m_matrixTotal._42;
		const float wp = m_proj.m_matrixTotal._14*x + m_proj.m_matrixTotal._24*y + m_proj.m_matrixTotal._34*z + m_proj.m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const float inv_wp = 1.0f/wp;
		const float vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const float vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
	}
}

HRESULT Pin3D::InitPin3D(const HWND hwnd, const bool fFullScreen, const int screenwidth, const int screenheight, const int colordepth, int &refreshrate, const bool stereo3DFXAA, const bool AA)
{
    m_hwnd = hwnd;
    //fullscreen = fFullScreen;
    // Check if we are rendering in hardware.

    // Get the dimensions of the viewport and screen bounds
    GetClientRect( hwnd, &m_rcScreen );
    ClientToScreen( hwnd, (POINT*)&m_rcScreen.left );
    ClientToScreen( hwnd, (POINT*)&m_rcScreen.right );
    m_dwRenderWidth  = m_rcScreen.right  - m_rcScreen.left;
    m_dwRenderHeight = m_rcScreen.bottom - m_rcScreen.top;

    try {
        m_pd3dDevice = new RenderDevice(m_hwnd, m_dwRenderWidth, m_dwRenderHeight, fFullScreen, screenwidth, screenheight, colordepth, refreshrate);
    } catch (...) {
        return E_FAIL;
    }

    // set the viewport for the newly created device
    vp.X=0;
    vp.Y=0;
    vp.Width=m_dwRenderWidth;
    vp.Height=m_dwRenderHeight;
    vp.MinZ=0.0f;
    vp.MaxZ=1.0f;
    m_pd3dDevice->SetViewport( &vp );

    m_pddsBackBuffer = m_pd3dDevice->GetBackBuffer();

    // Create the "static" color buffer.  
    // This will hold a pre-rendered image of the table and any non-changing elements (ie ramps, decals, etc).
    m_pddsStatic = m_pd3dDevice->DuplicateRenderTarget(m_pddsBackBuffer);

    m_pddsZBuffer = m_pd3dDevice->AttachZBufferTo(m_pddsBackBuffer);
    m_pddsStaticZ = m_pd3dDevice->AttachZBufferTo(m_pddsStatic);
    if (!m_pddsZBuffer || !m_pddsStatic)
        return E_FAIL;

    CreateBallShadow();

    ballTexture.CreateFromResource(IDB_BALLTEXTURE);
    ballTexture.SetAlpha(RGB(0,0,0));
    ballTexture.CreateMipMap();

    lightTexture[0].CreateFromResource(IDB_SUNBURST);
    lightTexture[0].SetAlpha(RGB(0,0,0));
    lightTexture[0].CreateMipMap();

    lightTexture[1].CreateFromResource(IDB_SUNBURST5);
    lightTexture[1].SetAlpha(RGB(0,0,0));
    lightTexture[1].CreateMipMap();

    m_pddsLightWhite.CreateFromResource(IDB_WHITE);
    m_pddsLightWhite.SetAlpha(RGB(0,0,0));
    m_pddsLightWhite.CreateMipMap();

    if(stereo3DFXAA) {
        // TODO (DX9): disabled for now
        ShowError("Stereo 3D not supported in this version");
    //	ZeroMemory(&ddsd,sizeof(ddsd));
    //	ddsd.dwSize = sizeof(ddsd);
    //	m_pddsBackBuffer->GetSurfaceDesc( &ddsd );

    //	m_pdds3Dbuffercopy  = (unsigned int*)_aligned_malloc(ddsd.lPitch*ddsd.dwHeight,16);
    //	m_pdds3Dbufferzcopy = (unsigned int*)_aligned_malloc(ddsd.lPitch*ddsd.dwHeight,16);
    //	m_pdds3Dbuffermask  = (unsigned char*)malloc(ddsd.lPitch*ddsd.dwHeight/4);
    //	if(m_pdds3Dbuffercopy == NULL || m_pdds3Dbufferzcopy == NULL || m_pdds3Dbuffermask == NULL)
    //	{
    //		ShowError("Could not allocate 3D stereo buffers.");
    //		return E_FAIL; 
    //	}

    //	ddsd.dwFlags         = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_CAPS; //!! ? just to be the exact same as the Backbuffer
    //	ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
    //	ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTDYNAMIC;
    //	if( FAILED( hr = m_pDD->CreateSurface( &ddsd, (LPDIRECTDRAWSURFACE7*)&m_pdds3DBackBuffer, NULL ) ) )
    //	{
    //		ShowError("Could not create 3D stereo buffer.");
    //		return hr; 
    //	}
    }

	InitRenderState();

    // Direct all renders to the "static" buffer.
    SetRenderTarget(m_pddsStatic, m_pddsStaticZ);

    return S_OK;
}


// Sets the texture filtering state.
void Pin3D::SetTextureFilter(const int TextureNum, const int Mode) const
{
    m_pd3dDevice->SetTextureFilter(TextureNum, Mode);
}

void Pin3D::SetRenderTarget(RenderTarget* pddsSurface, RenderTarget* pddsZ) const
{
	m_pd3dDevice->SetRenderTarget(pddsSurface);
	m_pd3dDevice->SetZBuffer(pddsZ);
}

void Pin3D::InitRenderState() 
{
	m_pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
	m_pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCALPHA );

	m_pd3dDevice->SetRenderState(RenderDevice::SPECULARENABLE, FALSE);
	m_pd3dDevice->SetRenderState(RenderDevice::DITHERENABLE, TRUE);

	m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, TRUE);
	m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
    m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

	m_pd3dDevice->SetRenderState( RenderDevice::CLIPPING, FALSE );
	m_pd3dDevice->SetRenderState( RenderDevice::CLIPPLANEENABLE, 0 );

	m_pd3dDevice->SetRenderState( RenderDevice::NORMALIZENORMALS, TRUE );

    // initialize first texture stage
    m_pd3dDevice->SetTextureAddressMode(ePictureTexture, RenderDevice::TEX_CLAMP/*WRAP*/);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureFilter(ePictureTexture, TEXTURE_MODE_TRILINEAR );
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_TEXCOORDINDEX, 0);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_pd3dDevice->SetTexture(ePictureTexture, NULL);

    // initialize second texture stage (light map)
	SetTextureFilter( eLightProject1, TEXTURE_MODE_BILINEAR );
	m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLORARG2, D3DTA_CURRENT );
	m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_TEXCOORDINDEX, 1 );
	//m_pd3dDevice->SetTextureStageState( eLightProject1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
}

static const WORD rgiPin3D1[4] = {2,3,5,6};

void Pin3D::DrawBackground()
{
	PinTable * const ptable = g_pplayer->m_ptable;
	Texture * const pin = ptable->GetDecalsEnabled() ? ptable->GetImage((char *)g_pplayer->m_ptable->m_szImageBackdrop) : NULL;
	if (pin)
	{
		float maxtu,maxtv;
		g_pplayer->m_ptable->GetTVTU(pin, &maxtu, &maxtv);

		Vertex3D_NoTex2 rgv3D[4];

		rgv3D[0].x = 0;
		rgv3D[0].y = 0;
		rgv3D[0].tu = 0;
		rgv3D[0].tv = 0;

		rgv3D[1].x = 1000.0f;
		rgv3D[1].y = 0;
		rgv3D[1].tu = maxtu;
		rgv3D[1].tv = 0;

		rgv3D[2].x = 1000.0f;
		rgv3D[2].y = 750.0f;
		rgv3D[2].tu = maxtu;
		rgv3D[2].tv = maxtv;

		rgv3D[3].x = 0;
		rgv3D[3].y = 750.0f;
		rgv3D[3].tu = 0;
		rgv3D[3].tv = maxtv;

		SetHUDVertices(rgv3D, 4);
		SetDiffuse(rgv3D, 4, 0xFFFFFF);
		Vertex3D_NoTex2 *buf;

		//init background
		if( !backgroundVBuffer )
			m_pd3dDevice->CreateVertexBuffer( 4, 0, MY_D3DTRANSFORMED_NOTEX2_VERTEX, &backgroundVBuffer);
		backgroundVBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
		memcpy( buf, rgv3D, sizeof(Vertex3D_NoTex2)*4);
		backgroundVBuffer->unlock();

		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0L );

		m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
        m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, FALSE);

		SetTexture(pin);
		m_pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLEFAN, backgroundVBuffer, 0, 4);

		m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
        m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE);
	}
	else
	{
        const D3DCOLOR d3dcolor = COLORREF_to_D3DCOLOR(ptable->m_colorbackdrop);
		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, d3dcolor, 1.0f, 0L );
	}
}



void Pin3D::InitLights()
{
	const float sn = sinf(m_inclination + (float)(M_PI - (M_PI*3.0/16.0)));
	const float cs = cosf(m_inclination + (float)(M_PI - (M_PI*3.0/16.0)));

	for(unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
    {
        if(g_pplayer->m_ptable->m_Light[i].enabled)
        {
            BaseLight light;
            light.setType((g_pplayer->m_ptable->m_Light[i].type == LIGHT_DIRECTIONAL) ? D3DLIGHT_DIRECTIONAL
                    : ((g_pplayer->m_ptable->m_Light[i].type == LIGHT_POINT) ? D3DLIGHT_POINT : D3DLIGHT_SPOT));
            light.setAmbient( (float)(g_pplayer->m_ptable->m_Light[i].ambient & 255) * (float)(1.0/255.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].ambient & 65280) * (float)(1.0/65280.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].ambient & 16711680) * (float)(1.0/16711680.0));
            light.setDiffuse( (float)(g_pplayer->m_ptable->m_Light[i].diffuse & 255) * (float)(1.0/255.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].diffuse & 65280) * (float)(1.0/65280.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].diffuse & 16711680) * (float)(1.0/16711680.0));
            light.setSpecular((float)(g_pplayer->m_ptable->m_Light[i].specular & 255) * (float)(1.0/255.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].specular & 65280) * (float)(1.0/65280.0),
                    (float)(g_pplayer->m_ptable->m_Light[i].specular & 16711680) * (float)(1.0/16711680.0));
            light.setRange( /*(light.getType() == D3DLIGHT_POINT) ? g_pplayer->m_ptable->m_Light[i].pointRange :*/
                    /* DX9 D3DLIGHT_RANGE_MAX */ 1e6f); //!!  expose?

            if((light.getType() == D3DLIGHT_POINT) || (light.getType() == D3DLIGHT_SPOT))
                light.setAttenuation2(0.0000025f); //!!  expose? //!! real world: light.dvAttenuation2 = 1.0f; but due to low dynamic 255-level-RGB lighting, we have to stick with the old crap

            if(light.getType() == D3DLIGHT_SPOT)
            {
                light.setFalloff( /*g_pplayer->m_ptable->m_Light[i].spotExponent;*/ 1.0f ); //!!  expose?
                light.setPhi    ( /*g_pplayer->m_ptable->m_Light[i].spotMin*/ (float)(60*M_PI/180.0) ); //!!  expose?
                light.setTheta  ( /*g_pplayer->m_ptable->m_Light[i].spotMax*/ (float)(20*M_PI/180.0) ); //!!  expose?
            }

            // transform dir & pos with world trafo to be always aligned with table (so that user trafos don't change the lighting!)
            if((g_pplayer->m_ptable->m_Light[i].dir.x == 0.0f) && (g_pplayer->m_ptable->m_Light[i].dir.y == 0.0f) &&
                    (g_pplayer->m_ptable->m_Light[i].dir.z == 0.0f) && (i < 2) && (light.getType() == D3DLIGHT_DIRECTIONAL))
            {
                // backwards compatibilty
                light.setAmbient(0.1f, 0.1f, 0.1f);
                light.setDiffuse(0.4f, 0.4f, 0.4f);
                light.setSpecular(0, 0, 0);
                light.setAttenuation0(0.0f);
                light.setAttenuation1(0.0f);
                light.setAttenuation2(0.0f);

                if ( i==0 )
                {
                    light.setDirection(5.0f, sn * 21.0f, cs * -21.0f);
                }
                else
                {
                    light.setDirection(-8.0f, sn * 11.0f, cs * -11.0f); 
                    light.setDiffuse(0.6f, 0.6f, 0.6f);
                    light.setSpecular(1.0f, 1.0f, 1.0f);
                }

            }
            else 
            {
                const Vertex3Ds tmp = m_proj.m_matWorld.MultiplyVectorNoTranslate(g_pplayer->m_ptable->m_Light[i].dir);
                light.setDirection(tmp.x, tmp.y, tmp.z);
            }

            const Vertex3Ds tmp = m_proj.m_matWorld.MultiplyVector(g_pplayer->m_ptable->m_Light[i].pos);
            light.setPosition(tmp.x, tmp.y, tmp.z);

            m_pd3dDevice->SetLight(i, &light);
            if (light.getAmbient().r  > 0.0f || light.getAmbient().g  > 0.0f || light.getAmbient().b  > 0.0f ||
                light.getDiffuse().r  > 0.0f || light.getDiffuse().g  > 0.0f || light.getDiffuse().b  > 0.0f ||
                light.getSpecular().r > 0.0f || light.getSpecular().g > 0.0f || light.getSpecular().b > 0.0f)
                m_pd3dDevice->LightEnable(i, TRUE);
            else
                m_pd3dDevice->LightEnable(i, FALSE);
        }
        else
            m_pd3dDevice->LightEnable(i, FALSE);
    }

	m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE);
}

// currently unused
//void LookAt( Matrix3D &mat, D3DVECTOR eye, D3DVECTOR target, D3DVECTOR up )
//{
//   D3DVECTOR zaxis = Normalize(eye - target);
//   D3DVECTOR xaxis = Normalize(CrossProduct(up,zaxis));
//   D3DVECTOR yaxis = CrossProduct(zaxis,xaxis);
//   mat._11 = xaxis.x; mat._12 = yaxis.x; mat._13 = zaxis.x; mat._14=0.0f;
//   mat._21 = xaxis.y; mat._22 = yaxis.y; mat._23 = zaxis.y; mat._24=0.0f;
//   mat._31 = xaxis.z; mat._32 = yaxis.z; mat._33 = zaxis.z; mat._34=0.0f;
//   mat._41 = 0.0f;    mat._42 = 0.0f;    mat._43 = zaxis.x; mat._44=0.0f;
//   Matrix3D trans;
//   trans.SetIdentity();
//   trans._41 = eye.x; trans._42 = eye.y; trans._43=eye.z;
//   mat.Multiply( trans, mat );
//}

void Pin3D::InitLayout(const float left, const float top, const float right, const float bottom, const float inclination, const float FOV, const float rotation, const float scalex, const float scaley, const float xlatex, const float xlatey, const float xlatez, const float layback, const float maxSeparation, const float ZPD)
{
    TRACE_FUNCTION();
	m_layback = layback;

	m_maxSeparation = maxSeparation;
	m_ZPD = ZPD;

	m_scalex = scalex;
	m_scaley = scaley;

	m_xlatex = xlatex;
	m_xlatey = xlatey;

	m_rotation = ANGTORAD(rotation);
	m_inclination = ANGTORAD(inclination);

	m_lightproject.m_v.x = g_pplayer->m_ptable->m_right *0.5f;
	m_lightproject.m_v.y = g_pplayer->m_ptable->m_bottom *0.5f;
    m_lightproject.inv_width  = 1.0f/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
    m_lightproject.inv_height = 1.0f/(g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom);

	Vector<Vertex3Ds> vvertex3D;
	for (int i=0; i<g_pplayer->m_ptable->m_vedit.Size(); ++i)
		g_pplayer->m_ptable->m_vedit.ElementAt(i)->GetBoundingVertices(&vvertex3D);

	const GPINFLOAT aspect = 4.0/3.0;//((GPINFLOAT)m_dwRenderWidth)/m_dwRenderHeight;
	m_proj.FitCameraToVertices(&vvertex3D/*rgv*/, vvertex3D.Size(), aspect, m_rotation, m_inclination, FOV, xlatez);

	for (int i=0; i<vvertex3D.Size(); ++i)
		delete vvertex3D.ElementAt(i);

    m_proj.SetFieldOfView(FOV, aspect, m_proj.m_rznear, m_proj.m_rzfar);

	const float skew = -tanf(m_layback*(float)(M_PI/360));
	// skew the coordinate system from kartesian to non kartesian.
	skewX = -sinf(m_rotation)*skew;
	skewY =  cosf(m_rotation)*skew;
	Matrix3D matTrans;
    matTrans.SetIdentity();
	// Skew for FOV of 0 Deg. is not supported. so change it a little bit.
	const float skewFOV = (FOV < 0.01f) ? 0.01f : FOV;
	// create skew the z axis to x and y direction.
	const float skewtan = tanf(ANGTORAD((180.0f-skewFOV)*0.5f))*m_proj.m_vertexcamera.y;
	matTrans._42 = skewtan*skewY;
	matTrans._32 = skewY;
	matTrans._41 = skewtan*skewX;
	matTrans._31 = skewX;
	m_proj.Multiply(matTrans);

    m_proj.Scale( m_scalex != 0.0f ? m_scalex : 1.0f, m_scaley != 0.0f ? m_scaley : 1.0f, 1.0f );
	m_proj.Rotate( 0, 0, m_rotation );
	m_proj.Translate(m_xlatex-m_proj.m_vertexcamera.x,m_xlatey-m_proj.m_vertexcamera.y,-m_proj.m_vertexcamera.z);
	m_proj.Rotate( m_inclination, 0, 0 );

	m_pd3dDevice->SetTransform(TRANSFORMSTATE_PROJECTION, &m_proj.m_matProj);
	m_pd3dDevice->SetTransform(TRANSFORMSTATE_VIEW, &m_proj.m_matView);
    m_pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &m_proj.m_matWorld);

	m_proj.CacheTransform();

    // Compute view vector
    Matrix3D temp, worldViewRot;
    m_proj.m_matView.Multiply( m_proj.m_matWorld, temp );   // TODO: use only view matrix once the camera transform is put there
    temp.Invert();
    temp.GetRotationPart( worldViewRot );
    worldViewRot.MultiplyVector(0, 0, 1, &m_viewVec);
    m_viewVec.Normalize();

	InitLights();
}

void Pin3D::RenderPlayfieldGraphics()
{
    TRACE_FUNCTION();
#define TRIANGULATE_BACK 100

	Vertex3D rgv[7];
	rgv[0].x=g_pplayer->m_ptable->m_left;     rgv[0].y=g_pplayer->m_ptable->m_top;      rgv[0].z=0;
	rgv[1].x=g_pplayer->m_ptable->m_right;    rgv[1].y=g_pplayer->m_ptable->m_top;      rgv[1].z=0;
	rgv[2].x=g_pplayer->m_ptable->m_right;    rgv[2].y=g_pplayer->m_ptable->m_bottom;   rgv[2].z=0;
	rgv[3].x=g_pplayer->m_ptable->m_left;     rgv[3].y=g_pplayer->m_ptable->m_bottom;   rgv[3].z=0;

	// These next 4 vertices are used just to set the extents
	rgv[4].x=g_pplayer->m_ptable->m_left;     rgv[4].y=g_pplayer->m_ptable->m_top;      rgv[4].z=50.0f;
	rgv[5].x=g_pplayer->m_ptable->m_left;     rgv[5].y=g_pplayer->m_ptable->m_bottom;   rgv[5].z=50.0f;
	rgv[6].x=g_pplayer->m_ptable->m_right;    rgv[6].y=g_pplayer->m_ptable->m_bottom;   rgv[6].z=50.0f;
	//rgv[7].x=g_pplayer->m_ptable->m_right;    rgv[7].y=g_pplayer->m_ptable->m_top;      rgv[7].z=50.0f;

	Texture * const pin = g_pplayer->m_ptable->GetImage((char *)g_pplayer->m_ptable->m_szImage);
	float maxtu,maxtv;

	if (pin)
	{
		// Calculate texture coordinates.
		g_pplayer->m_ptable->GetTVTU(pin, &maxtu, &maxtv);		
	}
	else // No image by that name
	{
		maxtv = maxtu = 1.0f;
	}

	EnableLightMap(0);

	for (int i=0; i<4; ++i)
	{
		rgv[i].nx = 0;
		rgv[i].ny = 0;
		rgv[i].nz = -1.0f;

		rgv[i].tv = (i&2) ? maxtv : 0;
		rgv[i].tu = (i==1 || i==2) ? maxtu : 0;

		m_lightproject.CalcCoordinates(&rgv[i]);
	}
	// triangulate for better vertex based lighting //!! disable/set to 0 as soon as pixel shaders do the lighting

	const DWORD numVerts = (TRIANGULATE_BACK+1)*(TRIANGULATE_BACK+1);
	const DWORD numIndices = TRIANGULATE_BACK*TRIANGULATE_BACK*6;

    assert(tableVBuffer == NULL);
    m_pd3dDevice->CreateVertexBuffer( numVerts+7, 0, MY_D3DFVF_VERTEX, &tableVBuffer); //+7 verts for second rendering step

    std::vector<Vertex3D> buffer(numVerts);

	const float inv_tb = (float)(1.0/TRIANGULATE_BACK);
	unsigned int offs = 0;
	for(unsigned int y = 0; y <= TRIANGULATE_BACK; ++y)
	{
		for(unsigned int x = 0; x <= TRIANGULATE_BACK; ++x,++offs) //!! triangulate more in y then in x?
		{
			Vertex3D &tmp = buffer[offs];
			tmp.x = rgv[0].x + (rgv[1].x-rgv[0].x) * ((float)x*inv_tb);
			tmp.y = rgv[0].y + (rgv[2].y-rgv[0].y) * ((float)y*inv_tb);
			tmp.z = rgv[0].z;

			tmp.tu = rgv[0].tu + (rgv[1].tu-rgv[0].tu) * ((float)x*inv_tb);
			tmp.tv = rgv[0].tv + (rgv[2].tv-rgv[0].tv) * ((float)y*inv_tb);

			tmp.nx = rgv[0].nx;
			tmp.ny = rgv[0].ny;
			tmp.nz = rgv[0].nz;
			m_lightproject.CalcCoordinates(&tmp);
		}
	}

    std::vector<WORD> playfieldPolyIndices(numIndices);

	offs = 0;
	for(int y = 0; y < TRIANGULATE_BACK; ++y)
	{
        unsigned int offs2 = y * (TRIANGULATE_BACK+1);
		for(int x = 0; x < TRIANGULATE_BACK; ++x,offs+=6,++offs2)
		{
			WORD *tmp = &playfieldPolyIndices[offs];
			tmp[3] = tmp[0] = offs2;
			tmp[1] = offs2+1;
			tmp[4] = tmp[2] = offs2+1+(TRIANGULATE_BACK+1);
			tmp[5] = offs2+(TRIANGULATE_BACK+1);
		}
	}

	Vertex3D *buf;
	tableVBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
	memcpy( buf, &buffer[0], sizeof(Vertex3D)*numVerts);
	SetNormal(rgv, rgiPin3D1, 4, NULL, NULL, 0);
	memcpy( &buf[numVerts], rgv, 7*sizeof(Vertex3D));
	tableVBuffer->unlock();

	EnableLightMap(0);

	Material mtrl;

	if (pin)
	{
		SetTexture(pin);
	}
	else // No image by that name
	{
		SetTexture(NULL);
		mtrl.setColor( 1.0f, g_pplayer->m_ptable->m_colorplayfield );
	}
	m_pd3dDevice->SetMaterial(mtrl);

    assert(tableIBuffer == NULL);
    tableIBuffer = m_pd3dDevice->CreateAndFillIndexBuffer(playfieldPolyIndices);
	m_pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, tableVBuffer, 0, numVerts, tableIBuffer, 0, numIndices);

	DisableLightMap();
	SetTexture(NULL);
    if (pin)
    {
        m_pd3dDevice->SetTexture(0, NULL);
        m_pd3dDevice->m_texMan.UnloadTexture(pin->m_pdsBuffer);
    }

	m_pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLEFAN, tableVBuffer, numVerts, 7, (LPWORD)rgiPin3D1, 4);

    // Apparently, releasing the vertex buffer here immediately can cause rendering glitches in
    // later rendering steps, so we keep it around for now.
}

const int rgfilterwindow[7][7] =
   {1, 4, 8, 10, 8, 4, 1,
	4, 12, 25, 29, 25, 12, 4,
	8, 25, 49, 58, 49, 25, 8,
	10, 29, 58, 67, 58, 29, 10,
	8, 25, 49, 58, 49, 25, 8,
	4, 12, 25, 29, 25, 12, 4,
	1, 4, 8, 10, 8, 4, 1};

void Pin3D::CreateBallShadow()
{
	ballShadowTexture = new MemTexture(16, 16);

	const int pitch = ballShadowTexture->pitch();
	const int width = ballShadowTexture->width();
	const int height = ballShadowTexture->height();
	BYTE * const pc = ballShadowTexture->data();

	// Sharp Shadow
	int offset = 0;
	for (int y=0; y<height; ++y)
	{
		for (int x=0; x<width; ++x)
		{
			const int dx = 8-x;
			const int dy = 8-y;
			const int dist = dx*dx + dy*dy;
			pc[offset+x*4] = (dist <= 32) ? (BYTE)255 : (BYTE)0;
		}
		offset += pitch;
	}

	int totalwindow = 0;
	for (int i=0;i<7;++i)
		for (int l=0;l<7;++l)
			totalwindow += rgfilterwindow[i][l];

	for (int i=0;i<height;i++)
	{
		for (int l=0;l<width;l++)
		{
			int value = 0;
			for (int n=0;n<7;n++)
			{
				const int y = i+n-3;
				if(/*y>=0 &&*/ (unsigned int)y<=15)
				{
					BYTE * const pcy = pc + pitch*y;
					for (int m=0;m<7;m++)
					{
						const int x = l+m-3;
						if (/*x>=0 &&*/ (unsigned int)x<=15)
							value += (int)(*(pcy + 4*x)) * rgfilterwindow[m][n];
					}
				}
			}

			value /= totalwindow;

			value = (value*5)>>3;

			*(pc + pitch*i + l*4 + 3) = (BYTE)value;
		}
	}

	// Black out color channel
	offset = 0;
	for (int y=0; y<height; ++y)
	{
		for (int x=0; x<width*4; x+=4)
		{
			pc[offset+x  ] = 0;
			pc[offset+x+1] = 0;
			pc[offset+x+2] = 0;
		}
		offset += pitch;
	}
}

BaseTexture* Pin3D::CreateShadow(const float z)
{
	const float centerx = (g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right)*0.5f;
	const float centery = (g_pplayer->m_ptable->m_top + g_pplayer->m_ptable->m_bottom)*0.5f;

	int shadwidth;
	int shadheight;
	if (centerx > centery)
	{
		shadwidth = 256;
		m_maxtu = 1.0f;
		m_maxtv = centery/centerx;
		shadheight = (int)(256.0f*m_maxtv);
	}
	else
	{
		shadheight = 256;
		m_maxtu = centerx/centery;
		m_maxtv = 1.0f;
		shadwidth = (int)(256.0f*m_maxtu);
	}

	// Create Shadow Picture
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 256;//shadwidth;
	bmi.bmiHeader.biHeight = -256;//-shadheight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;

	//WriteFile(hfile, &bmi, sizeof(bmi), &foo, NULL);

	HDC hdcScreen = GetDC(NULL);
	HDC hdc2 = CreateCompatibleDC(hdcScreen);

	BYTE *pbits;
	HBITMAP hdib = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void **)&pbits, NULL, 0);

	HBITMAP hbmOld = (HBITMAP)SelectObject(hdc2, hdib);
	const float zoom = (float)shadwidth/(centerx*2.0f);
	ShadowSur * const psur = new ShadowSur(hdc2, zoom, centerx, centery, shadwidth, shadheight, z, NULL);

	SelectObject(hdc2, GetStockObject(WHITE_BRUSH));
	PatBlt(hdc2, 0, 0, shadwidth, shadheight, PATCOPY);

	for (int i=0; i<g_pplayer->m_ptable->m_vedit.Size(); ++i)
		g_pplayer->m_ptable->m_vedit.ElementAt(i)->RenderShadow(psur, z);

	delete psur;

	BaseTexture* pddsProjectTexture = new MemTexture(shadwidth, shadheight);
	m_xvShadowMap.AddElement(pddsProjectTexture, (int)z);

    DWORD texWidth = pddsProjectTexture->width();
    DWORD texHeight = pddsProjectTexture->height();
	m_maxtu = (float)shadwidth/(float)texWidth;
	m_maxtv = (float)shadheight/(float)texHeight;

	SelectObject(hdc2, hbmOld);
	DeleteDC(hdc2);
	ReleaseDC(NULL, hdcScreen);

    Texture::Blur(pddsProjectTexture, pbits, shadwidth, shadheight);

	DeleteObject(hdib);

	return pddsProjectTexture;
}

void Pin3D::SetTexture(Texture* pTexture)
{
    SetBaseTexture(ePictureTexture, pTexture ? pTexture->m_pdsBuffer : NULL);
}

void Pin3D::SetBaseTexture(DWORD texUnit, BaseTexture* pddsTexture)
{
    m_pd3dDevice->SetTexture(texUnit,
            m_pd3dDevice->m_texMan.LoadTexture((pddsTexture == NULL) ? m_pddsLightWhite.m_pdsBufferColorKey : pddsTexture));
}


void Pin3D::EnableLightMap(const BOOL fEnable, const float z)
{
    if (fEnable)
        EnableLightMap(z);
    else
        DisableLightMap();
}

void Pin3D::EnableLightMap(const float z)
{
    BaseTexture* pdds = (BaseTexture*)m_xvShadowMap.ElementAt((int)z);
    if (!pdds)
        pdds = CreateShadow(z);
    SetBaseTexture(eLightProject1, pdds);
}

void Pin3D::DisableLightMap()
{
    m_pd3dDevice->SetTexture(eLightProject1, NULL);
}

void Pin3D::EnableAlphaTestReference(DWORD alphaRefValue) const
{
	m_pd3dDevice->SetRenderState(RenderDevice::ALPHAREF, alphaRefValue);
	m_pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, TRUE); 
	m_pd3dDevice->SetRenderState(RenderDevice::ALPHAFUNC, D3DCMP_GREATEREQUAL);
	m_pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
}

void Pin3D::EnableAlphaBlend( DWORD alphaRefValue, BOOL additiveBlending )
{
	m_pd3dDevice->SetRenderState(RenderDevice::DITHERENABLE, TRUE); 	
	EnableAlphaTestReference( alphaRefValue );
	m_pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
	m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, additiveBlending ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA);
	if(additiveBlending)
		m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_TFACTOR); // factor is 1,1,1,1
}

void Pin3D::Flip(const int offsetx, const int offsety, bool vsync)
{
    m_pd3dDevice->Flip(offsetx, offsety, vsync);
}

Vertex3Ds Pin3D::Unproject( Vertex3Ds *point)
{
   m_proj.CacheTransform(); // compute m_matrixTotal

   Matrix3D m2 = m_proj.m_matrixTotal; // = world * view * proj
   m2.Invert();
   Vertex3Ds p,p3;

   p.x = 2.0f * (point->x-g_pplayer->m_pin3d.vp.X) / g_pplayer->m_pin3d.vp.Width - 1.0f; 
   p.y = 1.0f - 2.0f * (point->y-g_pplayer->m_pin3d.vp.Y) / g_pplayer->m_pin3d.vp.Height; 
   p.z = (point->z - g_pplayer->m_pin3d.vp.MinZ) / (g_pplayer->m_pin3d.vp.MaxZ-g_pplayer->m_pin3d.vp.MinZ);
   p3 = m2.MultiplyVector( p );
   return p3;
}

Vertex3Ds Pin3D::Get3DPointFrom2D( POINT *p )
{
   Vertex3Ds p1,p2,pNear,pFar;
   pNear.x = (float)p->x; pNear.y = (float)p->y; pNear.z = (float)vp.MinZ;
   pFar.x = (float)p->x; pFar.y = (float)p->y; pFar.z = (float)vp.MaxZ;
   p1 = Unproject( &pNear );
   p2 = Unproject( &pFar);
   float wz = 0.0f;
   float wx = ((wz-p1.z)*(p2.x-p1.x))/(p2.z-p1.z) + p1.x;
   float wy = ((wz-p1.z)*(p2.y-p1.y))/(p2.z-p1.z) + p1.y;
   Vertex3Ds vertex(wx,wy,wz);
   return vertex;
}

#define MAX_INT 0x0fffffff //!!?

void Pin3D::ClearExtents(RECT * const prc, float * const pznear, float * const pzfar)
{
	prc->left = MAX_INT;
	prc->top = MAX_INT;
	prc->right = -MAX_INT;
	prc->bottom = -MAX_INT;

	if (pznear)
	{
		*pznear = FLT_MAX;
		*pzfar = -FLT_MAX;
	}
}

void Pin3D::ExpandExtents(RECT * const prc, Vertex3D* const rgv, float * const pznear, float * const pzfar, const int count, const BOOL fTransformed)
{
	Vertex3D * const rgvOut = (!fTransformed) ? new Vertex3D[count] : rgv;

	if (!fTransformed)
		TransformVertices(rgv, NULL, count, rgvOut);

	for (int i=0; i<count; ++i)
	{
		const int x = (int)(rgvOut[i].x + 0.5f);
		const int y = (int)(rgvOut[i].y + 0.5f);

		prc->left = min(prc->left, x - 1);
		prc->top = min(prc->top, y - 1);
		prc->right = max(prc->right, x + 1);
		prc->bottom = max(prc->bottom, y + 1);
      // clip the update rectangle to the screen boundary, 
      // if something gets out of the screen the result can be a slow-down, render destortion or a crash
      prc->bottom = min( prc->bottom, m_dwRenderHeight-1);
      prc->right  = min( prc->right, m_dwRenderWidth-1 );
      prc->top    = min( prc->top, m_dwRenderHeight-2 );
      prc->left   = min( prc->left, m_dwRenderWidth-2 );

		if (pznear)
		{
			*pznear = min(*pznear, rgvOut[i].z);
			*pzfar = max(*pzfar, rgvOut[i].z);
		}
	}

	if (!fTransformed)
		delete [] rgvOut;
}

//copy pasted from above
void Pin3D::ExpandExtents(RECT * const prc, Vertex3D_NoTex2* const rgv, float * const pznear, float * const pzfar, const int count, const BOOL fTransformed)
{
	Vertex3D_NoTex2 * const rgvOut = (!fTransformed) ? new Vertex3D_NoTex2[count] : rgv;

	if (!fTransformed)
		TransformVertices(rgv, NULL, count, rgvOut);

	for (int i=0; i<count; ++i)
	{
		const int x = (int)(rgvOut[i].x + 0.5f);
		const int y = (int)(rgvOut[i].y + 0.5f);

		prc->left = min(prc->left, x - 1);
		prc->top = min(prc->top, y - 1);
		prc->right = max(prc->right, x + 1);
		prc->bottom = max(prc->bottom, y + 1);
      // clip the update rectangle to the screen boundary, 
      // if something gets out of the screen the result can be a slow-down, render destortion or a crash
      prc->bottom = min( prc->bottom, m_dwRenderHeight-1);
      prc->right  = min( prc->right, m_dwRenderWidth-1 );
      prc->top    = min( prc->top, m_dwRenderHeight-2 );
      prc->left   = min( prc->left, m_dwRenderWidth-2 );

		if (pznear)
		{
			*pznear = min(*pznear, rgvOut[i].z);
			*pzfar = max(*pzfar, rgvOut[i].z);
		}
	}

	if (!fTransformed)
		delete [] rgvOut;
}

//copy pasted from above
void Pin3D::ExpandExtents(RECT * const prc, Vertex3D_NoLighting* const rgv, float * const pznear, float * const pzfar, const int count, const BOOL fTransformed)
{
	Vertex3D_NoLighting * const rgvOut = (!fTransformed) ? new Vertex3D_NoLighting[count] : rgv;

	if (!fTransformed)
		TransformVertices(rgv, NULL, count, rgvOut);

	for (int i=0; i<count; ++i)
	{
		const int x = (int)(rgvOut[i].x + 0.5f);
		const int y = (int)(rgvOut[i].y + 0.5f);

		prc->left = min(prc->left, x - 1);
		prc->top = min(prc->top, y - 1);
		prc->right = max(prc->right, x + 1);
		prc->bottom = max(prc->bottom, y + 1);
      // clip the update rectangle to the screen boundary, 
      // if something gets out of the screen the result can be a slow-down, render destortion or a crash
      prc->bottom = min( prc->bottom, m_dwRenderHeight-1);
      prc->right  = min( prc->right, m_dwRenderWidth-1 );
      prc->top    = min( prc->top, m_dwRenderHeight-2 );
      prc->left   = min( prc->left, m_dwRenderWidth-2 );
 
		if (pznear)
		{
			*pznear = min(*pznear, rgvOut[i].z);
			*pzfar = max(*pzfar, rgvOut[i].z);
		}
	}

	if (!fTransformed)
		delete [] rgvOut;
}

void PinProjection::Rotate(const GPINFLOAT x, const GPINFLOAT y, const GPINFLOAT z)
{
	Matrix3D matRotateX, matRotateY, matRotateZ;

	matRotateX.RotateXMatrix(x);
	m_matWorld.Multiply(matRotateX, m_matWorld);
	matRotateY.RotateYMatrix(y);
	m_matWorld.Multiply(matRotateY, m_matWorld);
	matRotateZ.RotateZMatrix(z);
	m_matWorld.Multiply(matRotateZ, m_matWorld);        // matWorld = rotZ * rotY * rotX * origMatWorld
}

void PinProjection::Translate(const float x, const float y, const float z)
{
	Matrix3D matTrans;

	matTrans._11 = matTrans._22 = matTrans._33 = matTrans._44 = 1.0f;
	matTrans._12 = matTrans._13 = matTrans._14 = 0.0f;
	matTrans._21 = matTrans._23 = matTrans._24 = 0.0f;
	matTrans._31 = matTrans._32 = matTrans._34 = 0.0f;

	matTrans._41 = x;
	matTrans._42 = y;
	matTrans._43 = z;
	m_matWorld.Multiply(matTrans, m_matWorld);
}

void PinProjection::Scale(const float x, const float y, const float z)
{
	m_matWorld.Scale( x, y, z );
}

void PinProjection::Multiply(const Matrix3D& mat)
{
	m_matWorld.Multiply(mat, m_matWorld);
}

void PinProjection::FitCameraToVertices(Vector<Vertex3Ds> * const pvvertex3D, const int cvert, const GPINFLOAT aspect, const GPINFLOAT rotation, const GPINFLOAT inclination, const GPINFLOAT FOV, const float xlatez)
{
	// Determine camera distance
	const GPINFLOAT rrotsin = sin(-rotation);
	const GPINFLOAT rrotcos = cos(-rotation);
	const GPINFLOAT rincsin = sin(-inclination);
	const GPINFLOAT rinccos = cos(-inclination);

	const GPINFLOAT slopey = tan(FOV*(2.0*0.5*M_PI/360.0)); // *0.5 because slope is half of FOV - FOV includes top and bottom

	// Field of view along the axis = atan(tan(yFOV)*width/height)
	// So the slope of x simply equals slopey*width/height

	const GPINFLOAT slopex = slopey*aspect;// slopey*m_rcHard.width/m_rcHard.height;

	GPINFLOAT maxyintercept = -DBL_MAX;
	GPINFLOAT minyintercept = DBL_MAX;
	GPINFLOAT maxxintercept = -DBL_MAX;
	GPINFLOAT minxintercept = DBL_MAX;

	m_rznear = 0;
	m_rzfar = 0;

	for (int i=0; i<cvert; ++i)
	{
		GPINFLOAT vertexTy = (*pvvertex3D->ElementAt(i)).y;

		// Rotate vertex about y axis according to incoming rotation
		const GPINFLOAT temp = (*pvvertex3D->ElementAt(i)).x;
		const GPINFLOAT vertexTx = rrotcos*temp + rrotsin*(*pvvertex3D->ElementAt(i)).z;
		GPINFLOAT vertexTz = rrotcos*(*pvvertex3D->ElementAt(i)).z - rrotsin*temp;

		// Rotate vertex about x axis according to incoming inclination
		const GPINFLOAT temp2 = vertexTy;
		vertexTy = rinccos*temp2 + rincsin*vertexTz;
		vertexTz = rinccos*vertexTz - rincsin*temp2;

		// Extend z-range if necessary
		m_rznear = min(m_rznear, -vertexTz);
		m_rzfar  = max(m_rzfar,  -vertexTz);

		// Extend slope lines from point to find camera intersection
		maxyintercept = max(maxyintercept, vertexTy + slopey*vertexTz);
		minyintercept = min(minyintercept, vertexTy - slopey*vertexTz);
		maxxintercept = max(maxxintercept, vertexTx + slopex*vertexTz);
		minxintercept = min(minxintercept, vertexTx - slopex*vertexTz);
	}

	slintf ("maxy: %f\n",maxyintercept);
	slintf ("miny: %f\n",minyintercept);
	slintf ("maxx: %f\n",maxxintercept);
	slintf ("minx: %f\n",minxintercept);
	slintf ("m_rznear: %f\n",m_rznear);
	slintf ("m_rzfar : %f\n",m_rzfar);

	// Find camera center in xy plane

	const GPINFLOAT ydist = (maxyintercept - minyintercept) / (slopey*2.0);
	const GPINFLOAT xdist = (maxxintercept - minxintercept) / (slopex*2.0);
	m_vertexcamera.z = (float)(max(ydist,xdist));
   m_vertexcamera.z += xlatez;
	// changed this since it's the same and better understandable.
	// m_vertexcamera.y = (float)(slopey*ydist + minyintercept);
	m_vertexcamera.y = (float)((maxyintercept - minyintercept)*0.5 + minyintercept);
	//m_vertexcamera.x = (float)(slopex*xdist + minxintercept);
	m_vertexcamera.x = (float)((maxxintercept - minxintercept)*0.5 + minxintercept);

	m_rznear += m_vertexcamera.z;
	m_rzfar += m_vertexcamera.z;

	const GPINFLOAT delta = m_rzfar - m_rznear;

#if 1
	m_rznear -= delta*0.15; // Allow for roundoff error (and tweak the setting too).
	m_rzfar += delta*0.01;
#else
	m_rznear -= delta*0.01; // Allow for roundoff error
	m_rzfar += delta*0.01;
#endif
}

void PinProjection::SetFieldOfView(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar)
{
	// From the Field Of View and far z clipping plane, determine the front clipping plane size
	const GPINFLOAT yrange = rznear * tan(ANGTORAD(rFOV*0.5));
	const GPINFLOAT xrange = yrange * raspect; //width/height

	ZeroMemory(&m_matProj, sizeof(Matrix3D));

	const float Q = (float)(rzfar / ( rzfar - rznear ));

	m_matProj._11 = (float)(rznear / xrange);
	m_matProj._22 = -(float)(rznear / yrange);
	m_matProj._33 = Q;
	m_matProj._34 = 1.0f;
	m_matProj._43 = -Q*(float)rznear;

    m_matView.SetIdentity();
	m_matView._33 = -1.0f;

	m_matWorld.SetIdentity();
}

void PinProjection::CacheTransform()
{
	Matrix3D matT;
	m_matProj.Multiply(m_matView, matT);        // matT = matView * matProj
	matT.Multiply(m_matWorld, m_matrixTotal);   // total = matWorld * matView * matProj
}

void PinProjection::TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex3D * const rgvout) const
{
	const float rClipWidth  = (m_rcviewport.right - m_rcviewport.left)*0.5f;
	const float rClipHeight = (m_rcviewport.bottom - m_rcviewport.top)*0.5f;
	const int xoffset = m_rcviewport.left;
	const int yoffset = m_rcviewport.top;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count;  ++i)
	{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const float x = rgv[l].x;
		const float y = rgv[l].y;
		const float z = rgv[l].z;

		// Transform it through the current matrix set
		const float xp = m_matrixTotal._11*x + m_matrixTotal._21*y + m_matrixTotal._31*z + m_matrixTotal._41;
		const float yp = m_matrixTotal._12*x + m_matrixTotal._22*y + m_matrixTotal._32*z + m_matrixTotal._42;
		const float wp = m_matrixTotal._14*x + m_matrixTotal._24*y + m_matrixTotal._34*z + m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const float inv_wp = 1.0f/wp;
		const float vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const float vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		const float zp = m_matrixTotal._13*x + m_matrixTotal._23*y + m_matrixTotal._33*z + m_matrixTotal._43;
		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
		rgvout[l].z = zp * inv_wp;
		rgvout[l].rhw = wp;
	}
}

//copy pasted from above
void PinProjection::TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const
{
	const float rClipWidth  = (m_rcviewport.right - m_rcviewport.left)*0.5f;
	const float rClipHeight = (m_rcviewport.bottom - m_rcviewport.top)*0.5f;
	const int xoffset = m_rcviewport.left;
	const int yoffset = m_rcviewport.top;

	// Transform each vertex through the current matrix set
	for(int i=0; i<count; ++i)
	{
		const int l = rgi ? rgi[i] : i;

		// Get the untransformed vertex position
		const float x = rgv[l].x;
		const float y = rgv[l].y;
		const float z = rgv[l].z;

		// Transform it through the current matrix set
		const float xp = m_matrixTotal._11*x + m_matrixTotal._21*y + m_matrixTotal._31*z + m_matrixTotal._41;
		const float yp = m_matrixTotal._12*x + m_matrixTotal._22*y + m_matrixTotal._32*z + m_matrixTotal._42;
		const float wp = m_matrixTotal._14*x + m_matrixTotal._24*y + m_matrixTotal._34*z + m_matrixTotal._44;

		// Finally, scale the vertices to screen coords. This step first
		// "flattens" the coordinates from 3D space to 2D device coordinates,
		// by dividing each coordinate by the wp value. Then, the x- and
		// y-components are transformed from device coords to screen coords.
		// Note 1: device coords range from -1 to +1 in the viewport.
		const float inv_wp = 1.0f/wp;
		const float vTx  = ( 1.0f + xp*inv_wp ) * rClipWidth  + xoffset;
		const float vTy  = ( 1.0f - yp*inv_wp ) * rClipHeight + yoffset;

		rgvout[l].x = vTx;
		rgvout[l].y	= vTy;
	}
}

void Matrix3D::Invert()
//void Gauss (RK8 ** a, RK8 ** b, int n)
{
	int ipvt[4];
	for (int i = 0; i < 4; ++i)
		ipvt[i] = i;

	for (int k = 0; k < 4; ++k)
	{
		float temp = 0.f;
		int l = k;
		for (int i = k; i < 4; ++i)
		{
			const float d = fabsf(m[k][i]);
			if (d > temp)
			{
				temp = d;
				l = i;
			}
		}
		if (l != k)
		{
			const int tmp = ipvt[k];
			ipvt[k] = ipvt[l];
			ipvt[l] = tmp;
			for (int j = 0; j < 4; ++j)
			{
				temp = m[j][k];
				m[j][k] = m[j][l];
				m[j][l] = temp;
			}
		}
		const float d = 1.0f / m[k][k];
		for (int j = 0; j < k; ++j)
		{
			const float c = m[j][k] * d;
			for (int i = 0; i < 4; ++i)
				m[j][i] -= m[k][i] * c;
			m[j][k] = c;
		}
		for (int j = k + 1; j < 4; ++j)
		{
			const float c = m[j][k] * d;
			for (int i = 0; i < 4; ++i)
				m[j][i] -= m[k][i] * c;
			m[j][k] = c;
		}
		for (int i = 0; i < 4; ++i)
			m[k][i] = -m[k][i] * d;
		m[k][k] = d;
	}

	Matrix3D mat3D;
	mat3D.m[ipvt[0]][0] = m[0][0]; mat3D.m[ipvt[0]][1] = m[0][1]; mat3D.m[ipvt[0]][2] = m[0][2]; mat3D.m[ipvt[0]][3] = m[0][3];
	mat3D.m[ipvt[1]][0] = m[1][0]; mat3D.m[ipvt[1]][1] = m[1][1]; mat3D.m[ipvt[1]][2] = m[1][2]; mat3D.m[ipvt[1]][3] = m[1][3];
	mat3D.m[ipvt[2]][0] = m[2][0]; mat3D.m[ipvt[2]][1] = m[2][1]; mat3D.m[ipvt[2]][2] = m[2][2]; mat3D.m[ipvt[2]][3] = m[2][3];
	mat3D.m[ipvt[3]][0] = m[3][0]; mat3D.m[ipvt[3]][1] = m[3][1]; mat3D.m[ipvt[3]][2] = m[3][2]; mat3D.m[ipvt[3]][3] = m[3][3];

	m[0][0] = mat3D.m[0][0]; m[0][1] = mat3D.m[0][1]; m[0][2] = mat3D.m[0][2]; m[0][3] = mat3D.m[0][3];
	m[1][0] = mat3D.m[1][0]; m[1][1] = mat3D.m[1][1]; m[1][2] = mat3D.m[1][2]; m[1][3] = mat3D.m[1][3];
	m[2][0] = mat3D.m[2][0]; m[2][1] = mat3D.m[2][1]; m[2][2] = mat3D.m[2][2]; m[2][3] = mat3D.m[2][3];
	m[3][0] = mat3D.m[3][0]; m[3][1] = mat3D.m[3][1]; m[3][2] = mat3D.m[3][2]; m[3][3] = mat3D.m[3][3];
}
