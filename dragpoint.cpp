#include "stdafx.h"

IHaveDragPoints::~IHaveDragPoints()
{
   for (int i=0;i<m_vdpoint.Size();i++)
   {
      m_vdpoint.ElementAt(i)->Release();
   }
}

void IHaveDragPoints::GetPointCenter(Vertex2D * const pv) const
{
   float minx = FLT_MAX;
   float maxx = -FLT_MAX;
   float miny = FLT_MAX;
   float maxy = -FLT_MAX;

   for (int i=0;i<m_vdpoint.Size();i++)
   {
      const Vertex2D v = m_vdpoint.ElementAt(i)->m_v;
      minx = min(minx, v.x);
      maxx = max(maxx, v.x);
      miny = min(miny, v.y);
      maxy = max(maxy, v.y);

   }

   pv->x = (maxx+minx)*0.5f;
   pv->y = (maxy+miny)*0.5f;
}

void IHaveDragPoints::PutPointCenter(const Vertex2D * const pv)
{
}

void IHaveDragPoints::FlipPointY(Vertex2D *pvCenter)
{
   GetIEditable()->BeginUndo();
   GetIEditable()->MarkForUndo();

   Vertex2D newcenter;
   GetPointCenter(&newcenter);

   const float ycenter = pvCenter->y;

   for (int i=0;i<m_vdpoint.Size();i++)
   {
      const float deltay = m_vdpoint.ElementAt(i)->m_v.y - ycenter;

      m_vdpoint.ElementAt(i)->m_v.y -= deltay*2.0f;
   }

   const float deltay = newcenter.y - ycenter;
   newcenter.y -= deltay*2.0f;
   PutPointCenter(&newcenter);

   ReverseOrder();

   GetIEditable()->EndUndo();

   GetPTable()->SetDirtyDraw();
}

void IHaveDragPoints::FlipPointX(Vertex2D *pvCenter)
{
   GetIEditable()->BeginUndo();
   GetIEditable()->MarkForUndo();

   Vertex2D newcenter;
   GetPointCenter(&newcenter);

   const float xcenter = pvCenter->x;

   for (int i=0;i<m_vdpoint.Size();i++)
   {
      const float deltax = m_vdpoint.ElementAt(i)->m_v.x - xcenter;

      m_vdpoint.ElementAt(i)->m_v.x -= deltax*2.0f;
   }

   const float deltax = newcenter.x - xcenter;
   newcenter.x -= deltax*2.0f;
   PutPointCenter(&newcenter);

   ReverseOrder();

   GetIEditable()->EndUndo();

   GetPTable()->SetDirtyDraw();
}

void IHaveDragPoints::RotateDialog()
{
#ifdef VBA
   g_pvp->ApcHost->BeginModalDialog();
#endif
   DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_ROTATE),
      g_pvp->m_hwnd, RotateProc, (long)this->GetIEditable()->GetISelect());//(long)this);
#ifdef VBA
   g_pvp->ApcHost->EndModalDialog();
#endif
}

void IHaveDragPoints::ScaleDialog()
{
#ifdef VBA
   g_pvp->ApcHost->BeginModalDialog();
#endif
   DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_SCALE),
      g_pvp->m_hwnd, ScaleProc, (long)this->GetIEditable()->GetISelect());
#ifdef VBA
   g_pvp->ApcHost->EndModalDialog();
#endif
}

void IHaveDragPoints::TranslateDialog()
{
#ifdef VBA
   g_pvp->ApcHost->BeginModalDialog();
#endif
   DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_TRANSLATE),
      g_pvp->m_hwnd, TranslateProc, (long)this->GetIEditable()->GetISelect());
#ifdef VBA
   g_pvp->ApcHost->EndModalDialog();
#endif
}

