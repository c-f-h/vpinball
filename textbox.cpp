#include "StdAfx.h"

Textbox::Textbox()
{
    m_pIFont = NULL;
    m_texture = NULL;
}

Textbox::~Textbox()
{
    m_pIFont->Release();
}

HRESULT Textbox::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
    m_ptable = ptable;

    const float width  = GetRegStringAsFloatWithDefault("DefaultProps\\TextBox","Width", 100.0f);
    const float height = GetRegStringAsFloatWithDefault("DefaultProps\\TextBox","Height", 50.0f);

    m_d.m_v1.x = x;
    m_d.m_v1.y = y;
    m_d.m_v2.x = x+width;
    m_d.m_v2.y = y+height;

    SetDefaults(fromMouseClick);

    return InitVBA(fTrue, 0, NULL);//ApcProjectItem.Define(ptable->ApcProject, GetDispatch(), axTypeHostProjectItem/*axTypeHostClass*/, L"Textbox", NULL);
}

void Textbox::SetDefaults(bool fromMouseClick)
{
    //Textbox is always located on backdrop
    m_fBackglass = fTrue;

    FONTDESC fd;
    fd.cbSizeofstruct = sizeof(FONTDESC);
    bool free_lpstrName = false;

    if (!fromMouseClick)
    {
        m_d.m_backcolor = RGB(0,0,0);
        m_d.m_fontcolor = RGB(255,255,255);
        m_d.m_tdr.m_fTimerEnabled = false;
        m_d.m_tdr.m_TimerInterval = 100;
        m_d.m_talign = TextAlignRight;
        m_d.m_fTransparent = FALSE;
        lstrcpy(m_d.sztext,"0");

        fd.cySize.int64 = (LONGLONG)(14.25f * 10000.0f);
        fd.lpstrName = L"Arial";
        fd.sWeight = FW_NORMAL;
        fd.sCharset = 0;
        fd.fItalic = 0;
        fd.fUnderline = 0;
        fd.fStrikethrough = 0;
    }
    else
    {
        m_d.m_backcolor = GetRegIntWithDefault("DefaultProps\\TextBox","BackColor", RGB(0,0,0));
        m_d.m_fontcolor = GetRegIntWithDefault("DefaultProps\\TextBox","FontColor", RGB(255,255,255));
        m_d.m_tdr.m_fTimerEnabled = GetRegIntWithDefault("DefaultProps\\TextBox","TimerEnabled", 0) ? true : false;
        m_d.m_tdr.m_TimerInterval = GetRegIntWithDefault("DefaultProps\\TextBox","TimerInterval", 100);
        m_d.m_talign = (TextAlignment)GetRegIntWithDefault("DefaultProps\\TextBox","TextAlignment", TextAlignRight);
        m_d.m_fTransparent = GetRegIntWithDefault("DefaultProps\\TextBox","Transparent", 0);

        const float fontSize = GetRegStringAsFloatWithDefault("DefaultProps\\TextBox","FontSize", 14.25f);
        fd.cySize.int64 = (LONGLONG)(fontSize * 10000.0f);

        char tmp[256];
        HRESULT hr;
        hr = GetRegString("DefaultProps\\TextBox","FontName", tmp, 256);
        if (hr != S_OK)
            fd.lpstrName = L"Arial";
        else
        {
            unsigned int len = strlen(&tmp[0])+1;
            fd.lpstrName = (LPOLESTR) malloc(len*sizeof(WCHAR));
            UNICODE_FROM_ANSI(fd.lpstrName, &tmp[0], len); 
            fd.lpstrName[len] = 0;
            free_lpstrName = true;
        }

        fd.sWeight = GetRegIntWithDefault("DefaultProps\\TextBox", "FontWeight", FW_NORMAL);
        fd.sCharset = GetRegIntWithDefault("DefaultProps\\TextBox", "FontCharSet", 0);
        fd.fItalic = GetRegIntWithDefault("DefaultProps\\TextBox", "FontItalic", 0);
        fd.fUnderline = GetRegIntWithDefault("DefaultProps\\TextBox", "FontUnderline", 0);
        fd.fStrikethrough = GetRegIntWithDefault("DefaultProps\\TextBox", "FontStrikeThrough", 0);

        hr = GetRegString("DefaultProps\\TextBox","Text", m_d.sztext, MAXSTRING);
        if (hr != S_OK)
            lstrcpy(m_d.sztext,"0");
    }

    OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
    if(free_lpstrName)
        free( fd.lpstrName );
}

