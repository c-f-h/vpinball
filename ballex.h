// Ball.h : Declaration of the CBall
#pragma once
#ifndef __BALL_H_
#define __BALL_H_

#include "resource.h"       // main symbols

#define CHECKSTALEBALL if (!m_pball) {return E_POINTER;}

class Ball;

/////////////////////////////////////////////////////////////////////////////
// CBall
class ATL_NO_VTABLE BallEx : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<BallEx, &CLSID_Ball>,
	public IDispatchImpl<IBall, &IID_IBall, &LIBID_VBATESTLib>,
	public IFireEvents,
	public IDebugCommands
{
public:
	BallEx();
	~BallEx();

DECLARE_REGISTRY_RESOURCEID(IDR_BALL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(BallEx)
	COM_INTERFACE_ENTRY(IBall)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IBall
public:
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Name)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_BackDecal)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_BackDecal)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_FrontDecal)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_FrontDecal)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Color)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Color)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_VelZ)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_VelZ)(/*[in]*/ float newVal);
	STDMETHOD(get_Z)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Z)(/*[in]*/ float newVal);
	STDMETHOD(get_VelY)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_VelY)(/*[in]*/ float newVal);
	STDMETHOD(get_VelX)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_VelX)(/*[in]*/ float newVal);
	STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_X)(/*[in]*/ float newVal);
	STDMETHOD(get_UserValue)(VARIANT *pVal);
	STDMETHOD(put_UserValue)(VARIANT *newVal);
	STDMETHOD(DestroyBall)(/*[out, retval]*/ int *pVal);

	virtual void FireGroupEvent(int dispid) {}
	virtual IDispatch *GetDispatch() {return ((IDispatch *) this);}
	virtual IDebugCommands *GetDebugCommands() {return (IDebugCommands *) this;}

	// IDebugCommands
	virtual void GetDebugCommands(VectorInt<int> *pvids, VectorInt<int> *pvcommandid);
	virtual void RunDebugCommand(int id);

	Ball *m_pball;
	VARIANT m_uservalue;
};

#endif //__BALL_H_
