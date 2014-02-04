#include "StdAfx.h"

Gate::Gate()
{
   m_phitgate = NULL;
   m_plineseg = NULL;
   m_d.m_fEnableLighting = fTrue;
   staticMaterial.setAmbient(0.0f, 0.6f, 0.6f, 0.6f );
   staticMaterial.setDiffuse(0.0f, 0.6f, 0.6f, 0.6f );
   litVertices = 0;
   nolitVertices = 0;
}

Gate::~Gate()
{
   if(litVertices)
	   delete [] litVertices;
   if(nolitVertices)
	   delete [] nolitVertices;
}

HRESULT Gate::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   HRESULT hr = S_OK;

   m_ptable = ptable;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   SetDefaults(fromMouseClick);

   InitVBA(fTrue, 0, NULL);

   return hr;
}

void Gate::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Gate","Length", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_length = fTmp;
   else
      m_d.m_length = 100;

   hr = GetRegStringAsFloat("DefaultProps\\Gate","Height", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_height = fTmp;
   else
      m_d.m_height = 50;

   hr = GetRegStringAsFloat("DefaultProps\\Gate","Rotation", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rotation = fTmp;
   else
      m_d.m_rotation = -90;

   hr = GetRegInt("DefaultProps\\Gate","Supports", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fSupports = iTmp == 0 ? false : true;
   else
      m_d.m_fSupports = fTrue;

   hr = GetRegInt("DefaultProps\\Gate","Collidable", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCollidable = iTmp == 0 ? false : true;
   else
      m_d.m_fCollidable = fTrue;

   hr = GetRegStringAsFloat("DefaultProps\\Gate","AngleMin", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_angleMin = fTmp;
   else
      m_d.m_angleMin = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Gate","AngleMax", &fTmp);
   if ((hr == S_OK)  && fromMouseClick)
      m_d.m_angleMax = fTmp;
   else
      m_d.m_angleMax = (float)(M_PI/2.0);

   hr = GetRegInt("DefaultProps\\Gate","Visible", &iTmp);
   if ((hr == S_OK)  && fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fVisible = fTrue;

   hr = GetRegInt("DefaultProps\\Gate","Animations", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_animations = iTmp;
   else
      m_d.m_animations = 0;	// animations frames, zero will calculate 1 frames per 6 degrees

   hr = GetRegInt("DefaultProps\\Gate","Color", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_color = iTmp;
   else
      m_d.m_color = RGB(128,128,128);

   hr = GetRegInt("DefaultProps\\Gate","TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = false;

   hr = GetRegInt("DefaultProps\\Gate","TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegString("DefaultProps\\Gate","Surface", &m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Gate","Elasticity", &fTmp);
   if ((hr == S_OK)  && fromMouseClick)
      m_d.m_elasticity = fTmp;
   else
      m_d.m_elasticity = 0.3f;

   hr = GetRegStringAsFloat("DefaultProps\\Gate","Friction", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_friction =  fTmp;
   else
      m_d.m_friction = 0;	//zero uses global value

   hr = GetRegStringAsFloat("DefaultProps\\Gate","Scatter", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_scatter = fTmp;
   else
      m_d.m_scatter = 0;	//zero uses global value

   hr = GetRegString("DefaultProps\\Gate","ImageFront", &m_d.m_szImageFront, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImageFront[0] = 0;

   hr = GetRegString("DefaultProps\\Gate","ImageBack", &m_d.m_szImageBack, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImageBack[0] = 0;

   hr = GetRegInt("DefaultProps\\Gate","EnableLighting", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fEnableLighting = iTmp == 0 ? false : true;
   else
      m_d.m_fEnableLighting = true;
}


void Gate::WriteRegDefaults()
{
   char strTmp[MAXTOKEN];

   sprintf_s(strTmp, 40, "%f", m_d.m_length);
   SetRegValue("DefaultProps\\Gate","Length", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_height);
   SetRegValue("DefaultProps\\Gate","Height", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_rotation);
   SetRegValue("DefaultProps\\Gate","Rotation", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue("DefaultProps\\Gate","Supports",REG_DWORD,&m_d.m_fSupports,4);
   SetRegValue("DefaultProps\\Gate","Collidable",REG_DWORD,&m_d.m_fCollidable,4);
   sprintf_s(strTmp, 40, "%f", m_d.m_angleMin);
   SetRegValue("DefaultProps\\Gate","AngleMin", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_angleMax);
   SetRegValue("DefaultProps\\Gate","AngleMax", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue("DefaultProps\\Gate","Visible",REG_DWORD,&m_d.m_fVisible,4);
   SetRegValue("DefaultProps\\Gate","Animations",REG_DWORD,&m_d.m_animations,4);
   SetRegValue("DefaultProps\\Gate","Color",REG_DWORD,&m_d.m_color,4);
   SetRegValue("DefaultProps\\Gate","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue("DefaultProps\\Gate","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue("DefaultProps\\Gate","Surface", REG_SZ, &m_d.m_szSurface,strlen(m_d.m_szSurface));
   sprintf_s(strTmp, 40, "%f", m_d.m_elasticity);
   SetRegValue("DefaultProps\\Gate","Elasticity", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_friction);
   SetRegValue("DefaultProps\\Gate","Friction", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_scatter);
   SetRegValue("DefaultProps\\Gate","Scatter", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue("DefaultProps\\Gate","ImageFront", REG_SZ, &m_d.m_szImageFront,strlen(m_d.m_szImageFront));
   SetRegValue("DefaultProps\\Gate","ImageBack", REG_SZ, &m_d.m_szImageBack,strlen(m_d.m_szImageBack));
   SetRegValue("DefaultProps\\Gate","EnableLighting",REG_DWORD,&m_d.m_fEnableLighting,4);
}

void Gate::PreRender(Sur * const psur)
{
}

void Gate::Render(Sur * const psur)
{
   psur->SetLineColor(RGB(0,0,0),false,2);
   psur->SetObject(this);

   float halflength = m_d.m_length * 0.5f;	

   Vertex2D tmp;

   const float radangle = ANGTORAD(m_d.m_rotation);
   {
      const float sn = sinf(radangle);
      const float cs = cosf(radangle);

      psur->Line(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength,
         m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength);

      // Draw Arrow

      psur->SetLineColor(RGB(0,0,0),false,1);

      halflength *= 0.5f;

      tmp.x = m_d.m_vCenter.x + sn*halflength;
      tmp.y = m_d.m_vCenter.y - cs*halflength;

      psur->Line(tmp.x, tmp.y,
         m_d.m_vCenter.x, m_d.m_vCenter.y);
   }

   halflength *= 0.5f;

   {
      const float arrowang = radangle+0.6f;
      const float sn = sinf(arrowang);
      const float cs = cosf(arrowang);

      psur->Line(tmp.x, tmp.y,
         m_d.m_vCenter.x + sn*halflength, m_d.m_vCenter.y - cs*halflength);
   }

   const float arrowang = radangle-0.6f;
   const float sn = sinf(arrowang);
   const float cs = cosf(arrowang);

   psur->Line(tmp.x, tmp.y,
      m_d.m_vCenter.x + sn*halflength, m_d.m_vCenter.y - cs*halflength);
}

void Gate::RenderBlueprint(Sur *psur)
{
}

void Gate::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_fTimerEnabled)
   {
      pvht->AddElement(pht);
   }
}

void Gate::GetHitShapes(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   const float h = m_d.m_height;		//relative height of the gate 

   float halflength = m_d.m_length * 0.5f;	

   const float angleMin = min(m_d.m_angleMin, m_d.m_angleMax); // correct angle inversions
   const float angleMax = max(m_d.m_angleMin, m_d.m_angleMax);

   m_d.m_angleMin = angleMin;	
   m_d.m_angleMax = angleMax;

   const float radangle = ANGTORAD(m_d.m_rotation);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   const Vertex2D rgv[2] = {
      Vertex2D(m_d.m_vCenter.x + cs*(halflength + (float)PHYS_SKIN),//oversize by the ball's radius
      m_d.m_vCenter.y + sn*(halflength + (float)PHYS_SKIN)),// to prevent the ball from clipping through

      Vertex2D(m_d.m_vCenter.x - cs*(halflength + (float)PHYS_SKIN),//the gate's edge
      m_d.m_vCenter.y - sn*(halflength + (float)PHYS_SKIN))};

      m_plineseg = new LineSeg();

      m_plineseg->m_pfe = NULL;

      m_plineseg->m_rcHitRect.zlow = height;
      m_plineseg->m_rcHitRect.zhigh = height + (float)(2.0*PHYS_SKIN); //+50.0f //ball diameter

      m_plineseg->v1.x = rgv[0].x;
      m_plineseg->v1.y = rgv[0].y;

      m_plineseg->v2.x = rgv[1].x;
      m_plineseg->v2.y = rgv[1].y;

      m_plineseg->CalcNormal();

      m_plineseg->m_elasticity = m_d.m_elasticity;
      m_plineseg->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
      m_plineseg->m_scatter = m_d.m_scatter;

      pvho->AddElement(m_plineseg);

      m_plineseg->m_fEnabled = m_d.m_fCollidable;	

      m_phitgate = new HitGate(this);

      m_phitgate->m_pfe = (IFireEvents *)this;

      m_phitgate->m_rcHitRect.zlow = height;
      m_phitgate->m_rcHitRect.zhigh = height + h; //+50;

      m_phitgate->v1.x = rgv[1].x;
      m_phitgate->v1.y = rgv[1].y;

      m_phitgate->v2.x = rgv[0].x;
      m_phitgate->v2.y = rgv[0].y;

      m_phitgate->CalcNormal();

      m_phitgate->m_elasticity = m_d.m_elasticity;
      m_phitgate->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
      m_phitgate->m_scatter = m_d.m_scatter;

      pvho->AddElement(m_phitgate);

      m_phitgate->m_fEnabled = m_d.m_fCollidable;

      if(m_d.m_fSupports)
      {
         {
            HitCircle * const phitcircle = new HitCircle();
            phitcircle->m_pfe = NULL;
            phitcircle->center.x = m_d.m_vCenter.x + cs*halflength;
            phitcircle->center.y = m_d.m_vCenter.y + sn*halflength;
            phitcircle->radius = 0.01f;
            phitcircle->zlow = height;
            phitcircle->zhigh = height+h; //+50;

            pvho->AddElement(phitcircle);
         }

         HitCircle * const phitcircle = new HitCircle();
         phitcircle->m_pfe = NULL;
         phitcircle->center.x = m_d.m_vCenter.x - cs*halflength;
         phitcircle->center.y = m_d.m_vCenter.y - sn*halflength;
         phitcircle->radius = 0.01f;
         phitcircle->zlow = height;
         phitcircle->zhigh = height+h; //+50;
         pvho->AddElement(phitcircle);
      }
}

void Gate::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Gate::EndPlay()
{
   IEditable::EndPlay();

   if (m_phitgate) // Failed Player case
   {
      for (int i=0;i<m_phitgate->m_gateanim.m_vddsFrame.Size();i++)
      {
         delete m_phitgate->m_gateanim.m_vddsFrame.ElementAt(i);
      }

      m_phitgate = NULL;
   }

   m_plineseg = NULL;
}

void Gate::PostRenderStatic(const RenderDevice* pd3dDevice)
{
}

static const WORD rgiGate0[8] = {0,1,2,3,6,7,4,5};
static const WORD rgiGate1[8] = {4,5,6,7,2,3,0,1};
static const WORD rgiGateNormal[3] = {0,1,3};

static const WORD rgiGate2[4] = {0,1,5,4};
static const WORD rgiGate3[4] = {2,6,7,3};
static const WORD rgiGate4[4] = {0,2,3,1};
static const WORD rgiGate5[4] = {4,5,7,6};
static const WORD rgiGate6[4] = {0,4,6,2};
static const WORD rgiGate7[4] = {1,3,7,5};

void Gate::PrepareStatic(RenderDevice* pd3dDevice)
{
   if(!m_d.m_fSupports) return; // no support structures are allocated ... therefore render none

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   const float halflength = m_d.m_length * 0.5f;// + m_d.m_overhang;
   const float halfthick = 2.0f;
   const float h = m_d.m_height;

   const float radangle = ANGTORAD(m_d.m_rotation);
   const float snY = sinf(radangle);
   const float csY = cosf(radangle);

   staticVertices[0].x = -halflength + halfthick;
   staticVertices[0].y = 0;
   staticVertices[0].z = 0;

   staticVertices[1].x = -halflength - halfthick;
   staticVertices[1].y = 0;
   staticVertices[1].z = 0;

   staticVertices[2].x = -halflength + halfthick;
   staticVertices[2].y = 0;
   staticVertices[2].z = h - halfthick;

   staticVertices[3].x = -halflength - halfthick;
   staticVertices[3].y = 0;
   staticVertices[3].z = h + halfthick;

   staticVertices[4].x = halflength - halfthick;
   staticVertices[4].y = 0;
   staticVertices[4].z = 0;

   staticVertices[5].x = halflength + halfthick;
   staticVertices[5].y = 0;
   staticVertices[5].z = 0;

   staticVertices[6].x = halflength - halfthick;
   staticVertices[6].y = 0;
   staticVertices[6].z = h - halfthick;

   staticVertices[7].x = halflength + halfthick;
   staticVertices[7].y = 0;
   staticVertices[7].z = h + halfthick;

   for (int l=0;l<8;l++)
   {
      const float temp = staticVertices[l].x;
      staticVertices[l].x = csY*temp - snY*staticVertices[l].y;
      staticVertices[l].y = csY*staticVertices[l].y + snY*temp;

      staticVertices[l].x += m_d.m_vCenter.x;
      staticVertices[l].y += m_d.m_vCenter.y;
      staticVertices[l].z += height*m_ptable->m_zScale;

      ppin3d->m_lightproject.CalcCoordinates(&staticVertices[l]);
   }
}

void Gate::PrepareMovers(RenderDevice* pd3dDevice )
{
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   COLORREF rgbTransparent = RGB(255,0,255); //RGB(0,0,0);

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);//surface gate is on
   const float h = m_d.m_height;		//relative height of the gate 

   Texture * const pinback = m_ptable->GetImage(m_d.m_szImageBack);
   float maxtuback, maxtvback;
   if (pinback)
   {
      m_ptable->GetTVTU(pinback, &maxtuback, &maxtvback);
   }
   else
   {
      maxtuback = maxtvback = 1.0f;
   }

   Texture * const pinfront = m_ptable->GetImage(m_d.m_szImageFront);
   float maxtufront, maxtvfront;
   if (pinfront)
   {
      m_ptable->GetTVTU(pinfront, &maxtufront, &maxtvfront);
   }
   else
   {
      maxtufront = maxtvfront = 1.0f;
   }

   int cframes;
   if (m_d.m_animations > 0)
      cframes = m_d.m_animations;
   else if (m_d.m_angleMax != 0.0f || m_d.m_angleMin != 0.0f)
      cframes = (int)((m_d.m_angleMax - m_d.m_angleMin)*(float)((15-1)*2/M_PI) + 1.5f); // 15 frames per 90 degrees
   else
      cframes = 1;

   frameCount=cframes;

   const float halflength = m_d.m_length * 0.5f;
   const float halfwidth =  m_d.m_height; //50;
   const float minx = -halflength;
   const float maxx = halflength;
   const float miny = -1.0f;
   const float maxy = 1.0f;
   const float minz = -halfwidth;
   const float maxz = 0;

   const float radangle = ANGTORAD(m_d.m_rotation);
   const float snY = sinf(radangle);
   const float csY = cosf(radangle);

   ppin3d->ClearExtents(&m_phitgate->m_gateanim.m_rcBounds, &m_phitgate->m_gateanim.m_znear, &m_phitgate->m_gateanim.m_zfar);

   const float inv_cframes = (cframes > 1) ? ((m_d.m_angleMax - m_d.m_angleMin)/(float)(cframes-1)) : 0.0f;

   if(litVertices)
	   delete [] litVertices;
   if(nolitVertices)
	   delete [] nolitVertices;

   if( m_d.m_fEnableLighting )
   {
      litVertices = new Vertex3D[8*frameCount];
   }
   else
   {
      nolitVertices = new Vertex3D_NoLighting[8*frameCount];
   }

   int ofs=0;
   for (int i=0;i<cframes;i++, ofs+=8)
   {
      const float angle = m_d.m_angleMin + inv_cframes*(float)i;

      const float snTurn = sinf(angle);
      const float csTurn = cosf(angle);

      if(m_d.m_fEnableLighting)
      {
         Vertex3D *rgv3D=&litVertices[ofs];
         for (int l=0;l<8;l++)
         {
            rgv3D[l].x = (l & 1) ? maxx : minx;
            rgv3D[l].y = (l & 2) ? maxy : miny;
            rgv3D[l].z = (l & 4) ? maxz : minz;

            if (l & 2)
            {
               rgv3D[l].tu = (l & 1) ? maxtufront : 0;
               rgv3D[l].tv = (l & 4) ? 0 : maxtvfront;
            }
            else
            {
               rgv3D[l].tu = (l & 1) ? maxtuback : 0;
               rgv3D[l].tv = (l & 4) ? maxtvback : 0;
            }
            {
               const float temp = rgv3D[l].y;
               rgv3D[l].y = csTurn*temp + snTurn*rgv3D[l].z;
               rgv3D[l].z = csTurn*rgv3D[l].z - snTurn*temp;
            }

            {
               const float temp = rgv3D[l].x;
               rgv3D[l].x = csY*temp - snY*rgv3D[l].y;
               rgv3D[l].y = csY*rgv3D[l].y + snY*temp;
            }

            rgv3D[l].x += m_d.m_vCenter.x;
            rgv3D[l].y += m_d.m_vCenter.y;
            //rgv3D[l].z += height + 50.0f;
            rgv3D[l].z += height + h;
            rgv3D[l].z *= m_ptable->m_zScale;

            ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
         }

         // Draw Backside
         if (pinback) //(pinback)
         {			
            pinback->CreateAlphaChannel();
         }

         // Draw Frontside
         if (pinfront) 
         {			
            pinfront->CreateAlphaChannel();
         }
      }
      else // copy pasted from above, just without lighting
      {
         Vertex3D_NoLighting *rgv3D=&nolitVertices[ofs];
         for (int l=0;l<8;l++)
         {
            rgv3D[l].x = (l & 1) ? maxx : minx;
            rgv3D[l].y = (l & 2) ? maxy : miny;
            rgv3D[l].z = (l & 4) ? maxz : minz;

            if (l & 2)
            {
               rgv3D[l].tu = (l & 1) ? maxtufront : 0;
               rgv3D[l].tv = (l & 4) ? 0 : maxtvfront;
            }
            else
            {
               rgv3D[l].tu = (l & 1) ? maxtuback : 0;
               rgv3D[l].tv = (l & 4) ? maxtvback : 0;
            }
            {
               const float temp = rgv3D[l].y;
               rgv3D[l].y = csTurn*temp + snTurn*rgv3D[l].z;
               rgv3D[l].z = csTurn*rgv3D[l].z - snTurn*temp;
            }

            {
               const float temp = rgv3D[l].x;
               rgv3D[l].x = csY*temp - snY*rgv3D[l].y;
               rgv3D[l].y = csY*rgv3D[l].y + snY*temp;
            }

            rgv3D[l].x += m_d.m_vCenter.x;
            rgv3D[l].y += m_d.m_vCenter.y;
            //rgv3D[l].z += height + 50.0f;
            rgv3D[l].z += height + h;
            rgv3D[l].z *= m_ptable->m_zScale;

            rgv3D[l].color = m_d.m_color;
         }

         // Draw Backside
         if (pinback) //(pinback)
         {			
            pinback->CreateAlphaChannel();
         }
         // Draw Frontside
         if (pinfront) 
         {			
            pinfront->CreateAlphaChannel();
         }
      }
   }

}
void Gate::RenderSetup(const RenderDevice* _pd3dDevice)
{
   solidMaterial.setColor( 1.0f, m_d.m_color );
   PrepareStatic((RenderDevice*)_pd3dDevice);
   PrepareMovers((RenderDevice*)_pd3dDevice);
}

void Gate::RenderStatic(const RenderDevice* _pd3dDevice) // only the support structures are rendered here
{
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;
   if(!m_d.m_fSupports) return; // no support structures are allocated ... therefore render none

   staticMaterial.set();
   Vertex3D rgv3D[8];
   memcpy( rgv3D, staticVertices, sizeof(Vertex3D)*8);
   static const WORD rgiGate0[8] = {0,1,2,3,6,7,4,5};
   static const WORD rgiGate1[8] = {4,5,6,7,2,3,0,1};
   SetNormal(rgv3D, rgiGateNormal, 3, rgv3D, rgiGate0, 8);
   pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_VERTEX,rgv3D,8,(LPWORD)rgiGate0, 8, 0);

   SetNormal(rgv3D, rgiGateNormal, 3, rgv3D, rgiGate1, 8);
   pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_VERTEX,rgv3D,8,(LPWORD)rgiGate1, 8, 0);
}

void Gate::RenderMovers(const RenderDevice* _pd3dDevice)
{
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   COLORREF rgbTransparent = RGB(255,0,255); //RGB(0,0,0);

   Texture * const pinback = m_ptable->GetImage(m_d.m_szImageBack);
   Texture * const pinfront = m_ptable->GetImage(m_d.m_szImageFront);

   if (g_pvp->m_pdd.m_fHardwareAccel)
   {
      pd3dDevice->SetRenderState(RenderDevice::ALPHAREF, 0x80);
      pd3dDevice->SetRenderState(RenderDevice::ALPHAFUNC, D3DCMP_GREATER);
      pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, TRUE);
   }
   else
   {
      //modified to correct software render of plain gates
      ppin3d->SetColorKeyEnabled(FALSE);
   }
   // Set texture to mirror, so the alpha state of the texture blends correctly to the outside
   pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_ADDRESS, D3DTADDRESS_MIRROR);

   //ppin3d->ClearExtents(&m_phitgate->m_gateanim.m_rcBounds, &m_phitgate->m_gateanim.m_znear, &m_phitgate->m_gateanim.m_zfar);
   ppin3d->ClearSpriteRectangle( &m_phitgate->m_gateanim, NULL );

   if(!m_d.m_fEnableLighting)
      pd3dDevice->SetRenderState(RenderDevice::LIGHTING, FALSE);

   static const WORD idx[24] = {0,1,2,0,1,2, 4,5,6,4,6,7, 8,9,10,8,10,11, 12,13,14,12,14,15 };

   int ofs=0;
   for (int i=0;i<frameCount;i++, ofs+=8)
   {
      ObjFrame * const pof = new ObjFrame(); 

      if(m_d.m_fEnableLighting)
      {
         Vertex3D verts[16];
         Vertex3D rgv3D[8];
         memcpy(rgv3D,&litVertices[ofs],sizeof(Vertex3D)*8);

         ppin3d->ClearSpriteRectangle( NULL, pof );
         ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitgate->m_gateanim.m_znear, &m_phitgate->m_gateanim.m_zfar, 8, fFalse);

         // Draw Backside
         if (pinback) //(pinback)
         {			
            pinback->CreateAlphaChannel();
            pinback->Set( ePictureTexture );

            if (pinback->m_fTransparent)
            {				
               pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);				
               if (m_d.m_color != rgbTransparent)
                  rgbTransparent = pinback->m_rgbTransparent;
            }
            else 
            {	
               ppin3d->EnableAlphaBlend( 0x80, fFalse );
               pd3dDevice->SetRenderState(RenderDevice::ALPHAFUNC, D3DCMP_GREATER);
/*               pd3dDevice->SetRenderState(RenderDevice::DITHERENABLE, TRUE); 	
               pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
               pd3dDevice->SetRenderState(RenderDevice::ALPHAREF, 0x80);
               pd3dDevice->SetRenderState(RenderDevice::ALPHAFUNC, D3DCMP_GREATER);
               pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, TRUE);
               pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
               pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCALPHA); 
*/
            }

            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, (m_d.m_color == rgbTransparent || m_d.m_color == NOTRANSCOLOR) ? D3DCULL_CCW : D3DCULL_NONE);
            ppin3d->SetColorKeyEnabled(TRUE);
            pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
            ppin3d->SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

            textureMaterial.set();
         }
         else // No image by that name  
         {
            ppin3d->SetTexture(NULL);
            solidMaterial.set();
         }
         SetNormal(rgv3D, rgiGate2, 4, NULL, NULL, 0);
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 6,(LPWORD)rgiGate2, 4, 0);

         // Draw Frontside
         if (pinfront) 
         {			
            pinfront->CreateAlphaChannel();
            pinfront->Set( ePictureTexture );
            if (pinfront->m_fTransparent)
            {				
               pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);
               if (m_d.m_color != rgbTransparent)
                  rgbTransparent = pinfront->m_rgbTransparent;
            }
            else 
            {	
               ppin3d->EnableAlphaBlend( 1,fFalse );
            }

            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, (m_d.m_color == rgbTransparent || m_d.m_color == NOTRANSCOLOR) ? D3DCULL_CCW : D3DCULL_NONE);
            ppin3d->SetColorKeyEnabled(TRUE);
            pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
            ppin3d->SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

            textureMaterial.set();
         }
         else // No image by that name  
         {
            ppin3d->SetTexture(NULL);
            solidMaterial.set();
         }

         SetNormal(rgv3D, rgiGate3, 4, NULL, NULL, 0);
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 8,(LPWORD)rgiGate3, 4, 0);

         solidMaterial.set();
         ppin3d->SetTexture(NULL);


         if (m_d.m_color != rgbTransparent && m_d.m_color != NOTRANSCOLOR)
         {
            // Top & Bottom
            SetNormal(rgv3D, rgiGate4, 4, NULL, NULL, 0);
            memcpy(&verts[0], &rgv3D[rgiGate4[0]], sizeof(Vertex3D));
            memcpy(&verts[1], &rgv3D[rgiGate4[1]], sizeof(Vertex3D));
            memcpy(&verts[2], &rgv3D[rgiGate4[2]], sizeof(Vertex3D));
            memcpy(&verts[3], &rgv3D[rgiGate4[3]], sizeof(Vertex3D));
            //pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 4,(LPWORD)rgiGate4, 4, 0);

            SetNormal(rgv3D, rgiGate5, 4, NULL, NULL, 0);
            memcpy(&verts[4], &rgv3D[rgiGate5[0]], sizeof(Vertex3D));
            memcpy(&verts[5], &rgv3D[rgiGate5[1]], sizeof(Vertex3D));
            memcpy(&verts[6], &rgv3D[rgiGate5[2]], sizeof(Vertex3D));
            memcpy(&verts[7], &rgv3D[rgiGate5[3]], sizeof(Vertex3D));
            //pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 8,(LPWORD)rgiGate5, 4, 0);
            
            // Sides
            SetNormal(rgv3D, rgiGate6, 4, NULL, NULL, 0);
            memcpy(&verts[8], &rgv3D[rgiGate6[0]], sizeof(Vertex3D));
            memcpy(&verts[9], &rgv3D[rgiGate6[1]], sizeof(Vertex3D));
            memcpy(&verts[10], &rgv3D[rgiGate6[2]], sizeof(Vertex3D));
            memcpy(&verts[11], &rgv3D[rgiGate6[3]], sizeof(Vertex3D));
            //pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 7,(LPWORD)rgiGate6, 4, 0);

            SetNormal(rgv3D, rgiGate7, 4, NULL, NULL, 0);
            memcpy(&verts[12], &rgv3D[rgiGate7[0]], sizeof(Vertex3D));
            memcpy(&verts[13], &rgv3D[rgiGate7[1]], sizeof(Vertex3D));
            memcpy(&verts[14], &rgv3D[rgiGate7[2]], sizeof(Vertex3D));
            memcpy(&verts[15], &rgv3D[rgiGate7[3]], sizeof(Vertex3D));
            //pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 8,(LPWORD)rgiGate7, 4, 0);
            pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_VERTEX,verts, 16,(LPWORD)idx, 24, 0);
         }
      }
      else // copy pasted from above, just without lighting
      {
         Vertex3D_NoLighting rgv3D[8];
         Vertex3D_NoLighting verts[16];

         memcpy( rgv3D, &nolitVertices[ofs],sizeof(Vertex3D_NoLighting)*8);

         ppin3d->ClearSpriteRectangle( NULL, pof );
         ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitgate->m_gateanim.m_znear, &m_phitgate->m_gateanim.m_zfar, 8, fFalse);

         // Draw Backside
         if (pinback) //(pinback)
         {			
            pinback->CreateAlphaChannel();
            pinback->Set( ePictureTexture );
            if (pinback->m_fTransparent)
            {				
               pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);				
               if (m_d.m_color != rgbTransparent)
                  rgbTransparent = pinback->m_rgbTransparent;
            }
            else 
            {	
               ppin3d->EnableAlphaBlend( 0x80, fFalse );
               pd3dDevice->SetRenderState(RenderDevice::ALPHAFUNC, D3DCMP_GREATER);
/*               pd3dDevice->SetRenderState(RenderDevice::DITHERENABLE, TRUE); 	
               pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
               pd3dDevice->SetRenderState(RenderDevice::ALPHAREF, 0x80);
               pd3dDevice->SetRenderState(RenderDevice::ALPHAFUNC, D3DCMP_GREATER);
               pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, TRUE);
               pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
               pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCALPHA); 
*/
            }

            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, (m_d.m_color == rgbTransparent || m_d.m_color == NOTRANSCOLOR) ? D3DCULL_CCW : D3DCULL_NONE);
            g_pplayer->m_pin3d.SetColorKeyEnabled(TRUE);
            pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
            g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

            textureMaterial.set();
         }
         else // No image by that name  
         {
            ppin3d->SetTexture(NULL);
            solidMaterial.set();
         }

         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_NOLIGHTING_VERTEX,rgv3D, 6,(LPWORD)rgiGate2, 4, 0);

         // Draw Frontside
         if (pinfront) 
         {			
            pinfront->CreateAlphaChannel();
            //pd3dDevice->SetTexture(ePictureTexture, pinfront->m_pdsBufferColorKey);
            pinfront->Set( ePictureTexture );
            if (pinfront->m_fTransparent)
            {				
               pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);
               if (m_d.m_color != rgbTransparent)
                  rgbTransparent = pinfront->m_rgbTransparent;
            }
            else 
            {	
               ppin3d->EnableAlphaBlend(1,fFalse);
            }

            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, (m_d.m_color == rgbTransparent || m_d.m_color == NOTRANSCOLOR) ? D3DCULL_CCW : D3DCULL_NONE);
            ppin3d->SetColorKeyEnabled(TRUE);
            pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
            ppin3d->SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

            textureMaterial.set();
         }
         else // No image by that name  
         {
            ppin3d->SetTexture(NULL);
            solidMaterial.set();
         }

         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_NOLIGHTING_VERTEX,rgv3D, 8,(LPWORD)rgiGate3, 4, 0);
         solidMaterial.set();
         ppin3d->SetTexture(NULL);

         if (m_d.m_color != rgbTransparent && m_d.m_color != NOTRANSCOLOR)
         {
            // Top & Bottom
            memcpy(&verts[0], &rgv3D[rgiGate4[0]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[1], &rgv3D[rgiGate4[1]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[2], &rgv3D[rgiGate4[2]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[3], &rgv3D[rgiGate4[3]], sizeof(Vertex3D_NoLighting));

            memcpy(&verts[4], &rgv3D[rgiGate5[0]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[5], &rgv3D[rgiGate5[1]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[6], &rgv3D[rgiGate5[2]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[7], &rgv3D[rgiGate5[3]], sizeof(Vertex3D_NoLighting));

            // Sides
            memcpy(&verts[8], &rgv3D[rgiGate6[0]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[9], &rgv3D[rgiGate6[1]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[10], &rgv3D[rgiGate6[2]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[11], &rgv3D[rgiGate6[3]], sizeof(Vertex3D_NoLighting));

            memcpy(&verts[12], &rgv3D[rgiGate7[0]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[13], &rgv3D[rgiGate7[1]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[14], &rgv3D[rgiGate7[2]], sizeof(Vertex3D_NoLighting));
            memcpy(&verts[15], &rgv3D[rgiGate7[3]], sizeof(Vertex3D_NoLighting));
            pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_NOLIGHTING_VERTEX,verts, 16,(LPWORD)idx, 24, 0);
         }
      }
      ppin3d->CreateAndCopySpriteBuffers( &m_phitgate->m_gateanim, pof );
      m_phitgate->m_gateanim.m_vddsFrame.AddElement(pof);
   }

   g_pplayer->m_pin3d.SetColorKeyEnabled(FALSE);
   pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);
   pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);

   if(!m_d.m_fEnableLighting)
      pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE);
}

void Gate::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Gate::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;

   m_ptable->SetDirtyDraw();
}

void Gate::GetCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_vCenter;
}

void Gate::PutCenter(const Vertex2D * const pv)
{
   m_d.m_vCenter = *pv;

   m_ptable->SetDirtyDraw();
}

HRESULT Gate::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
   bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(LGTH), m_d.m_length);
   bw.WriteFloat(FID(HGTH), m_d.m_height);
   bw.WriteFloat(FID(ROTA), m_d.m_rotation);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteInt(FID(GANM), m_d.m_animations);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteBool(FID(GSUPT), m_d.m_fSupports);
   bw.WriteBool(FID(GCOLD), m_d.m_fCollidable);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(IMGF), m_d.m_szImageFront);
   bw.WriteString(FID(IMGB), m_d.m_szImageBack);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(GAMAX), m_d.m_angleMax);
   bw.WriteFloat(FID(GAMIN), m_d.m_angleMin);
   bw.WriteFloat(FID(GFRCT), m_d.m_friction);
   bw.WriteBool(FID(GVSBL), m_d.m_fVisible);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteBool(FID(ELIT), m_d.m_fEnableLighting);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Gate::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

