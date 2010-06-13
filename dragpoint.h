// DragPoint.h: Definition of the DragPoint class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_)
#define AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols

//class Surface;

class IHaveDragPoints
	{
public:
	virtual ~IHaveDragPoints();

	virtual IEditable *GetIEditable()=0;
	virtual PinTable *GetPTable()=0;

	virtual int GetMinimumPoints() {return 3;}

	virtual HRESULT SavePointData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
	//virtual HRESULT InitPointLoad(IStream *pstm, HCRYPTHASH hcrypthash);
	virtual void LoadPointToken(int id, BiffReader *pbr, int version);

	virtual void ClearPointsForOverwrite();

	virtual void GetPointCenter(Vertex2D *pv);
	virtual void PutPointCenter(Vertex2D *pv);

	void FlipPointY(Vertex2D *pvCenter);
	void FlipPointX(Vertex2D *pvCenter);
	void RotateDialog();
	void RotatePoints(float ang, Vertex2D *pvCenter);
	void ScaleDialog();
	void ScalePoints(float scalex, float scaley, Vertex2D *pvCenter);
	void TranslateDialog();
	void TranslatePoints(Vertex2D *pvOffset);
	void ReverseOrder();
	virtual void GetPointDialogPanes(Vector<PropertyPane> *pvproppane);

	void GetRgVertex(Vector<RenderVertex> *pvv);
	void GetTextureCoords(Vector<RenderVertex> *pvv, float **ppcoords);

	Vector< CComObject<DragPoint> > m_vdpoint;
	};

/////////////////////////////////////////////////////////////////////////////
// DragPoint

class DragPoint :
	public IDispatchImpl<IControlPoint, &IID_IControlPoint, &LIBID_VBATESTLib>,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<DragPoint,&CLSID_DragPoint>,
	public ISelect
{
public:
	DragPoint() {}

	void Init(IHaveDragPoints *pihdp, float x, float y);

	// From ISelect
	virtual void OnLButtonDown(int x, int y);
	virtual void OnLButtonUp(int x, int y);
	virtual void MoveOffset(float dx, float dy);
	virtual void SetObjectPos();
	virtual ItemTypeEnum GetItemType() {return eItemDragPoint;}

	// Multi-object manipulation
	virtual void GetCenter(Vertex2D *pv);
	virtual void PutCenter(Vertex2D *pv);

	virtual void EditMenu(HMENU hmenu);
	virtual void DoCommand(int icmd, int x, int y);
	virtual void SetSelectFormat(Sur *psur);
	virtual void SetMultiSelectFormat(Sur *psur);
	virtual PinTable *GetPTable() {return m_pihdp->GetIEditable()->GetPTable();}
#ifdef VBA
	virtual IApcProjectItem *GetIApcProjectItem() {return m_pihdp->GetIEditable()->GetIApcProjectItem();}
#endif
	virtual IEditable *GetIEditable() {return m_pihdp->GetIEditable();}
	//virtual HRESULT GetTypeName(BSTR *pVal);
	virtual IDispatch *GetDispatch();
	//virtual int GetDialogID();
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	virtual int GetSelectLevel() {return 2;} // So dragpoints won't be band-selected with the main objects

BEGIN_COM_MAP(DragPoint)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IControlPoint)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
//DECLARE_NOT_AGGREGATABLE(DragPoint)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

DECLARE_REGISTRY_RESOURCEID(IDR_DragPoint)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

#ifdef VBA
	virtual IApcControl *GetIApcControl() {return NULL;}
#endif

	virtual void Delete();
	virtual void Uncreate();

	virtual BOOL LoadToken(int id, BiffReader *pbr);

// IControlPoint
public:
	STDMETHOD(get_TextureCoordinateU)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_TextureCoordinateU)(/*[in]*/ float newVal);
	STDMETHOD(get_IsAutoTextureCoordinate)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_IsAutoTextureCoordinate)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Smooth)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Smooth)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_X)(/*[in]*/ float newVal);

	Vertex2D m_v;
	BOOL m_fSmooth;
	BOOL m_fSlingshot;
	BOOL m_fAutoTexture;
	float m_texturecoord;
	IHaveDragPoints *m_pihdp;
};

#endif // !defined(AFX_DRAGPOINT_H__E0C074C9_5BF2_4F8C_8012_76082BAC2203__INCLUDED_)