void IHaveDragPoints::RotatePoints(float ang, Vertex2D *pvCenter)
{
   Vertex2D newcenter;
   GetPointCenter(&newcenter);

   GetIEditable()->BeginUndo();
   GetIEditable()->MarkForUndo();

   const float centerx = pvCenter->x;
   const float centery = pvCenter->y;

   const float sn = sinf(ANGTORAD(ang));
   const float cs = cosf(ANGTORAD(ang));

   for (int i=0;i<m_vdpoint.Size();i++)
   {
      DragPoint * const pdp1 = m_vdpoint.ElementAt(i);
      const float dx = pdp1->m_v.x - centerx;
      const float dy = pdp1->m_v.y - centery;
      const float dx2 = cs*dx - sn*dy;
      const float dy2 = cs*dy + sn*dx;
      pdp1->m_v.x = centerx + dx2;
      pdp1->m_v.y = centery + dy2;
   }

   // Move object center as well (if rotating around object center,
   // this will have no effect)
   {
      const float dx = newcenter.x - centerx;
      const float dy = newcenter.y - centery;
      const float dx2 = cs*dx - sn*dy;
      const float dy2 = cs*dy + sn*dx;
      newcenter.x = centerx + dx2;
      newcenter.y = centery + dy2;
      PutPointCenter(&newcenter);
   }

   GetIEditable()->EndUndo();

   GetPTable()->SetDirtyDraw();
}

void IHaveDragPoints::ScalePoints(float scalex, float scaley, Vertex2D *pvCenter)
{
   Vertex2D newcenter;
   GetPointCenter(&newcenter);

   GetIEditable()->BeginUndo();
   GetIEditable()->MarkForUndo();

   const float centerx = pvCenter->x;
   const float centery = pvCenter->y;

   for (int i=0;i<m_vdpoint.Size();i++)
   {
      DragPoint *pdp1 = m_vdpoint.ElementAt(i);
      const float dx = (pdp1->m_v.x - centerx) * scalex;
      const float dy = (pdp1->m_v.y - centery) * scaley;
      pdp1->m_v.x = centerx + dx;
      pdp1->m_v.y = centery + dy;
   }

   // Move object center as well (if scaling from object center,
   // this will have no effect)
   {
      const float dx = (newcenter.x - centerx) * scalex;
      const float dy = (newcenter.y - centery) * scaley;
      newcenter.x = centerx + dx;
      newcenter.y = centery + dy;
      PutPointCenter(&newcenter);
   }

   GetIEditable()->EndUndo();

   GetPTable()->SetDirtyDraw();
}

void IHaveDragPoints::TranslatePoints(Vertex2D *pvOffset)
{
   GetIEditable()->BeginUndo();
   GetIEditable()->MarkForUndo();

   for (int i=0;i<m_vdpoint.Size();i++)
   {
      DragPoint * const pdp1 = m_vdpoint.ElementAt(i);
      pdp1->m_v.x += pvOffset->x;
      pdp1->m_v.y += pvOffset->y;
   }

   Vertex2D newcenter;
   GetPointCenter(&newcenter);

   newcenter.x += pvOffset->x;
   newcenter.y += pvOffset->y;

   PutPointCenter(&newcenter);

   GetIEditable()->EndUndo();

   GetPTable()->SetDirtyDraw();
}

void IHaveDragPoints::ReverseOrder()
{
   if (m_vdpoint.Size() == 0)
   {
      return;
   }

   // Reverse order of points (switches winding, reverses inside/outside)
   for (int i=0;i<m_vdpoint.Size()/2;i++)
   {
      DragPoint * const pdp1 = m_vdpoint.ElementAt(i);
      DragPoint * const pdp2 = m_vdpoint.ElementAt(m_vdpoint.Size() - 1 - i);
      m_vdpoint.ReplaceElementAt(pdp2,i);
      m_vdpoint.ReplaceElementAt(pdp1, m_vdpoint.Size() - 1 - i);
   }

   const BOOL fSlingshotTemp = m_vdpoint.ElementAt(0)->m_fSlingshot;

   for (int i=0;i<m_vdpoint.Size()-1;i++)
   {
      DragPoint * const pdp1 = m_vdpoint.ElementAt(i);
      const DragPoint * const pdp2 = m_vdpoint.ElementAt(i+1);

      pdp1->m_fSlingshot = pdp2->m_fSlingshot;
   }

   m_vdpoint.ElementAt(m_vdpoint.Size()-1)->m_fSlingshot = fSlingshotTemp;
}

