#include "StdAfx.h"

#define DISPID_FAKE_NAME 30000

enum
	{
	eNotControl,
	eEdit,
	eButton,
	eCombo,
	eColor,
	eFont,
	};

#define EXPANDO_EXPAND			WM_USER+100
#define EXPANDO_COLLAPSE		WM_USER+101

#define EXPANDO_X_OFFSET 0 /*3*/
#define EXPANDO_Y_OFFSET 30

#define EXPANDOHEIGHT 18

LRESULT CALLBACK SBFrameProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ColorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FontProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ExpandoProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

SmartBrowser::SmartBrowser()
	{
	m_hwndFrame = NULL;
	m_olddialog = -1;
	m_pvsel = NULL;
	m_maxdialogwidth = 20;
	}

SmartBrowser::~SmartBrowser()
	{
	if (m_pvsel)
		{
		delete m_pvsel;
		}

	DestroyWindow(m_hwndFrame);
	DeleteObject(m_hfontHeader);
	
	FreePropPanes();
	}

void SmartBrowser::Init(HWND hwndParent)
	{
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_DBLCLKS;//CS_NOCLOSE | CS_OWNDC;
	wcex.lpfnWndProc = (WNDPROC) SBFrameProc;
	wcex.hInstance = g_hinst;
	wcex.hIcon = NULL;
	wcex.lpszClassName = "SBFrame";
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = NULL;
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	RegisterClassEx(&wcex);

	m_hwndFrame = CreateWindowEx(0, "SBFrame", "SBFrame",
			WS_CHILD | WS_BORDER,
			10, 0, eSmartBrowserWidth, 500, hwndParent, NULL, g_hinst, this);

	m_hfontHeader = CreateFont(-18, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");

	// Register expando window
	wcex.style = CS_DBLCLKS | CS_NOCLOSE; //| CS_OWNDC;
	wcex.lpfnWndProc = (WNDPROC) ExpandoProc;
	wcex.lpszClassName = "ExpandoControl";
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = NULL;
	wcex.hbrBackground = NULL;
	RegisterClassEx(&wcex);

	// Register custom controls
	wcex.style = CS_DBLCLKS | CS_NOCLOSE; //| CS_OWNDC;
	wcex.lpfnWndProc = (WNDPROC) ColorProc;
	wcex.lpszClassName = "ColorControl";
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = NULL;
	wcex.hbrBackground = NULL;
	atom = RegisterClassEx(&wcex);

	// Register custom controls
	wcex.lpfnWndProc = (WNDPROC) FontProc;
	wcex.lpszClassName = "FontControl";
	RegisterClassEx(&wcex);

	m_szHeader[0] = '\0';
	}

void SmartBrowser::FreePropPanes()
	{
	for (int i=0;i<m_vproppane.Size();i++)
		{
		delete m_vproppane.ElementAt(i);
		}

	m_vproppane.RemoveAllElements();
	}

void SmartBrowser::CreateFromDispatch(HWND hwndParent, Vector<ISelect> *pvsel)
	{
	//int resourceid;
	ISelect *pisel = NULL;
	FreePropPanes();

	if (pvsel != NULL)
		{
		ItemTypeEnum maintype = pvsel->ElementAt(0)->GetItemType();
		bool fSame = true;

		// See if all items in multi-select are of the same type.
		// If not, we can't edit their collective properties
		for (int i=1;i<pvsel->Size();i++)
			{
			if (pvsel->ElementAt(i)->GetItemType() != maintype)
				{
				PropertyPane *pproppane = new PropertyPane(IDD_PROPMULTI, NULL);
				m_vproppane.AddElement(pproppane);
				//resourceid = IDD_PROPMULTI;
				fSame = false;
				}
			}

		if (fSame)
			{
			pisel = pvsel->ElementAt(0);
			pisel->GetDialogPanes(&m_vproppane);
			//resourceid = pisel->GetDialogID();
			}
		}

	const int propID = (m_vproppane.Size() > 0) ? m_vproppane.ElementAt(0)->dialogid : -1;

	// Optimized for selecting the same object
	// Have to check resourceid too, since there can be more than one dialog view of the same object (table/backdrop)
	if (pvsel && m_pvsel && (pvsel->Size() == m_pvsel->Size()) /*m_pisel == pisel*/ && (m_olddialog == propID))
		{
		bool fSame = fTrue;
		for (int i=0;i<pvsel->Size();i++)
			{
			if (pvsel->ElementAt(i) != m_pvsel->ElementAt(i))
				{
				fSame = false;
				break;
				}
			}
		if (fSame)
			{
			return;
			}
		}

	m_olddialog = propID;

	if (m_vhwndExpand.Size() > 0)
		{
		// Stop focus from going to no-man's land if focus is in dialog
		HWND hwndFocus = GetFocus();
		do
			{
			if (hwndFocus == m_hwndFrame)
				{
				SetFocus(hwndParent);
				}
			}
			while ((hwndFocus = GetParent(hwndFocus)) != NULL);

		for (int i=0;i<m_vhwndExpand.Size();i++)
			{
			DestroyWindow(m_vhwndExpand.ElementAt(i));
			}
		m_vhwndExpand.RemoveAllElements();
		m_vhwndDialog.RemoveAllElements(); // Dialog windows will have been destroyed along with their parent expando window
		}

	if (m_pvsel)
		{
		delete m_pvsel;
		m_pvsel = NULL;
		}

	if (pvsel)
		{
		m_pvsel = new Vector<ISelect>();
		pvsel->Clone(m_pvsel);
		}

	if (m_vproppane.Size() == 0)
		{
		m_szHeader[0] = '\0';
		InvalidateRect(m_hwndFrame, NULL, fTrue);
		return;
		}

	if (pisel)
		{
		CComBSTR bstr;
		pisel->GetTypeName(&bstr);

		char szTemp[64];

		WideCharToMultiByte(CP_ACP, 0, bstr, -1, szTemp, 64, NULL, NULL);

		//char szNum[64];
		if (pvsel->Size() > 1)
			{
			sprintf(m_szHeader, "%s(%d)", szTemp, pvsel->Size());
			}
		else
			{
			lstrcpy(m_szHeader, szTemp);
			}
		}
	else
		{
		m_szHeader[0] = '\0';
		}

	InvalidateRect(m_hwndFrame, NULL, fTrue);

	for (int i=0;i<m_vproppane.Size();i++)
		{
		PropertyPane * const pproppane = m_vproppane.ElementAt(i);
		ExpandoInfo * const pexinfo = new ExpandoInfo();
		pexinfo->m_id = i;
		pexinfo->m_fExpanded = fFalse;
		pexinfo->m_psb = this;

		char *szCaption;
		if (pproppane->titlestringid)
			{
			LocalString ls(pproppane->titlestringid);
			szCaption = ls.m_szbuffer;
			pexinfo->m_fHasCaption = fTrue;
			}
		else
			{
			szCaption = "";
			pexinfo->m_fHasCaption = fFalse;
			}

		HWND hwndExpand = CreateWindowEx(WS_EX_TOOLWINDOW, "ExpandoControl", szCaption,
			WS_CHILD /*| WS_VISIBLE /*| WS_BORDER*/,
			2, EXPANDO_Y_OFFSET, eSmartBrowserWidth-5, 300, m_hwndFrame, NULL, g_hinst, pexinfo);

		m_vhwndExpand.AddElement(hwndExpand);

		HWND hwndDialog;
		if (pproppane->dialogid != 0)
			{
			hwndDialog = CreateDialogParam(g_hinstres, MAKEINTRESOURCE(pproppane->dialogid),
				hwndExpand, PropertyProc, (long)this);
			}
		else
			{
			hwndDialog = CreateDialogIndirectParam(g_hinstres, pproppane->ptemplate,
				hwndExpand, PropertyProc, (long)this);
			}

		m_vhwndDialog.AddElement(hwndDialog);

		RECT rcDialog;
		GetWindowRect(hwndDialog, &rcDialog);
		pexinfo->m_dialogheight = rcDialog.bottom - rcDialog.top;

		// A little hack - if we have multi-select, we know the Name property
		// can never be use.  Disable it to make that easy to understand for the
		// user.
		if (m_pvsel->Size() > 1)
			{
			HWND hwndName = GetDlgItem(hwndDialog, DISPID_FAKE_NAME);
			if (hwndName)
				{
				EnableWindow(hwndName, FALSE);
				}
			}
		}

	LayoutExpandoWidth();

	for	(int i=0;i<m_vhwndExpand.Size();i++)
	{
		SendMessage(m_vhwndExpand.ElementAt(i), EXPANDO_EXPAND, 0, 0);
		ShowWindow(m_vhwndExpand.ElementAt(i), SW_SHOWNOACTIVATE);
	}

	// expand bottom last
	//for (int i=0;i<m_vhwndExpand.Size();i++) SendMessage(m_vhwndExpand.ElementAt(i), EXPANDO_EXPAND, 1, 0); 

	//expand top last
	for	(int i= m_vhwndExpand.Size()-1; i >= 0;--i) SendMessage(m_vhwndExpand.ElementAt(i), EXPANDO_EXPAND, 1, 0);
}

BOOL CALLBACK EnumChildInitList(HWND hwnd, LPARAM lParam)
	{
	SmartBrowser *const psb = (SmartBrowser *)lParam;

	char szName[256];
	GetClassName(hwnd, szName, 256);

	int type = eNotControl;

	if (!strcmp(szName, "ComboBox"))
		{
		type = eCombo;
		}

	if (type == eNotControl)
		{
		return TRUE;
		}

	int dispid = GetDlgCtrlID(hwnd);

		//case eCombo:
			{
			CALPOLESTR     castr;
			CADWORD        cadw;
			IPerPropertyBrowsing *pippb;
			char szT[512];

			psb->GetBaseIDisp()->QueryInterface(IID_IPerPropertyBrowsing, (void **)&pippb);

			BOOL fGotStrings = fFalse;

			if (pippb)
				{
				HRESULT hr = pippb->GetPredefinedStrings(dispid, &castr, &cadw);

				if (hr == S_OK)
					{
					fGotStrings = fTrue;

					SetWindowLong(hwnd, GWL_USERDATA, 0); // So we know later whether to set the property as a string or a number from itemdata

					SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

					for (ULONG i=0;i<castr.cElems;i++)
						{
						WideCharToMultiByte(CP_ACP, 0, castr.pElems[i], -1, szT, 512, NULL, NULL);
						const int index = SendMessage(hwnd, CB_ADDSTRING, 0, (int)szT);
						SendMessage(hwnd, CB_SETITEMDATA, index, (DWORD)cadw.pElems[i]);
						}

					CoTaskMemFree((void *)cadw.pElems);

					for (ULONG i=0; i < castr.cElems; i++)
						CoTaskMemFree((void *)castr.pElems[i]);

					CoTaskMemFree((void *)castr.pElems);
					}

				pippb->Release();
				}

			if (!fGotStrings)
				{
				SetWindowLong(hwnd, GWL_USERDATA, 1); // So we know later whether to set the property as a string or a number from itemdata

				ITypeInfo *piti;
				psb->GetBaseIDisp()->GetTypeInfo(0, 0x409, &piti);

				TYPEATTR *pta;
				piti->GetTypeAttr(&pta);

				const int cfunc = pta->cFuncs;

				for (int i=0;i<cfunc;i++)
					{
					FUNCDESC *pfd;
					piti->GetFuncDesc(i, &pfd);

					if (pfd->memid == dispid && pfd->invkind == INVOKE_PROPERTYGET)
						{
						// We found the function that this
						// dialog control references
						// Figure out what type the get function returns -
						// Probably an enum (since that's all this supports for now)
						// Then get the TypeInfo for the enum and loop
						// through the names and values, and add them to
						// the combo box
						ITypeInfo *pitiEnum;
						piti->GetRefTypeInfo(pfd->elemdescFunc.tdesc.hreftype, &pitiEnum);
						if (pitiEnum)
							{
							TYPEATTR *ptaEnum;
							pitiEnum->GetTypeAttr(&ptaEnum);

							const int cenum = ptaEnum->cVars;

							for (int l=0;l<cenum;l++)
								{
								VARDESC *pvd;
								pitiEnum->GetVarDesc(l, &pvd);

								// Get Name
									{
									BSTR * const rgstr = (BSTR *) CoTaskMemAlloc(6 * sizeof(BSTR *));

									unsigned int cnames;
									/*const HRESULT hr =*/ pitiEnum->GetNames(pvd->memid, rgstr, 6, &cnames);

									// Add enum string to combo control
									WideCharToMultiByte(CP_ACP, 0, rgstr[0], -1, szT, 512, NULL, NULL);
									const int index = SendMessage(hwnd, CB_ADDSTRING, 0, (int)szT);
									SendMessage(hwnd, CB_SETITEMDATA, index, V_I4(pvd->lpvarValue));

									for (unsigned int i2=0; i2 < cnames; i2++)
										{
										SysFreeString(rgstr[i2]);
										}

									CoTaskMemFree(rgstr);
									}

								pitiEnum->ReleaseVarDesc(pvd);
								}

							piti->ReleaseTypeAttr(ptaEnum);

							pitiEnum->Release();
							}
						}

					piti->ReleaseFuncDesc(pfd);
					}

				piti->ReleaseTypeAttr(pta);

				piti->Release();
				}
			}

	return TRUE;
	}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
	{
	SmartBrowser * const psb = (SmartBrowser *)lParam;

	psb->GetControlValue(hwnd);

	return TRUE;
	}

void SmartBrowser::PopulateDropdowns()
	{
	for (int i=0;i<m_vhwndDialog.Size();i++)
		{
		EnumChildWindows(m_vhwndDialog.ElementAt(i), EnumChildInitList, (long)this);
		}
	}

void SmartBrowser::RefreshProperties()
	{
	for (int i=0;i<m_vhwndDialog.Size();i++)
		{
		EnumChildWindows(m_vhwndDialog.ElementAt(i), EnumChildProc, (long)this);
		}
	}

void SmartBrowser::SetVisible(BOOL fVisible)
	{
	ShowWindow(m_hwndFrame, fVisible ? SW_SHOW : SW_HIDE);
	}

BOOL SmartBrowser::GetVisible()
	{
	return IsWindowVisible(m_hwndFrame);
	}

void SmartBrowser::DrawHeader(HDC hdc)
	{
	char szText[256];
	HFONT hfontOld;

	strcpy(szText, m_szHeader);

	hfontOld = (HFONT)SelectObject(hdc, m_hfontHeader);

	SetTextAlign(hdc, TA_CENTER);

	SetBkMode(hdc, TRANSPARENT);

	ExtTextOut(hdc, m_maxdialogwidth>>1, 0, 0, NULL, szText, strlen(szText), NULL);

	SelectObject(hdc, hfontOld);
	}

void SmartBrowser::GetControlValue(HWND hwndControl)
	{
	bool fNinch = false;
	char szName[256];
	GetClassName(hwndControl, szName, 256);
	IDispatch * const pdisp = GetBaseIDisp();

	int type = eNotControl;

	if (!strcmp(szName, "Edit"))
		{
		type = eEdit;

		HWND hwndParent = GetParent(hwndControl);
		char szParentName[256];
		GetClassName(hwndParent, szParentName, 256);
		if (!strcmp(szParentName, "ComboBox"))
			{
			type = eNotControl; // Ignore edit boxes which are part of a combo-box
			}
		}
	else if (!strcmp(szName, "Button"))
		{
		type = eButton;
		}
	else if (!strcmp(szName, "ComboBox"))
		{
		type = eCombo;
		}
	else if (!strcmp(szName, "ColorControl"))
		{
		type = eColor;
		}
	else if (!strcmp(szName, "FontControl"))
		{
		type = eFont;
		}

	if (type == eNotControl)
		{
		return;
		}

	DISPPARAMS dispparams  = {
	NULL,
	NULL,
	0,
	0
	};

	// Get value of first object in multi-select
	// If there is only one object in list, we just end up using that value

	DISPID dispid = GetDlgCtrlID(hwndControl);

	// We use a fake name id in the property browser since the official OLE
	// name id is greater than 0xffff, which doesn't work as a
	// dialog control id on Win9x.
	if (dispid == DISPID_FAKE_NAME)
		{
		dispid = 0x80010000;
		}

	CComVariant var;
	HRESULT hr = pdisp->Invoke(
		dispid, IID_NULL,
		LOCALE_USER_DEFAULT,
		DISPATCH_PROPERTYGET,
		&dispparams, &var, NULL, NULL);

	// Check each selection in a multiple selection and see if everything
	// has the same value.  If not, set a ninched state to the control
	for (int i=1;i<m_pvsel->Size();i++)
		{
		CComVariant varCheck;

		hr = m_pvsel->ElementAt(i)->GetDispatch()->Invoke(
			dispid, IID_NULL,
			LOCALE_USER_DEFAULT,
			DISPATCH_PROPERTYGET,
			&dispparams, &varCheck, NULL, NULL);

		HRESULT hrEqual;
		if (var.vt == 19)
			{
			// Special case OLE_COLOR because the built-in variant
			// comparer doesn't know how to deal with it
			hrEqual = (var.lVal == varCheck.lVal) ? VARCMP_EQ : VARCMP_LT;
			}
		else
			{
			hrEqual = VarCmp(&var, &varCheck, 0x409, 0);
			}

		if (hrEqual != VARCMP_EQ)
			{
			fNinch = true;
			break;
			}
		}

	switch (type)
		{
		case eEdit:
			{
			if (!fNinch)
				{
				VariantChangeType(&var, &var, 0, VT_BSTR);

				WCHAR *wzT;
				wzT = V_BSTR(&var);

				char szT[512+1];

				WideCharToMultiByte(CP_ACP, 0, wzT, -1, szT, 512, NULL, NULL);

				SetWindowText(hwndControl, szT);
				}
			else
				{
				SetWindowText(hwndControl, "");
				}
			}
			break;

		case eButton:
			{
			if (!fNinch)
				{
				VariantChangeType(&var, &var, 0, VT_BOOL);

				BOOL fCheck = V_BOOL(&var);

				SendMessage(hwndControl, BM_SETCHECK, fCheck ? BST_CHECKED : BST_UNCHECKED, 0);
				}
			else
				{
				SendMessage(hwndControl, BM_SETCHECK, BST_UNCHECKED, 0);
				}
			}
			break;

		case eColor:
			{
			VariantChangeType(&var, &var, 0, VT_I4);
			if (!fNinch)
				{
				SendMessage(hwndControl, CHANGE_COLOR, 0, V_I4(&var));
				}
			else
				{
				SendMessage(hwndControl, CHANGE_COLOR, 1, V_I4(&var));
				}
			}
			break;

		case eFont:
			{
			VariantChangeType(&var, &var, 0, VT_I4);
			if (!fNinch)
				{
				SendMessage(hwndControl, CHANGE_FONT, 0, (long)V_DISPATCH(&var));
				}
			else
				{
				SendMessage(hwndControl, CHANGE_FONT, 1, (long)V_DISPATCH(&var));
				}
			}
			break;

		case eCombo:
			{
			if (!fNinch)
				{
				char szT[512];

				const int style = GetWindowLong(hwndControl, GWL_STYLE);

				VariantChangeType(&var, &var, 0, VT_BSTR);

				WCHAR *wzT;
				wzT = V_BSTR(&var);

				WideCharToMultiByte(CP_ACP, 0, wzT, -1, szT, 512, NULL, NULL);

				if (style & CBS_DROPDOWNLIST)
					{
					// Entry must be on the list - search for existing match

					const int index = SendMessage(hwndControl, CB_FINDSTRINGEXACT, ~0u,
							(LPARAM)szT);

					if (index == CB_ERR)
						{
						// No string - look for numerical match in itemdata
						VariantChangeType(&var, &var, 0, VT_INT);

						const int data = V_INT(&var);
						const int citems = SendMessage(hwndControl, CB_GETCOUNT, 0, 0);

						for (int i=0;i<citems;i++)
							{
							const int matchdata = SendMessage(hwndControl, CB_GETITEMDATA, i, 0);

							if (data == matchdata)
								{
								SendMessage(hwndControl, CB_SETCURSEL, i, 0);
								break;
								}
							}
						}
					else
						{
						SendMessage(hwndControl, CB_SETCURSEL, index, 0);
						}
					}
				else
					{
					SendMessage(hwndControl, WM_SETTEXT, 0, (LPARAM)szT);
					}
				} // !fNinch
			}
			break;
		}
	}

void SmartBrowser::SetProperty(int dispid, VARIANT *pvar, BOOL fPutRef)
	{
	DISPID mydispid = DISPID_PROPERTYPUT;
	DISPPARAMS disp;
	disp.cNamedArgs = 1;
	disp.rgdispidNamedArgs = &mydispid;//NULL;
	disp.cArgs = 1;
	disp.rgvarg = pvar;

	for (int i=0;i<m_pvsel->Size();i++)
		{
		/*const HRESULT hr =*/ m_pvsel->ElementAt(i)->GetDispatch()->Invoke(dispid,
				IID_NULL,
				LOCALE_USER_DEFAULT,
				fPutRef ? DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT,
				&disp,
				NULL, NULL, NULL);
		}
	}

void SmartBrowser::LayoutExpandoWidth()
	{
	int maxwidth = 20; // Just in case we have a weird situation where there are no dialogs
	for (int i=0;i<m_vhwndDialog.Size();i++)
		{
		HWND hwnd = m_vhwndDialog.ElementAt(i);
		RECT rc;
		GetWindowRect(hwnd, &rc);
		maxwidth = max(maxwidth, (rc.right - rc.left));
		}

	for (int i=0;i<m_vhwndExpand.Size();i++)
		{
		HWND hwndExpand = m_vhwndExpand.ElementAt(i);
		SetWindowPos(hwndExpand, NULL, 0, 0, maxwidth, 20, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);
		}

	m_maxdialogwidth = maxwidth + EXPANDO_X_OFFSET*2 + 1;

	SendMessage(g_pvp->m_hwnd, WM_SIZE, 0, 0);
	}

void SmartBrowser::RelayoutExpandos()
	{
	int totalheight = 0;
	for (int i=0;i<m_vhwndExpand.Size();i++)
		{
		HWND hwndExpand = m_vhwndExpand.ElementAt(i);
		ExpandoInfo *pexinfo = (ExpandoInfo *)GetWindowLong(hwndExpand, GWL_USERDATA);
		if (pexinfo->m_fExpanded)
			{
			totalheight += pexinfo->m_dialogheight;
			}
		totalheight += EXPANDOHEIGHT;
		}

	RECT rcFrame;
	GetWindowRect(m_hwndFrame, &rcFrame);
	const int maxheight = (rcFrame.bottom - rcFrame.top) - EXPANDO_Y_OFFSET;
	while (totalheight > maxheight)
		{
		// The total height of our expandos is taller than our window
		// We have to shrink some
		int indexBest = -1;
		int prioBest = 0xffff;
		int cnt = 0;

		// Loop through all expandos.  Find the one which is currently expanded
		// and is the lowest on the priority list (the one that was opened the
		// longest time ago)
		// If nothing has ever been expanded by the user, higher panels will
		// get priority over lower panels
		for (int i=0;i<m_vhwndExpand.Size();i++)
			{			
			const int titleid = m_vproppane.ElementAt(i)->titlestringid;
			if (titleid != NULL)
				{
				HWND hwndExpand = m_vhwndExpand.ElementAt(i);
				RECT rc;
				GetWindowRect(hwndExpand, &rc);
				if ((rc.bottom - rc.top) > EXPANDOHEIGHT)
					{
					int prio = m_vproppriority.IndexOf(titleid);
					cnt++;
					if (prio != -1 && prio <= prioBest) //
						{
						prioBest = prio;
						indexBest = i;
						}
					}
				}
			}
		
		if (indexBest != -1 && cnt > 1) //oldest  only the one we expanded
			{	
			RECT rc;
			HWND hwndExpand = m_vhwndExpand.ElementAt(indexBest);
			GetWindowRect(hwndExpand, &rc);
			SendMessage(hwndExpand, EXPANDO_COLLAPSE, 0, 0);
			totalheight -= (rc.bottom - rc.top) - EXPANDOHEIGHT;
			}
		else break; // Ugh?  No expandos? Or one panel is larger than maximum.
		}

	totalheight = 0;
	for (int i=0;i<m_vhwndExpand.Size();i++)
		{
		HWND hwndExpand = m_vhwndExpand.ElementAt(i);
		SetWindowPos(hwndExpand, NULL, EXPANDO_X_OFFSET, totalheight + EXPANDO_Y_OFFSET, 0, 0, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
		RECT rc;
		GetWindowRect(hwndExpand, &rc);
		totalheight += (rc.bottom - rc.top);
		}
	}

void SmartBrowser::ResetPriority(int expandoid)
	{
	// base prioritys on the title of the property pane
	const int titleid = m_vproppane.ElementAt(expandoid)->titlestringid;	
	m_vproppriority.RemoveElement(titleid); // Remove this element if it currently exists in our current priority chain
	m_vproppriority.AddElement(titleid); // Add it back at the end (top) of the chain
	}


int CALLBACK PropertyProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	//HRESULT hr;

	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);

			EnumChildWindows(hwndDlg, EnumChildInitList, lParam);

			EnumChildWindows(hwndDlg, EnumChildProc, lParam);

			HWND hwndExpando = GetParent(hwndDlg);
			const int textlength = GetWindowTextLength(hwndExpando);
			const int titleheight = (textlength > 0) ? EXPANDOHEIGHT : 0;

			SetWindowPos(hwndDlg, NULL,
					0, titleheight, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);

			return FALSE;//TRUE;
			}
			break;

		case WM_DESTROY:
			return FALSE;
			break;

		case GET_COLOR_TABLE:
			{
			SmartBrowser * const psb = (SmartBrowser *)GetWindowLong(hwndDlg, GWL_USERDATA);
			*((unsigned long **)lParam) = psb->GetBaseISel()->GetPTable()->rgcolorcustom;
			return TRUE;
			}
			break;

		case WM_COMMAND:
			{
			const int code = HIWORD(wParam);
			int dispid = LOWORD(wParam);

			if (dispid == DISPID_FAKE_NAME)
				{
				dispid = 0x80010000;
				}

			SmartBrowser * const psb = (SmartBrowser *)GetWindowLong(hwndDlg, GWL_USERDATA);
			//IDispatch *pdisp = psb->m_pdisp;
			switch (code)
				{
				case EN_KILLFOCUS:
				case CBN_KILLFOCUS:
					{
					HWND hwndEdit;
					BOOL fChanged;

					if (code == EN_KILLFOCUS)
						{
						hwndEdit = (HWND)lParam;
						fChanged = SendMessage(hwndEdit, EM_GETMODIFY, 0, 0);
						}
					else
						{
						POINT pt;
						pt.x = 1;
						pt.y = 1;
						hwndEdit = ChildWindowFromPoint((HWND)lParam, pt);
						fChanged = GetWindowLong(hwndEdit, GWL_USERDATA);
						if (fChanged)
							{
							SetWindowLong(hwndEdit, GWL_USERDATA, 0);
							}
						}

					if (fChanged)//SendMessage(hwndEdit, EM_GETMODIFY, 0, 0))
						{
						char szText[1024];

						GetWindowText(hwndEdit, szText, 1024);

						CComVariant var(szText);

						psb->SetProperty(dispid, &var, fFalse);

						psb->GetControlValue((HWND)lParam); // If the new value was not valid, re-fill the control with the real value
						}
					}
					break;

				case CBN_EDITCHANGE:
					{
						POINT pt;
						pt.x = 1;
						pt.y = 1;
						HWND hwndEdit = ChildWindowFromPoint((HWND)lParam, pt);
						SetWindowLong(hwndEdit, GWL_USERDATA, 1);
					}
					break;

				case BN_CLICKED:
					{
					if (dispid == IDOK)
						{
						//(The user pressed enter)
						// BUG!!!!!!
						// If the object has a boolean at dispid 1 (IDOK),
						// This will override the correct behavior
						PostMessage(hwndDlg, WM_NEXTDLGCTL, 0, 0L) ;
						return TRUE;
						}

					const int state = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);

					const BOOL fChecked = (state & BST_CHECKED) != 0;

					CComVariant var(fChecked);

					psb->SetProperty(dispid, &var, fFalse);

					psb->GetControlValue((HWND)lParam);
					}
					break;

				case CBN_SELENDOK://CBN_SELCHANGE:
					{
					const int sel = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
					const DWORD cookie = SendMessage((HWND)lParam, CB_GETITEMDATA, sel, 0);

					const int proptype = GetWindowLong((HWND)lParam, GWL_USERDATA);

					if (proptype == 0)
						{
						IPerPropertyBrowsing *pippb;
						psb->GetBaseIDisp()->QueryInterface(IID_IPerPropertyBrowsing, (void **)&pippb);

						CComVariant var;
						pippb->GetPredefinedValue(dispid, cookie, &var);

						pippb->Release();

						psb->SetProperty(dispid, &var, fFalse);
						}
					else
						{
						// enum value
						CComVariant var((int)cookie);
						psb->SetProperty(dispid, &var, fFalse);
						}

					//SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)"Foo"/*szT*/);
					psb->GetControlValue((HWND)lParam);
					}
					break;

				case COLOR_CHANGED:
					{
					const int color = GetWindowLong((HWND)lParam, GWL_USERDATA);/*SendMessage((HWND)lParam, WM_GETTEXT, 0, 0);*/

					CComVariant var(color);

					psb->SetProperty(dispid, &var, fFalse);

					psb->GetControlValue((HWND)lParam);
					}
					break;

				case FONT_CHANGED:
					{
					IFontDisp * const pifd = (IFontDisp*)GetWindowLong((HWND)lParam, GWL_USERDATA);
					// Addred because the object will release the old one (really this one), before addreffing it again
					pifd->AddRef();
					CComVariant var(pifd);
					psb->SetProperty(dispid, &var, fTrue);
					pifd->Release();
					}
				}
			}
			break;
		}

	return FALSE;
	}