void Textbox::WriteRegDefaults()
	{
	char strTmp[40];
	float fTmp;
	
	SetRegValue("DefaultProps\\TextBox","BackColor", REG_DWORD, &m_d.m_backcolor, 4);
	SetRegValue("DefaultProps\\TextBox","FontColor", REG_DWORD, &m_d.m_fontcolor, 4);
	SetRegValue("DefaultProps\\TextBox","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
	SetRegValue("DefaultProps\\TextBox","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
	SetRegValue("DefaultProps\\TextBox","FontColor", REG_DWORD, &m_d.m_fontcolor, 4);
	SetRegValue("DefaultProps\\TextBox","Transparent",REG_DWORD,&m_d.m_fTransparent,4);

	FONTDESC fd;
	fd.cbSizeofstruct = sizeof(FONTDESC);
	m_pIFont->get_Size(&fd.cySize); 
	m_pIFont->get_Name(&fd.lpstrName); 
	m_pIFont->get_Weight(&fd.sWeight); 
	m_pIFont->get_Charset(&fd.sCharset); 
	m_pIFont->get_Italic(&fd.fItalic);
	m_pIFont->get_Underline(&fd.fUnderline); 
	m_pIFont->get_Strikethrough(&fd.fStrikethrough); 
	
	fTmp = (float)(fd.cySize.int64 / 10000.0);
	sprintf_s(strTmp, 40, "%f", fTmp);
	SetRegValue("DefaultProps\\TextBox","FontSize", REG_SZ, &strTmp,strlen(strTmp));
	int charCnt = wcslen(fd.lpstrName) +1;
	WideCharToMultiByte(CP_ACP, 0, fd.lpstrName, charCnt, strTmp, 2*charCnt, NULL, NULL);
	SetRegValue("DefaultProps\\TextBox","FontName", REG_SZ, &strTmp,strlen(strTmp));
	SetRegValue("DefaultProps\\TextBox","FontWeight",REG_DWORD,&fd.sWeight,4);
	SetRegValue("DefaultProps\\TextBox","FontCharSet",REG_DWORD,&fd.sCharset,4);
	SetRegValue("DefaultProps\\TextBox","FontItalic",REG_DWORD,&fd.fItalic,4);
	SetRegValue("DefaultProps\\TextBox","FontUnderline",REG_DWORD,&fd.fUnderline,4);
	SetRegValue("DefaultProps\\TextBox","FontStrikeThrough",REG_DWORD,&fd.fStrikethrough,4);

	SetRegValue("DefaultProps\\TextBox","Text", REG_SZ, &m_d.sztext,strlen(m_d.sztext));
	}

STDMETHODIMP Textbox::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_ITextbox,
	};

	for (size_t i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

void Textbox::PreRender(Sur * const psur)
	{
	psur->SetBorderColor(-1,false,0);
	psur->SetFillColor(m_d.m_backcolor);
	psur->SetObject(this);

	psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);
	}

void Textbox::Render(Sur * const psur)
	{
	psur->SetBorderColor(RGB(0,0,0),false,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);
	psur->SetObject(NULL);

	psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);
	}

void Textbox::GetTimers(Vector<HitTimer> * const pvht)
	{
	IEditable::BeginPlay();

	HitTimer *pht;
	pht = new HitTimer();
	pht->m_interval = m_d.m_tdr.m_TimerInterval;
	pht->m_nextfire = pht->m_interval;
	pht->m_pfe = (IFireEvents *)this;

	m_phittimer = pht;

	if (m_d.m_tdr.m_fTimerEnabled)
		{
		pvht->AddElement(pht);
		}
	}

void Textbox::GetHitShapes(Vector<HitObject> * const pvho)
{
}

void Textbox::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Textbox::EndPlay()
{
    if (m_texture)
    {
        delete m_texture;
        m_texture = NULL;

        m_pIFontPlay->Release();
    }

    IEditable::EndPlay();
}