BOOL Gate::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(VCEN))
   {
      pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex2D));
   }
   else if (id == FID(LGTH))
   {
      pbr->GetFloat(&m_d.m_length);
   }
   else if (id == FID(HGTH))
   {
      pbr->GetFloat(&m_d.m_height);
   }
   else if (id == FID(ROTA))
   {
      pbr->GetFloat(&m_d.m_rotation);
   }
   else if (id == FID(COLR))
   {
      pbr->GetInt(&m_d.m_color);
      //if (!(m_d.m_color & MINBLACKMASK)) {m_d.m_color |= MINBLACK;}	// set minimum black
   }
   else if (id == FID(GANM))
   {
      pbr->GetInt(&m_d.m_animations);
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(GSUPT))
   {
      pbr->GetBool(&m_d.m_fSupports); 
   }
   else if (id == FID(GCOLD))
   {
      pbr->GetBool(&m_d.m_fCollidable); 
   }
   else if (id == FID(ELIT))
   {
      pbr->GetBool(&m_d.m_fEnableLighting); 
   }
   else if (id == FID(GVSBL))
   {
      pbr->GetBool(&m_d.m_fVisible); 
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(IMGF))
   {
      pbr->GetString(m_d.m_szImageFront);
   }
   else if (id == FID(IMGB))
   {
      pbr->GetString(m_d.m_szImageBack);
   }
   else if (id == FID(SURF))
   {
      pbr->GetString(m_d.m_szSurface);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(ELAS))
   {
      pbr->GetFloat(&m_d.m_elasticity);	
   }
   else if (id == FID(GAMAX))
   {
      pbr->GetFloat(&m_d.m_angleMax);
   }
   else if (id == FID(GAMIN))
   {
      pbr->GetFloat(&m_d.m_angleMin);
   }
   else if (id == FID(GFRCT))
   {
      pbr->GetFloat(&m_d.m_friction);
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
}