LRESULT CALLBACK SBFrameProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_CREATE:
			{
			CREATESTRUCT * const pcs = (CREATESTRUCT *)lParam;
			SetWindowLong(hwnd, GWL_USERDATA, (long)pcs->lpCreateParams);
			}
			break;

		case WM_PAINT:
			{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd,&ps);

			SmartBrowser * const psb = (SmartBrowser *)GetWindowLong(hwnd, GWL_USERDATA);
			psb->DrawHeader(hdc);

			EndPaint(hwnd,&ps);
			}
			return 0;
			break;
		}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

LRESULT CALLBACK ColorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_CREATE:
			{
			RECT rc;
			GetClientRect(hwnd, &rc);

			int colorkey;
			const HRESULT hr = GetRegInt("Editor", "TransparentColorKey", &colorkey);
			if (hr != S_OK) colorkey = (int)NOTRANSCOLOR; //not set assign no transparent color 	

			/*const HWND hwndButton =*/ CreateWindow("BUTTON","Color",WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hwnd, NULL, g_hinst, 0);
			SetWindowLong(hwnd, GWL_USERDATA, colorkey); //0);//rlc  get cached colorkey
			}
			break;

		case CHANGE_COLOR:
			{
			int color = lParam;
			if (wParam == 1)
				{
				color |= 0x80000000;
				}
			SetWindowLong(hwnd, GWL_USERDATA, color);
			InvalidateRect(hwnd, NULL, fFalse);
			}
			break;

		case WM_GETTEXT:
			{
			return GetWindowLong(hwnd, GWL_USERDATA);
			}
			break;

		/*case WM_PAINT:
			{
			HDC hdc;
			PAINTSTRUCT ps;
			hdc = BeginPaint(hwnd,&ps);

			//SmartBrowser *psb = (SmartBrowser *)GetWindowLong(hwnd, GWL_USERDATA);
			//psb->DrawHeader(hdc);

			HBRUSH hbrush = CreateSolidBrush(RGB(255,0,0));

			HBRUSH hbrushOld = (HBRUSH)SelectObject(hdc, hbrush);

			PatBlt(hdc, 0, 0, 60, 60, PATCOPY);

			SelectObject(hdc, hbrushOld);

			EndPaint(hwnd,&ps);
			}
			return 0;
			break;*/

		case WM_DRAWITEM:
			{
			DRAWITEMSTRUCT * const pdis = (DRAWITEMSTRUCT *)lParam;
			HDC hdc = pdis->hDC;
			int offset = 0;

			DWORD state = DFCS_BUTTONPUSH;
			if (pdis->itemState & ODS_SELECTED)
				{
				state |= DFCS_PUSHED;
				offset = 1;
				}

			DrawFrameControl(hdc, &pdis->rcItem, DFC_BUTTON, state);

			const int color = GetWindowLong(hwnd, GWL_USERDATA);

			if (!(color & 0x80000000)) // normal color, not ninched
				{
				const int oldcolor = GetWindowLong(hwnd, GWL_USERDATA) & 0xffffff; // have to AND it to get rid of ninch bit
				HBRUSH hbrush = CreateSolidBrush(oldcolor);

				HBRUSH hbrushOld = (HBRUSH)SelectObject(hdc, hbrush);

				PatBlt(hdc, 6 + offset, 6 + offset, pdis->rcItem.right - pdis->rcItem.left - 12, pdis->rcItem.bottom - pdis->rcItem.top - 12, PATCOPY);

				SelectObject(hdc, hbrushOld);
				}
			}
			break;

		case WM_COMMAND:
			{
			const int code = HIWORD(wParam);
			switch (code)
				{
				case BN_CLICKED:
					{
					HWND hwndDlg = GetParent(hwnd);
					/*SmartBrowser * const psb =*/ (SmartBrowser *)GetWindowLong(hwndDlg, GWL_USERDATA);
					CHOOSECOLOR cc;
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwnd;
					cc.hInstance = NULL;
					cc.rgbResult = GetWindowLong(hwnd, GWL_USERDATA);
					SendMessage(hwndDlg, GET_COLOR_TABLE, 0, (long)&cc.lpCustColors);
					//cc.lpCustColors = (unsigned long *)SendMessage(hwndDlg, GET_COLOR_TABLE, 0, 0);//psb->m_pisel->GetPTable()->rgcolorcustom;//cr;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
					cc.lCustData = NULL;
					cc.lpfnHook  = NULL;
					cc.lpTemplateName = NULL;
					if (ChooseColor(&cc))
						{
						const int id = GetDlgCtrlID(hwnd);
						SetWindowLong(hwnd, GWL_USERDATA, cc.rgbResult);
						SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(id, COLOR_CHANGED), (LPARAM)hwnd);
						InvalidateRect(hwnd, NULL, fFalse);
						}
					break;
					}
				}
			}
		}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