void Textbox::PostRenderStatic(const RenderDevice* _pd3dDevice)
{
    TRACE_FUNCTION();

    if (!m_texture)
        return;

    RenderDevice* pd3dDevice = (RenderDevice*)_pd3dDevice;
    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

    pd3dDevice->SetRenderState(RenderDevice::ZENABLE, FALSE);
    pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);

    Material mat;
    mat.setColor( 1.0f, 1.0f, 1.0f, 1.0f );
    pd3dDevice->SetMaterial(mat);

    // Set texture to mirror, so the alpha state of the texture blends correctly to the outside
    pd3dDevice->SetTextureAddressMode(ePictureTexture, RenderDevice::TEX_MIRROR);

    ppin3d->SetBaseTexture(ePictureTexture, m_texture);

    ppin3d->SetTextureFilter(ePictureTexture, TEXTURE_MODE_BILINEAR);
    ppin3d->EnableAlphaTestReference(0x80);

    pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_TFACTOR); // default tfactor: 1,1,1,1

    pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, MY_D3DTRANSFORMED_NOTEX2_VERTEX, rgv3D, 4);

    // reset render state
    ppin3d->DisableAlphaBlend();
    pd3dDevice->SetTexture(ePictureTexture, NULL);
    ppin3d->SetTextureFilter(ePictureTexture, TEXTURE_MODE_TRILINEAR);
    pd3dDevice->SetTextureAddressMode(ePictureTexture, RenderDevice::TEX_WRAP);
    pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
    pd3dDevice->SetRenderState(RenderDevice::ZENABLE, TRUE);
}

void Textbox::RenderSetup(const RenderDevice* _pd3dDevice)
{
    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

    const float left   = min(m_d.m_v1.x, m_d.m_v2.x);
    const float right  = max(m_d.m_v1.x, m_d.m_v2.x);
    const float top    = min(m_d.m_v1.y, m_d.m_v2.y);
    const float bottom = max(m_d.m_v1.y, m_d.m_v2.y);

    m_rect.left   = (int)left;
    m_rect.top    = (int)top;
    m_rect.right  = (int)right;
    m_rect.bottom = (int)bottom;

    rgv3D[0].x = left;
    rgv3D[0].y = top;
    rgv3D[0].tu = 0;
    rgv3D[0].tv = 0;

    rgv3D[1].x = right;
    rgv3D[1].y = top;
    rgv3D[1].tu = 1;
    rgv3D[1].tv = 0;

    rgv3D[2].x = right;
    rgv3D[2].y = bottom;
    rgv3D[2].tu = 1;
    rgv3D[2].tv = 1;

    rgv3D[3].x = left;
    rgv3D[3].y = bottom;
    rgv3D[3].tu = 0;
    rgv3D[3].tv = 1;

    SetHUDVertices(rgv3D, 4);

    rgv3D[0].z = rgv3D[1].z = rgv3D[2].z = rgv3D[3].z = 1.0f;
    rgv3D[0].rhw = rgv3D[1].rhw = rgv3D[2].rhw = rgv3D[3].rhw = 1.0f;

    m_pIFont->Clone(&m_pIFontPlay);

    CY size;
    m_pIFontPlay->get_Size(&size);
    // I choose 912 because that was the original playing size I tested with,
    // and this way I don't have to change my tables
    size.int64 = size.int64 / 1.5f*ppin3d->m_dwRenderHeight * ppin3d->m_dwRenderWidth;
    m_pIFontPlay->put_Size(size);

    RenderText();
}

void Textbox::RenderStatic(const RenderDevice* pd3dDevice)
{
}
	