void IHaveDragPoints::GetRgVertex(Vector<RenderVertex> * const pvv)
{
   const int cpoint = m_vdpoint.Size();

   //cpointCur = 0;

   for (int i=0;i<cpoint;i++)
   {
      const CComObject<DragPoint> * const pdp1 = m_vdpoint.ElementAt(i);
      const CComObject<DragPoint> * const pdp2 = m_vdpoint.ElementAt((i < cpoint-1) ? (i+1) : 0);

      if ((pdp1->m_v.x == pdp2->m_v.x) && (pdp1->m_v.y == pdp2->m_v.y))
      {
         // Special case - two points coincide
         continue;
      }

      const CComObject<DragPoint> * const pdp0 = pdp1->m_fSmooth ? m_vdpoint.ElementAt((i == 0) ? (cpoint-1) : (i-1)) : pdp1;
      const CComObject<DragPoint> * const pdp3 = pdp2->m_fSmooth ? m_vdpoint.ElementAt((i < cpoint-2) ? (i+2) : ((i+2)-cpoint)) : pdp2;

      CatmullCurve cc;
      cc.SetCurve(&pdp0->m_v, &pdp1->m_v, &pdp2->m_v, &pdp3->m_v);

      RenderVertex rendv1, rendv2; // Create these to add the special properties

      rendv1.x = pdp1->m_v.x;
      rendv1.y = pdp1->m_v.y;
      rendv1.fSmooth = (pdp1->m_fSmooth != 0);
      rendv1.fSlingshot = (pdp1->m_fSlingshot != 0);
      rendv1.fControlPoint = true;

      // Properties of last point don't matter, because it won't be added to the list on this pass (it'll get added as the first point of the next curve)
      rendv2.x = pdp2->m_v.x;
      rendv2.y = pdp2->m_v.y;

      RecurseSmoothLine(&cc, 0, 1, &rendv1, &rendv2, pvv);
   }
}

void IHaveDragPoints::GetPointDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane = new PropertyPane(IDD_PROPPOINT_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPOINT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);
}

void IHaveDragPoints::GetTextureCoords(Vector<RenderVertex> *pvv, float **ppcoords)
{
   VectorInt<int> vitexpoints;
   VectorInt<int> virenderpoints;
   bool m_fNoCoords = false;

   const int cpoints = pvv->Size();
   int icontrolpoint = 0;

   *ppcoords = new float[cpoints];

   for (int i=0; i<cpoints; ++i)
   {
      RenderVertex * const prv = pvv->ElementAt(i);
      if (prv->fControlPoint)
      {
         if (!m_vdpoint.ElementAt(icontrolpoint)->m_fAutoTexture)
         {
            vitexpoints.AddElement(icontrolpoint);
            virenderpoints.AddElement(i);
         }
         ++icontrolpoint;
      }
   }

   if (vitexpoints.Size() == 0)
   {
      // Special case - no texture coordinates were specified
      // Make them up starting at point 0
      vitexpoints.AddElement(0);
      virenderpoints.AddElement(0);

      m_fNoCoords = true;
   }

   // Wrap the array around so we cover the last section
   vitexpoints.AddElement(vitexpoints.ElementAt(0) + m_vdpoint.Size());
   virenderpoints.AddElement(virenderpoints.ElementAt(0) + cpoints);

   for (int i=0; i<vitexpoints.Size()-1; ++i)
   {
      const int startrenderpoint = virenderpoints.ElementAt(i) % cpoints;
      int endrenderpoint = virenderpoints.ElementAt((i < cpoints-1) ? (i+1) : 0) % cpoints;

      float starttexcoord;
      float endtexcoord;
      if (m_fNoCoords)
      {
         starttexcoord = 0.0f;
         endtexcoord = 1.0f;
      }
      else
      {
         starttexcoord = m_vdpoint.ElementAt(vitexpoints.ElementAt(i) % m_vdpoint.Size())->m_texturecoord;
         endtexcoord = m_vdpoint.ElementAt(vitexpoints.ElementAt(i+1) % m_vdpoint.Size())->m_texturecoord;
      }

      const float deltacoord = endtexcoord - starttexcoord;

      if (endrenderpoint <= startrenderpoint)
      {
         endrenderpoint += cpoints;
      }

      float totallength = 0.0f;
      for (int l=startrenderpoint; l<endrenderpoint; ++l)
      {
         const Vertex2D * const pv1 = (Vertex2D *)pvv->ElementAt(l % cpoints);
         const Vertex2D * const pv2 = (Vertex2D *)pvv->ElementAt((l+1) % cpoints);

         const float dx = pv1->x - pv2->x;
         const float dy = pv1->y - pv2->y;
         const float length = sqrtf(dx*dx + dy*dy);

         totallength += length;
      }

      float partiallength = 0.0f;
      for (int l=startrenderpoint; l<endrenderpoint; ++l)
      {
         const Vertex2D * const pv1 = (Vertex2D *)pvv->ElementAt(l % cpoints);
         const Vertex2D * const pv2 = (Vertex2D *)pvv->ElementAt((l+1) % cpoints);

         const float dx = pv1->x - pv2->x;
         const float dy = pv1->y - pv2->y;
         const float length = sqrtf(dx*dx + dy*dy);

         const float texcoord = partiallength / totallength;

         (*ppcoords)[l % cpoints] = (texcoord * deltacoord) + starttexcoord;

         partiallength += length;
      }
   }
}

