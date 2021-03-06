#pragma once
class PinTable;

class ScriptGlobalTable:
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<ITableGlobal, &IID_ITableGlobal, &LIBID_VPinballLib>,
	public IScriptable
	{
public:
		// Headers to support communication between the game and the script.
		STDMETHOD(EndModal)();
		STDMETHOD(BeginModal)();
		STDMETHOD(GetTextFile)(BSTR FileName, /*[out, retval]*/ BSTR *pContents);
		STDMETHOD(get_GameTime)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_AddCreditKey)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_AddCreditKey2)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_ActiveBall)(/*[out, retval]*/ IBall **pVal);
		STDMETHOD(LoadValue)(BSTR TableName, BSTR ValueName, /*[out, retval]*/ VARIANT *Value);
		STDMETHOD(SaveValue)(BSTR TableName, BSTR ValueName, VARIANT Value);
		STDMETHOD(StopSound)(BSTR Sound);
		STDMETHOD(AddObject)(BSTR Name, IDispatch *pdisp);
		STDMETHOD(get_GetPlayerHWnd)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_UserDirectory)(/*[out, retval]*/ BSTR *pVal);
		STDMETHOD(get_StartGameKey)(/*[out, retval]*/ long *pVal);
		STDMETHOD(EndMusic)();
		STDMETHOD(PlayMusic)(BSTR str);
		STDMETHOD(get_PlungerKey)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_CenterTiltKey)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_RightTiltKey)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_LeftTiltKey)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_RightFlipperKey)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_LeftFlipperKey)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_VPBuildVersion)(/*[out, retval]*/ long *pVal);

		STDMETHOD(PlaySound)(BSTR bstr, long LoopCount, float volume, float pan, float randompitch, long pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart);
		STDMETHOD(FireKnocker)(/*[in]*/ int Count);
		STDMETHOD(QuitPlayer)(/*[in]*/ int CloseType);
		STDMETHOD(StartShake)(/*[in]*/ void);
		STDMETHOD(StopShake)(/*[in]*/ void);
		

   		STDMETHOD(Nudge)(float Angle, float Force);
		STDMETHOD(get_Name)(BSTR *pVal);
		STDMETHOD(get_MechanicalTilt)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_LeftMagnaSave)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_RightMagnaSave)(/*[out, retval]*/ long *pVal);
		STDMETHOD(get_ExitGame)(/*[out, retval]*/ long *pVal);
		
        STDMETHOD(VersionMajor)(/*[out, retval]*/ int *pVal);
        STDMETHOD(VersionMinor)(/*[out, retval]*/ int *pVal);
        STDMETHOD(VersionRevision)(/*[out, retval]*/ int *pVal);

	void Init(PinTable *pt);

	virtual IDispatch *GetDispatch();

	virtual ISelect *GetISelect() {return NULL;}

	BOOL GetTextFileFromDirectory(char *szfilename, char *dirname, BSTR *pContents);

BEGIN_COM_MAP(ScriptGlobalTable)
	COM_INTERFACE_ENTRY(ITableGlobal)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	PinTable *m_pt;
	};