void Textbox::RenderText()
{
    Pin3D *const ppin3d = &g_pplayer->m_pin3d;

    const int width = m_rect.right - m_rect.left;
    const int height = m_rect.bottom - m_rect.top;

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;

    void *bits;
    HBITMAP hbm = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    assert(hbm);

    HDC hdc = CreateCompatibleDC(NULL);
    HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hbm);

    {
        HBRUSH hbrush = CreateSolidBrush(m_d.m_backcolor);
        HBRUSH hbrushold = (HBRUSH)SelectObject(hdc, hbrush);
        PatBlt(hdc, 0, 0, width, height, PATCOPY);
        SelectObject(hdc, hbrushold);
        DeleteObject(hbrush);
    }
    HFONT hFont;
    m_pIFontPlay->get_hFont(&hFont);
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
    SetTextColor(hdc, m_d.m_fontcolor);
    SetBkMode(hdc, TRANSPARENT);
    SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP);

    int alignment;
    switch (m_d.m_talign)
    {
        case TextAlignLeft:
            alignment = DT_LEFT;
            break;

        default:
        case TextAlignCenter:
            alignment = DT_CENTER;
            break;

        case TextAlignRight:
            alignment = DT_RIGHT;
            break;
    }

    int border = (4 * ppin3d->m_dwRenderWidth) / EDITOR_BG_WIDTH;
    RECT rcOut;

    rcOut.left = border;
    rcOut.top = border;
    rcOut.right = width - border * 2;
    rcOut.bottom = height - border * 2;

    DrawText(hdc, m_d.sztext, lstrlen(m_d.sztext), &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK );

    GdiFlush();     // make sure everything is drawn

    if (!m_texture)
        m_texture = new MemTexture(m_rect.right - m_rect.left, m_rect.bottom - m_rect.top);
    m_texture->CopyBits(bits);
    Texture::SetOpaque(m_texture);
    if (m_d.m_fTransparent)
        Texture::SetAlpha(m_texture, COLORREF_to_D3DCOLOR(m_d.m_backcolor));

    ppin3d->m_pd3dDevice->m_texMan.SetDirty(m_texture);

    SelectObject(hdc, oldFont);
    SelectObject(hdc, oldBmp);
    DeleteDC(hdc);
    DeleteObject(hbm);
}

void Textbox::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_v1.x, m_d.m_v1.y);
	}

void Textbox::MoveOffset(const float dx, const float dy)
	{
	m_d.m_v1.x += dx;
	m_d.m_v1.y += dy;

	m_d.m_v2.x += dx;
	m_d.m_v2.y += dy;

	m_ptable->SetDirtyDraw();
	}

void Textbox::GetCenter(Vertex2D * const pv) const
{
	*pv = m_d.m_v1;
}

void Textbox::PutCenter(const Vertex2D * const pv)
{
	m_d.m_v2.x = pv->x + m_d.m_v2.x - m_d.m_v1.x;
	m_d.m_v2.y = pv->y + m_d.m_v2.y - m_d.m_v1.y;

	m_d.m_v1 = *pv;

	m_ptable->SetDirtyDraw();
}

STDMETHODIMP Textbox::get_BackColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_backcolor;

	return S_OK;
}

STDMETHODIMP Textbox::put_BackColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_backcolor = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_FontColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_fontcolor;

	return S_OK;
}

STDMETHODIMP Textbox::put_FontColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_fontcolor = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_Text(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, (char *)m_d.sztext, -1, wz, 512);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Textbox::put_Text(BSTR newVal)
{
	if (lstrlenW(newVal) < 512)
		{
		STARTUNDO

		WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.sztext, 512, NULL, NULL);

		if (g_pplayer)
			RenderText();

		STOPUNDO
		}

	return S_OK;
}

HRESULT Textbox::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VER1), &m_d.m_v1, sizeof(Vertex2D));
	bw.WriteStruct(FID(VER2), &m_d.m_v2, sizeof(Vertex2D));
	bw.WriteInt(FID(CLRB), m_d.m_backcolor);
	bw.WriteInt(FID(CLRF), m_d.m_fontcolor);
	bw.WriteString(FID(TEXT), m_d.sztext);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
	bw.WriteInt(FID(ALGN), m_d.m_talign);
	bw.WriteBool(FID(TRNS), m_d.m_fTransparent);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(FONT));
	IPersistStream * ips;
	m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);
	HRESULT hr;
	hr = ips->Save(pstm, TRUE);

	bw.WriteTag(FID(ENDB));

	return S_OK;
	}

HRESULT Textbox::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
	SetDefaults(false);

	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
}