void IHaveDragPoints::ClearPointsForOverwrite()
{
   for (int i=0;i<m_vdpoint.Size();i++)
   {
      if (m_vdpoint.ElementAt(i)->m_selectstate != eNotSelected/*GetPTable()->m_pselcur == m_vdpoint.ElementAt(i)*/)
      {
         //GetPTable()->SetSel(GetPTable());
         GetPTable()->AddMultiSel(GetPTable(), fFalse, fTrue);
      }

      m_vdpoint.ElementAt(i)->Release();
   }

   m_vdpoint.RemoveAllElements();
}

HRESULT IHaveDragPoints::SavePointData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   for (int i=0;i<m_vdpoint.Size();i++)
   {
      bw.WriteTag(FID(DPNT));
      CComObject<DragPoint> *pdp = m_vdpoint.ElementAt(i);
      bw.WriteStruct(FID(VCEN), &(pdp->m_v), sizeof(Vertex2D));
      bw.WriteBool(FID(SMTH), pdp->m_fSmooth);
      bw.WriteBool(FID(SLNG), pdp->m_fSlingshot);
      bw.WriteBool(FID(ATEX), pdp->m_fAutoTexture);
      bw.WriteFloat(FID(TEXC), pdp->m_texturecoord);

      (ISelect *)pdp->SaveData(pstm, hcrypthash, hcryptkey);

      bw.WriteTag(FID(ENDB));
   }

   return S_OK;
}


void IHaveDragPoints::LoadPointToken(int id, BiffReader *pbr, int version)
{
   if (id == FID(DPNT))
   {
      CComObject<DragPoint> *pdp;
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, 0, 0);
         m_vdpoint.AddElement(pdp);
         BiffReader br(pbr->m_pistream, pdp, NULL, version, pbr->m_hcrypthash, pbr->m_hcryptkey);
         br.Load();
      }
   }
}

void DragPoint::Init(IHaveDragPoints *pihdp, float x, float y)
{
   m_pihdp = pihdp;
   m_fSmooth = fFalse;

   m_fSlingshot = fFalse;
   m_v.x = x;
   m_v.y = y;
   m_fAutoTexture = fTrue;
   m_texturecoord = 0.0;

   m_menuid = IDR_POINTMENU;
}

void DragPoint::OnLButtonDown(int x, int y)
{
   ISelect::OnLButtonDown(x,y);
   m_pihdp->GetIEditable()->SetDirtyDraw();
}

void DragPoint::OnLButtonUp(int x, int y)
{
   ISelect::OnLButtonUp(x,y);
   m_pihdp->GetIEditable()->SetDirtyDraw();
}

void DragPoint::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_v.x, m_v.y);
}

void DragPoint::MoveOffset(const float dx, const float dy)
{
   m_v.x += dx;
   m_v.y += dy;

   m_pihdp->GetIEditable()->SetDirtyDraw();
}

void DragPoint::GetCenter(Vertex2D * const pv) const
{
   *pv = m_v;
}

void DragPoint::PutCenter(const Vertex2D * const pv)
{
   m_v = *pv;

   m_pihdp->GetIEditable()->SetDirtyDraw();
}

void DragPoint::EditMenu(HMENU hmenu)
{
   CheckMenuItem(hmenu, ID_POINTMENU_SMOOTH, MF_BYCOMMAND | (m_fSmooth ? MF_CHECKED : MF_UNCHECKED));
   //EnableMenuItem(hmenu, ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | (m_fSmooth ? MF_GRAYED : MF_ENABLED));
   CheckMenuItem(hmenu, ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | ((m_fSlingshot && !m_fSmooth) ? MF_CHECKED : MF_UNCHECKED));
}

void DragPoint::Delete()
{
   if (m_pihdp->m_vdpoint.Size() > m_pihdp->GetMinimumPoints()) // Can't allow less points than the user can recover from
   {
      m_pihdp->GetIEditable()->BeginUndo();
      m_pihdp->GetIEditable()->MarkForUndo();
      m_pihdp->m_vdpoint.RemoveElement(this);
      m_pihdp->GetIEditable()->EndUndo();
      m_pihdp->GetIEditable()->SetDirtyDraw();
      Release();
   }
}