LRESULT CALLBACK FontProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_CREATE:
			{
			RECT rc;
			GetClientRect(hwnd, &rc);
			HWND hwndButton = CreateWindow("BUTTON","",BS_LEFT | WS_VISIBLE | WS_CHILD, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hwnd, NULL, g_hinst, 0);
			SetWindowLong(hwnd, GWL_USERDATA, 0);

			HFONT hfontButton = CreateFont(-10, 0, 0, 0, /*FW_NORMAL*/ FW_MEDIUM, FALSE, FALSE, FALSE,
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");

			SendMessage(hwndButton, WM_SETFONT, (DWORD)hfontButton, 0);
			}
			break;

		case WM_DESTROY:
			{
			POINT pt;
			pt.x = 1;
			pt.y = 1;
			HWND hwndButton = ChildWindowFromPoint(hwnd, pt);

			HFONT hfontButton = (HFONT)SendMessage(hwndButton, WM_GETFONT, 0, 0);

			SendMessage(hwndButton, WM_SETFONT, NULL, 0); // Just in case the button needs to use a font before it is destroyed

			if (hfontButton)
				{
				DeleteObject(hfontButton);
				}
			}
			break;

		case CHANGE_FONT:
			{
			SetWindowLong(hwnd, GWL_USERDATA, lParam);

			POINT pt;
			pt.x = 1;
			pt.y = 1;
			HWND hwndButton = ChildWindowFromPoint(hwnd, pt);

			IFontDisp * const pifd = (IFontDisp *)lParam;
			IFont *pif;
			pifd->QueryInterface(IID_IFont, (void **)&pif);
			CComBSTR bstrName;
			/*const HRESULT hr =*/ pif->get_Name(&bstrName);
			pif->Release();

			if (wParam != 1) // non-niched value
				{
				char szT[64];
				WideCharToMultiByte(CP_ACP, 0, bstrName, -1, szT, 64, NULL, NULL);
				SetWindowText(hwndButton, szT);
				}
			else
				{
				SetWindowText(hwndButton, "");
				}
			}
			break;

		case WM_GETTEXT:
			{
			return GetWindowLong(hwnd, GWL_USERDATA);
			}
			break;

		case WM_COMMAND:
			{
			const int code = HIWORD(wParam);
			switch (code)
				{
				case BN_CLICKED:
					{
					CHOOSEFONT cf;
					LOGFONT lf;
					char szstyle[256];

					// Set up logfont to be like our current font
					IFontDisp *pifd = (IFontDisp *)GetWindowLong(hwnd, GWL_USERDATA);
					IFont *pif;
					pifd->QueryInterface(IID_IFont, (void **)&pif);

					HFONT hfont;
					pif->get_hFont(&hfont);
					GetObject(hfont, sizeof(LOGFONT), &lf);

					pif->Release();

					memset(&cf, 0, sizeof(CHOOSEFONT));

					cf.lStructSize = sizeof(CHOOSEFONT);
					cf.hwndOwner = hwnd;
					cf.lpLogFont = &lf;
					cf.lpszStyle = (char *)&szstyle;
					//cf.iPointSize = 120;
					cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
					if (ChooseFont(&cf))
						{
						IFontDisp * const pifd = (IFontDisp *)GetWindowLong(hwnd, GWL_USERDATA);
						IFont *pif;
						pifd->QueryInterface(IID_IFont, (void **)&pif);

						CY cy;
						cy.int64 = (cf.iPointSize * 10000 / 10);
						HRESULT hr = pif->put_Size(cy);

						WCHAR wzT[64];
						MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, -1, wzT, 64);
						CComBSTR bstr(wzT);
						hr = pif->put_Name(bstr);

						pif->put_Weight((short)lf.lfWeight);

						pif->put_Italic(lf.lfItalic);

						SendMessage(hwnd, CHANGE_FONT, 0, (long)pifd);

						HWND hwndDlg = GetParent(hwnd);
						const int id = GetDlgCtrlID(hwnd);
						SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(id, FONT_CHANGED), (LPARAM)hwnd);

						pif->Release();
						}
					break;
					}
				}
			}
		}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	
	LRESULT CALLBACK ExpandoProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	ExpandoInfo *pexinfo;

	switch (uMsg)
		{
		case WM_CREATE:
			{
			CREATESTRUCT * const pcs = (CREATESTRUCT *)lParam;
			SetWindowLong(hwnd, GWL_USERDATA, (long)pcs->lpCreateParams);
			break;
			}

		case WM_DESTROY:
			{
			pexinfo = (ExpandoInfo *)GetWindowLong(hwnd, GWL_USERDATA);
			delete pexinfo;
			break;
			}

		case WM_LBUTTONUP:
			{
			pexinfo = (ExpandoInfo *)GetWindowLong(hwnd, GWL_USERDATA);

			if (pexinfo->m_fHasCaption) // Null title means not an expando
				{
				//const int xPos = LOWORD(lParam); 
				const int yPos = HIWORD(lParam);
				if (yPos < 16)//wParam == HTCAPTION)
					{
					if (pexinfo->m_fExpanded)
						{
						SendMessage(hwnd, EXPANDO_COLLAPSE, 1, 0);
						}
					else
						{
						SendMessage(hwnd, EXPANDO_EXPAND, 1, 0);
						}
					}
				}
			}
			break;

		case EXPANDO_EXPAND:
			{
			pexinfo = (ExpandoInfo *)GetWindowLong(hwnd, GWL_USERDATA);
			pexinfo->m_fExpanded = fTrue;			

			int titleheight;

			if (pexinfo->m_fHasCaption) // Null title means not an expando
				{
				titleheight = EXPANDOHEIGHT;
				pexinfo->m_psb->ResetPriority(pexinfo->m_id);
				}
			else
				{
				titleheight = 0;
				}

			SetWindowPos(hwnd, NULL, 0, 0, pexinfo->m_psb->m_maxdialogwidth, titleheight + pexinfo->m_dialogheight, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);
			
			InvalidateRect(hwnd, NULL, fFalse);

			if (wParam == 1)
				{
				pexinfo->m_psb->RelayoutExpandos();
				}
			}
			break;

		case EXPANDO_COLLAPSE:
			{
			pexinfo = (ExpandoInfo *)GetWindowLong(hwnd, GWL_USERDATA);
			pexinfo->m_fExpanded = fFalse;
		
			SetWindowPos(hwnd, NULL, 0, 0, pexinfo->m_psb->m_maxdialogwidth, EXPANDOHEIGHT, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);
			
			InvalidateRect(hwnd, NULL, fFalse);

			if (wParam == 1)
				{
				pexinfo->m_psb->RelayoutExpandos();
				}
			}
			break;

		case WM_SETCURSOR:
			{
			POINT ptCursor;
			GetCursorPos(&ptCursor);
			ScreenToClient(hwnd, &ptCursor);

			if (ptCursor.y < EXPANDOHEIGHT)
				{
				HCURSOR hcursor = LoadCursor(NULL, MAKEINTRESOURCE(32649));

				SetCursor(hcursor);
				return TRUE;
				}
			}
			break;

		case WM_PAINT:
			{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd,&ps);

			pexinfo = (ExpandoInfo *)GetWindowLong(hwnd, GWL_USERDATA);

			//int textlength = GetWindowTextLength(hwnd);

			RECT rc;
			if (pexinfo->m_fHasCaption) // Null title means not an expando
				{
				RECT rcCur;
				GetWindowRect(hwnd, &rcCur);

				rc.left = 0;
				rc.top = 0;
				rc.right = rcCur.right - rcCur.left;
				rc.bottom = EXPANDOHEIGHT;

				DrawCaption(hwnd, hdc, &rc, DC_SMALLCAP | DC_TEXT | 0x0020 | DC_ACTIVE);

				HBITMAP hbmchevron = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(IDB_CHEVRON), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
				HDC hdcbmp = CreateCompatibleDC(hdc);
				HBITMAP hbmOld = (HBITMAP)SelectObject(hdcbmp, hbmchevron);
				const int offsetx = (rcCur.bottom - rcCur.top > EXPANDOHEIGHT) ? 18 : 0;
				BitBlt(hdc, rc.right - 20, 0, 18, 18, hdcbmp, offsetx, 0, SRCPAINT);
				SelectObject(hdcbmp, hbmOld);
				DeleteDC(hdcbmp);
				DeleteObject(hbmchevron);
				}

			EndPaint(hwnd,&ps);
			}
			break;
		}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