BOOL Textbox::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(VER1))
		{
		pbr->GetStruct(&m_d.m_v1, sizeof(Vertex2D));
		}
	else if (id == FID(VER2))
		{
		pbr->GetStruct(&m_d.m_v2, sizeof(Vertex2D));
		}
	else if (id == FID(CLRB))
		{
		pbr->GetInt(&m_d.m_backcolor);
		}
	else if (id == FID(CLRF))
		{
		pbr->GetInt(&m_d.m_fontcolor);
		}
	else if (id == FID(TMON))
		{
		pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
		}
	else if (id == FID(TMIN))
		{
		pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
		}
	else if (id == FID(TEXT))
		{
		pbr->GetString(m_d.sztext);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(ALGN))
		{
		pbr->GetInt(&m_d.m_talign);
		}
	else if (id == FID(TRNS))
		{
		pbr->GetBool(&m_d.m_fTransparent);
		}
	else if (id == FID(FONT))
		{
		if (!m_pIFont)
			{
			FONTDESC fd;
			fd.cbSizeofstruct = sizeof(FONTDESC);
			fd.lpstrName = L"Arial";
			fd.cySize.int64 = 142500;
			//fd.cySize.Lo = 0;
			fd.sWeight = FW_NORMAL;
			fd.sCharset = 0;
			fd.fItalic = 0;
			fd.fUnderline = 0;
			fd.fStrikethrough = 0;
			OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
			}

		IPersistStream * ips;
		m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);

		ips->Load(pbr->m_pistream);
		}
	else
		{
		ISelect::LoadToken(id, pbr);
		}

	return fTrue;
	}

HRESULT Textbox::InitPostLoad()
	{
	m_texture = NULL;

	return S_OK;
	}

STDMETHODIMP Textbox::get_Font(IFontDisp **pVal)
{
	m_pIFont->QueryInterface(IID_IFontDisp, (void **)pVal);

	return S_OK;
}

STDMETHODIMP Textbox::put_Font(IFontDisp *newVal)
{
	// Does anybody use this way of setting the font?  Need to add to idl file.
	return S_OK;
}

STDMETHODIMP Textbox::putref_Font(IFontDisp* pFont)
	{
	//We know that our own property browser gives us the same pointer

	//m_pIFont->Release();
	//pFont->QueryInterface(IID_IFont, (void **)&m_pIFont);

	SetDirtyDraw();
	return S_OK;
	}

STDMETHODIMP Textbox::get_Width(float *pVal)
{
	*pVal = m_d.m_v2.x - m_d.m_v1.x;

	return S_OK;
}

STDMETHODIMP Textbox::put_Width(float newVal)
{
	STARTUNDO

	m_d.m_v2.x = m_d.m_v1.x + newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_Height(float *pVal)
{
	*pVal = m_d.m_v2.y - m_d.m_v1.y;

	return S_OK;
}

STDMETHODIMP Textbox::put_Height(float newVal)
{
	STARTUNDO

	m_d.m_v2.y = m_d.m_v1.y + newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_X(float *pVal)
{
	*pVal = m_d.m_v1.x;

	return S_OK;
}

STDMETHODIMP Textbox::put_X(float newVal)
{
	STARTUNDO

	float delta = newVal - m_d.m_v1.x;

	m_d.m_v1.x += delta;
	m_d.m_v2.x += delta;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_Y(float *pVal)
{
	*pVal = m_d.m_v1.y;

	return S_OK;
}

STDMETHODIMP Textbox::put_Y(float newVal)
{
	STARTUNDO

	float delta = newVal - m_d.m_v1.y;

	m_d.m_v1.y += delta;
	m_d.m_v2.y += delta;

	STOPUNDO

	return S_OK;
}

void Textbox::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPTEXTBOX_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPTEXTBOX_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPTEXTBOX_STATE, IDS_STATE);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}

STDMETHODIMP Textbox::get_Alignment(TextAlignment *pVal)
{
	*pVal = m_d.m_talign;

	return S_OK;
}

STDMETHODIMP Textbox::put_Alignment(TextAlignment newVal)
{
	STARTUNDO

	m_d.m_talign = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_IsTransparent(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_fTransparent);

	return S_OK;
}

STDMETHODIMP Textbox::put_IsTransparent(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fTransparent = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}