void DragPoint::Uncreate()
{
   m_pihdp->m_vdpoint.RemoveElement(this);
   Release();
}

void DragPoint::DoCommand(int icmd, int x, int y)
{
   ISelect::DoCommand(icmd, x, y);
   switch (icmd)
   {
   case ID_POINTMENU_SMOOTH:
      int index2;

      m_pihdp->GetIEditable()->BeginUndo();
      m_pihdp->GetIEditable()->MarkForUndo();

      m_fSmooth = !m_fSmooth;
      index2 = (m_pihdp->m_vdpoint.IndexOf(this) - 1 + m_pihdp->m_vdpoint.Size()) % m_pihdp->m_vdpoint.Size();
      if (m_fSmooth && m_fSlingshot)
      {
         m_fSlingshot = fFalse;
      }
      if (m_fSmooth && m_pihdp->m_vdpoint.ElementAt(index2)->m_fSlingshot)
      {
         m_pihdp->m_vdpoint.ElementAt(index2)->m_fSlingshot = fFalse;
      }

      m_pihdp->GetIEditable()->EndUndo();
      m_pihdp->GetIEditable()->SetDirtyDraw();
      break;

   case ID_POINTMENU_SLINGSHOT:

      m_pihdp->GetIEditable()->BeginUndo();
      m_pihdp->GetIEditable()->MarkForUndo();

      m_fSlingshot = !m_fSlingshot;
      if (m_fSlingshot)
      {
         m_fSmooth = fFalse;
         index2 = (m_pihdp->m_vdpoint.IndexOf(this) + 1) % m_pihdp->m_vdpoint.Size();
         m_pihdp->m_vdpoint.ElementAt(index2)->m_fSmooth = fFalse;
      }

      m_pihdp->GetIEditable()->EndUndo();
      m_pihdp->GetIEditable()->SetDirtyDraw();
      break;
   }
}

void DragPoint::SetSelectFormat(Sur *psur)
{
   psur->SetFillColor(RGB(150,200,255));
}

void DragPoint::SetMultiSelectFormat(Sur *psur)
{
   psur->SetFillColor(RGB(200,225,255));
}

STDMETHODIMP DragPoint::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IControlPoint,
   };

   for (size_t i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
   {
      if (InlineIsEqualGUID(*arr[i],riid))
         return S_OK;
   }
   return S_FALSE;
}

BOOL DragPoint::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(VCEN))
   {
      pbr->GetStruct(&m_v, sizeof(Vertex2D));
   }
   else if (id == FID(SMTH))
   {
      pbr->GetBool(&m_fSmooth);
   }
   else if (id == FID(SLNG))
   {
      pbr->GetBool(&m_fSlingshot);
   }
   else if (id == FID(ATEX))
   {
      pbr->GetBool(&m_fAutoTexture);
   }
   else if (id == FID(TEXC))
   {
      pbr->GetFloat(&m_texturecoord);
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
}

IDispatch *DragPoint::GetDispatch()
{
   return (IDispatch *)this;
}

void DragPoint::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   m_pihdp->GetPointDialogPanes(pvproppane);
}

STDMETHODIMP DragPoint::get_X(float *pVal)
{
   *pVal = m_v.x;

   return S_OK;
}

STDMETHODIMP DragPoint::put_X(float newVal)
{
   STARTUNDOSELECT

      m_v.x = newVal;

   STOPUNDOSELECT

      return S_OK;
}

STDMETHODIMP DragPoint::get_Y(float *pVal)
{
   *pVal = m_v.y;

   return S_OK;
}

STDMETHODIMP DragPoint::put_Y(float newVal)
{
   STARTUNDOSELECT

      m_v.y = newVal;

   STOPUNDOSELECT

      return S_OK;
}


STDMETHODIMP DragPoint::get_Smooth(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fSmooth);

   return S_OK;
}

STDMETHODIMP DragPoint::put_Smooth(VARIANT_BOOL newVal)
{
   STARTUNDOSELECT

      m_fSmooth = VBTOF(newVal);

   STOPUNDOSELECT

      return S_OK;
}

STDMETHODIMP DragPoint::get_IsAutoTextureCoordinate(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fAutoTexture);

   return S_OK;
}