HRESULT Gate::InitPostLoad()
{
   return S_OK;
}

STDMETHODIMP Gate::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IGate,
   };

   for (size_t i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
   {
      if (InlineIsEqualGUID(*arr[i],riid))
         return S_OK;
   }
   return S_FALSE;
}

STDMETHODIMP Gate::get_Length(float *pVal)
{
   *pVal = m_d.m_length;

   return S_OK;
}

STDMETHODIMP Gate::put_Length(float newVal)
{
   STARTUNDO

      m_d.m_length = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Height(float *pVal)
{
   *pVal = m_d.m_height;

   return S_OK;
}

STDMETHODIMP Gate::put_Height(float newVal)
{
   STARTUNDO

      m_d.m_height = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Rotation(float *pVal)
{
   *pVal = m_d.m_rotation;

   return S_OK;
}

STDMETHODIMP Gate::put_Rotation(float newVal)
{
   STARTUNDO

      m_d.m_rotation = newVal;

   STOPUNDO;

   return S_OK;
}

STDMETHODIMP Gate::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;

   return S_OK;
}

STDMETHODIMP Gate::put_X(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.x = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Gate::put_Y(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.y = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Surface(BSTR *pVal)
{
   WCHAR wz[512];
   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Gate::put_Surface(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Gate::put_Color(OLE_COLOR newVal)
{
   STARTUNDO

      m_d.m_color = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_ImageFront(BSTR *pVal)
{
   WCHAR wz[512];
   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImageFront, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Gate::put_ImageFront(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImageFront, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_ImageBack(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImageBack, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Gate::put_ImageBack(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImageBack, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}


STDMETHODIMP Gate::get_Open(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((m_phitgate) ? m_phitgate->m_gateanim.m_fOpen : fFalse);

   return S_OK;
}

STDMETHODIMP Gate::put_Open(VARIANT_BOOL newVal)
{
   if (m_phitgate)
   {
      m_phitgate->m_gateanim.m_angleMax = m_d.m_angleMax;
      m_phitgate->m_gateanim.m_angleMin = m_d.m_angleMin;

      if (newVal)
      {
         m_phitgate->m_gateanim.m_fOpen = fTrue;
         m_phitgate->m_fEnabled = fFalse;
         m_plineseg->m_fEnabled = fFalse;

         if (m_phitgate->m_gateanim.m_angle < m_phitgate->m_gateanim.m_angleMax)
            m_phitgate->m_gateanim.m_anglespeed = 0.2f;
      }
      else
      {
         m_phitgate->m_gateanim.m_fOpen = fFalse;

         m_phitgate->m_fEnabled = m_d.m_fCollidable;
         m_plineseg->m_fEnabled = m_d.m_fCollidable;					

         if (m_phitgate->m_gateanim.m_angle > m_phitgate->m_gateanim.m_angleMin)
            m_phitgate->m_gateanim.m_anglespeed = -0.2f;
      }
   }

   return S_OK;
}

STDMETHODIMP Gate::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Gate::put_Elasticity(float newVal)
{
   STARTUNDO

      m_d.m_elasticity = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Supports(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fSupports);

   return S_OK;
}

STDMETHODIMP Gate::put_Supports(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fSupports = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_CloseAngle(float *pVal)
{
   *pVal = RADTOANG(g_pplayer ? m_phitgate->m_gateanim.m_angleMin : m_d.m_angleMin);

   return S_OK;
}

STDMETHODIMP Gate::put_CloseAngle(float newVal)
{
   if (m_d.m_fCollidable) newVal = 0;
   else newVal = ANGTORAD(newVal);

   if (g_pplayer)
   {
      if (newVal > m_d.m_angleMax) newVal = m_d.m_angleMax;
      else if (newVal < m_d.m_angleMin) newVal = m_d.m_angleMin;	

      if (m_phitgate->m_gateanim.m_angleMax > newVal)	// max is bigger
         m_phitgate->m_gateanim.m_angleMin = newVal;	//then set new minumum
      else m_phitgate->m_gateanim.m_angleMax = newVal;//else set new max
   }
   else
   {
      STARTUNDO
         m_d.m_angleMin = newVal;
      STOPUNDO
   }
   return S_OK;
}


STDMETHODIMP Gate::get_OpenAngle(float *pVal)
{
   *pVal = RADTOANG((g_pplayer) ? m_phitgate->m_gateanim.m_angleMax : m_d.m_angleMax);	//player active value

   return S_OK;
}

STDMETHODIMP Gate::put_OpenAngle(float newVal)
{
   if (m_d.m_fCollidable) newVal = 0;
   else newVal = ANGTORAD(newVal);

   if (g_pplayer)
   {
      if (newVal > m_d.m_angleMax) newVal = m_d.m_angleMax;
      else if (newVal < m_d.m_angleMin) newVal = m_d.m_angleMin;	

      if (m_phitgate->m_gateanim.m_angleMin < newVal)	// min is smaller
         m_phitgate->m_gateanim.m_angleMax = newVal;	//then set new maximum
      else m_phitgate->m_gateanim.m_angleMin = newVal;//else set new min
   }
   else
   {
      STARTUNDO
         m_d.m_angleMax = newVal;
      STOPUNDO
   }
   return S_OK;
}


STDMETHODIMP Gate::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((g_pplayer) ? m_phitgate->m_fEnabled : m_d.m_fCollidable);

   return S_OK;
}


STDMETHODIMP Gate::put_Collidable(VARIANT_BOOL newVal)
{	
   if (g_pplayer)
   {
      m_phitgate->m_fEnabled = newVal;
      m_plineseg->m_fEnabled = newVal;
      m_phitgate->m_gateanim.m_angleMax = m_d.m_angleMax;
      m_phitgate->m_gateanim.m_angleMin = m_d.m_angleMin;

      if (newVal) m_phitgate->m_gateanim.m_angleMin = 0;
   }
   else
   {
      STARTUNDO

         m_d.m_fCollidable = newVal;

      if (newVal) m_d.m_angleMin = 0;	

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Gate::Move(int dir, float speed, float angle)//move non-collidable gate, for graphic effects only
{
   if (g_pplayer)
   {
      m_phitgate->m_gateanim.m_fOpen = fTrue;	// move always turns off natural swing
      m_phitgate->m_fEnabled = fFalse;		// and collidable off
      m_plineseg->m_fEnabled = fFalse;		

      if (speed <= 0.0f) speed = 0.2f;		//default gate angle speed
      else speed *= (float)(M_PI/180.0);		// convert to radians

      if (!dir || angle != 0)					// if no direction or non-zero angle
      {
         angle *= (float)(M_PI/180.0);		// convert to radians

         if (angle < m_d.m_angleMin) angle = m_d.m_angleMin;
         else if (angle > m_d.m_angleMax) angle = m_d.m_angleMax;

         const float da = angle - m_phitgate->m_gateanim.m_angle; //calc true direction

         if (da > 1.0e-5f) dir = +1;
         else if (da < -1.0e-5f) dir = -1;
         else 
         {
            dir = 0;									// do nothing
            m_phitgate->m_gateanim.m_anglespeed = 0;	//stop 
         }
      }
      else {angle = (dir < 0) ? m_d.m_angleMin : m_d.m_angleMax;}	//dir selected and angle not specified			

      if (dir > 0)
      {
         m_phitgate->m_gateanim.m_angleMax = angle;

         if (m_phitgate->m_gateanim.m_angle < m_phitgate->m_gateanim.m_angleMax)
            m_phitgate->m_gateanim.m_anglespeed = speed;
      }
      else if (dir < 0)
      {
         m_phitgate->m_gateanim.m_angleMin = angle;

         if (m_phitgate->m_gateanim.m_angle > m_phitgate->m_gateanim.m_angleMin)
            m_phitgate->m_gateanim.m_anglespeed = -speed;
      }
   }

   return S_OK;
}

STDMETHODIMP Gate::get_Friction(float *pVal)
{
   *pVal = (g_pplayer) ? m_phitgate->m_gateanim.m_friction : m_d.m_friction;	

   return S_OK;
}

STDMETHODIMP Gate::put_Friction(float newVal)
{	
   if (newVal < 0) newVal = 0;
   else if (newVal > 1.0f) newVal = 1.0f;

   if (g_pplayer)
   {
      m_phitgate->m_gateanim.m_friction = newVal;	
   }
   else
   {
      STARTUNDO

         m_d.m_friction =  newVal;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Gate::get_Animations(int *pVal)
{
   if (!g_pplayer)
   {
      *pVal = m_d.m_animations;
   }

   return S_OK;
}

STDMETHODIMP Gate::put_Animations(int newVal)
{	
   if (!g_pplayer)
   {
      STARTUNDO

         m_d.m_animations = newVal;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Gate::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((g_pplayer) ? m_phitgate->m_gateanim.m_fVisible : m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP Gate::put_Visible(VARIANT_BOOL newVal)
{	
   if (g_pplayer)
   {
      m_phitgate->m_gateanim.m_fVisible = newVal;// && m_d.m_fVisible;
   }
   else
   {
      STARTUNDO

         m_d.m_fVisible = newVal;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Gate::get_EnableLighting(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fEnableLighting);

   return S_OK;
}

STDMETHODIMP Gate::put_EnableLighting(VARIANT_BOOL newVal)
{	
   STARTUNDO

      m_d.m_fEnableLighting = newVal;

   STOPUNDO

      return S_OK;
}

void Gate::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPGATE_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPGATE_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPGATE_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}