STDMETHODIMP DragPoint::put_IsAutoTextureCoordinate(VARIANT_BOOL newVal)
{
   STARTUNDOSELECT

      m_fAutoTexture = VBTOF(newVal);

   STOPUNDOSELECT

      return S_OK;
}

STDMETHODIMP DragPoint::get_TextureCoordinateU(float *pVal)
{
   *pVal = m_texturecoord;

   return S_OK;
}

STDMETHODIMP DragPoint::put_TextureCoordinateU(float newVal)
{
   STARTUNDOSELECT

      m_texturecoord = newVal;

   STOPUNDOSELECT

      return S_OK;
}

int rotateApplyCount=0;
INT_PTR CALLBACK RotateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ISelect *psel;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      {
         rotateApplyCount=0;
         SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

         psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

         Vertex2D v;			
         psel->GetCenter(&v);
         SetDlgItemInt(hwndDlg, IDC_ROTATEBY, 0, TRUE);
         char szT[256];
         f2sz(v.x, szT);
         SetDlgItemText(hwndDlg, IDC_CENTERX, szT);
         f2sz(v.y, szT);
         SetDlgItemText(hwndDlg, IDC_CENTERY, szT);
      }
      return TRUE;
      break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_COMMAND:
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
            {
               if( rotateApplyCount==0 )
               {
                  char szT[256];
                  GetDlgItemText(hwndDlg, IDC_ROTATEBY, szT, 255);							

                  const float f = sz2f(szT);
                  GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
                  Vertex2D v;
                  v.x = sz2f(szT);
                  GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
                  v.y = sz2f(szT);

                  psel->Rotate(f, &v);
               }
               EndDialog(hwndDlg, TRUE);
               break;
            }
         case IDC_ROTATE_APPLY_BUTTON:
            {
               rotateApplyCount++;
               char szT[256];
               GetDlgItemText(hwndDlg, IDC_ROTATEBY, szT, 255);							
               const float f = sz2f(szT);
               GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
               Vertex2D v;
               v.x = sz2f(szT);
               GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
               v.y = sz2f(szT);

               psel->Rotate(f, &v);
               psel->GetPTable()->SetDirtyDraw();
               break;
            }
         case IDC_ROTATE_UNDO_BUTTON:
            {
               if( rotateApplyCount>0 ) 
               {
                  rotateApplyCount--;
                  psel->GetPTable()->Undo();
                  psel->GetPTable()->SetDirtyDraw();
               }
               break;
            }
         case IDCANCEL:
            if( rotateApplyCount>0 )
            {
               for( int i=0;i<rotateApplyCount;i++ ) psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            EndDialog(hwndDlg, FALSE);
            break;
         }
         break;
      }
      break;
   }

   return FALSE;
}

int scaleApplyCount=0;
INT_PTR CALLBACK ScaleProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ISelect *psel;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      {
         scaleApplyCount=0;
         SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
         psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

         Vertex2D v;			
         psel->GetCenter(&v);
         SetDlgItemInt(hwndDlg, IDC_SCALEFACTOR, 1, TRUE);
         SetDlgItemInt(hwndDlg, IDC_SCALEY, 1, TRUE);
         char szT[256];
         f2sz(v.x, szT);
         SetDlgItemText(hwndDlg, IDC_CENTERX, szT);
         f2sz(v.y, szT);
         SetDlgItemText(hwndDlg, IDC_CENTERY, szT);

         SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_SETCHECK, TRUE, 0);

         HWND hwndEdit = GetDlgItem(hwndDlg, IDC_SCALEY);
         HWND hwndText = GetDlgItem(hwndDlg, IDC_STATIC_SCALEY);
         EnableWindow(hwndEdit, FALSE);
         EnableWindow(hwndText, FALSE);
      }
      return TRUE;
      break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_COMMAND:
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
            {
               if( scaleApplyCount==0 )
               {
                  char szT[256];
                  GetDlgItemText(hwndDlg, IDC_SCALEFACTOR, szT, 255);
                  const float fx = sz2f(szT);
                  const int checked = SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_GETCHECK, 0, 0);
                  float fy;
                  if (checked)
                  {
                     fy = fx;
                  }
                  else
                  {
                     GetDlgItemText(hwndDlg, IDC_SCALEY, szT, 255);
                     fy = sz2f(szT);
                  }

                  GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
                  Vertex2D v;
                  v.x = sz2f(szT);
                  GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
                  v.y = sz2f(szT);

                  //pihdp->ScalePoints(fx, fy, &v);
                  psel->Scale(fx, fy, &v);
               }
               EndDialog(hwndDlg, TRUE);
               break;
            }
         case IDC_SCALE_APPLY_BUTTON:
            {
               scaleApplyCount++;
               char szT[256];
               GetDlgItemText(hwndDlg, IDC_SCALEFACTOR, szT, 255);
               const float fx = sz2f(szT);
               const int checked = SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_GETCHECK, 0, 0);
               float fy;
               if (checked)
               {
                  fy = fx;
               }
               else
               {
                  GetDlgItemText(hwndDlg, IDC_SCALEY, szT, 255);
                  fy = sz2f(szT);
               }

               GetDlgItemText(hwndDlg, IDC_CENTERX, szT, 255);
               Vertex2D v;
               v.x = sz2f(szT);
               GetDlgItemText(hwndDlg, IDC_CENTERY, szT, 255);
               v.y = sz2f(szT);

               //pihdp->ScalePoints(fx, fy, &v);
               psel->Scale(fx, fy, &v);
               psel->GetPTable()->SetDirtyDraw();
               break;
            }
         case IDC_SCALE_UNDO_BUTTON:
            {
               if( scaleApplyCount>0 )
               {
                  scaleApplyCount--;
                  psel->GetPTable()->Undo();
                  psel->GetPTable()->SetDirtyDraw();
               }
               break;
            }
         case IDCANCEL:
            if( scaleApplyCount>0 )
            {
               for( int i=0;i<scaleApplyCount;i++) psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            EndDialog(hwndDlg, FALSE);
            break;

         case IDC_SQUARE:
            {
               int checked = SendDlgItemMessage(hwndDlg, IDC_SQUARE, BM_GETCHECK, 0, 0);
               HWND hwndEdit = GetDlgItem(hwndDlg, IDC_SCALEY);
               HWND hwndText = GetDlgItem(hwndDlg, IDC_STATIC_SCALEY);

               EnableWindow(hwndEdit, !(checked == BST_CHECKED));
               EnableWindow(hwndText, !(checked == BST_CHECKED));
            }
            break;
         }
         break;
      }
      break;
   }

   return FALSE;
}

int translateApplyCount=0;
INT_PTR CALLBACK TranslateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   ISelect *psel;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      {
         translateApplyCount=0;
         SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

         SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
         psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

         char szT[256];
         f2sz(0, szT);
         SetDlgItemText(hwndDlg, IDC_OFFSETX, szT);
         f2sz(0, szT);
         SetDlgItemText(hwndDlg, IDC_OFFSETY, szT);
      }
      return TRUE;
      break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_COMMAND:
      psel = (ISelect *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
            {
               if( translateApplyCount==0 )
               {
                  char szT[256];							
                  GetDlgItemText(hwndDlg, IDC_OFFSETX, szT, 255);
                  Vertex2D v;
                  v.x = sz2f(szT);
                  GetDlgItemText(hwndDlg, IDC_OFFSETY, szT, 255);
                  v.y = sz2f(szT);

                  psel->Translate(&v);
               }
               EndDialog(hwndDlg, TRUE);
               break;
            }
         case IDC_TRANSLATE_APPLY_BUTTON:
            {
               translateApplyCount++;
               char szT[256];							
               GetDlgItemText(hwndDlg, IDC_OFFSETX, szT, 255);
               Vertex2D v;
               v.x = sz2f(szT);
               GetDlgItemText(hwndDlg, IDC_OFFSETY, szT, 255);
               v.y = sz2f(szT);

               psel->Translate(&v);
               psel->GetPTable()->SetDirtyDraw();
               break;
            }
         case IDC_TRANSLATE_UNDO_BUTTON:
            {
               if( translateApplyCount>0 )
               {
                  translateApplyCount--;
                  psel->GetPTable()->Undo();
                  psel->GetPTable()->SetDirtyDraw();
               }
               break;
            }
         case IDCANCEL:
            if( translateApplyCount>0 )
            {
               for( int i=0;i<translateApplyCount;i++ ) psel->GetPTable()->Undo();
               psel->GetPTable()->SetDirtyDraw();
            }
            EndDialog(hwndDlg, FALSE);
            break;
         }
         break;
      }
      break;
   }

   return FALSE;
}
