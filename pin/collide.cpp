#include "stdafx.h"

float c_maxBallSpeedSqr = C_SPEEDLIMIT*C_SPEEDLIMIT; 
float c_dampingFriction = C_DAMPFRICTION;
float c_plungerNormalize = C_PLUNGERNORMALIZE;
bool c_plungerFilter = false;

float c_hardScatter = 0.0f;
float c_hardFriction = 1.0f - C_FRICTIONCONST;

HitObject *CreateCircularHitPoly(const float x, const float y, const float z, const float r, const int sections)
	{
	Vertex3Ds * const rgv3d = new Vertex3Ds[sections];

	const float inv_sections = (float)(M_PI*2.0)/(float)sections;
	
	for (int i=0; i<sections; ++i)
		{
		const float angle = inv_sections * (float)i;

		rgv3d[i].x = x + sinf(angle) * r;
		rgv3d[i].y = y + cosf(angle) * r;
		rgv3d[i].z = z;
		}

	return new Hit3DPoly(rgv3d, sections);
	}

HitObject::HitObject() : m_fEnabled(fTrue), m_ObjType(eNull), m_pObj(NULL),
						 m_elasticity(0.3f), m_pfedebug(NULL)
	{
	}

void LineSeg::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left = min(v1.x, v2.x);
	m_rcHitRect.right = max(v1.x, v2.x);
	m_rcHitRect.top = min(v1.y, v2.y);
	m_rcHitRect.bottom = max(v1.y, v2.y);

	//m_rcHitRect.zlow = 0; //!!?
	//m_rcHitRect.zhigh = 50;
	}

float LineSeg::HitTestBasic(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal, const bool direction, const bool lateral, const bool rigid)
	{
	if (!m_fEnabled || pball->fFrozen) return -1.0f;	

	const float ballvx = pball->vx;							// ball velocity
	const float ballvy = pball->vy;

	const float bnv = ballvx*normal.x + ballvy*normal.y;	// ball velocity normal to segment, positive if receding, zero=parallel
	bool bUnHit = (bnv > C_LOWNORMVEL);

	if (direction && bUnHit)								// direction true and clearly receding from normal face
		{
		return -1.0f;
		}

	const float ballx = pball->x;							// ball position
	const float bally = pball->y;

	// ball normal distance: contact distance normal to segment. lateral contact subtract the ball radius 

	const float rollingRadius = lateral ? pball->radius : C_TOL_RADIUS;	  //lateral or rolling point
	const float bcpd = (ballx - v1.x)*normal.x + (bally - v1.y)*normal.y; // ball center to plane distance
	const float bnd = bcpd - rollingRadius;

	const bool inside = (bnd <= 0.f);									  // in ball inside object volume
	
	//HitTestBasic
	float hittime;
	if (rigid)
		{
		if (bnv > C_LOWNORMVEL || bnd < (float)(-PHYS_SKIN) || (lateral && bcpd < 0)) return -1.0f;	// (ball normal distance) excessive pentratration of object skin ... no collision HACK
			
		if (lateral && (bnd <= (float)PHYS_TOUCH))
			{
			if (inside || (fabsf(bnv) > C_CONTACTVEL)				// fast velocity, return zero time
																	//zero time for rigid fast bodies				
			|| (bnd <= (float)(-PHYS_TOUCH)))
				hittime = 0;										// slow moving but embedded
			else
				hittime = bnd*(float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f;	// don't compete for fast zero time events
            }
		else if (fabsf(bnv) > C_LOWNORMVEL) 					// not velocity low ????
			hittime = bnd/(-bnv);								// rate ok for safe divide 
		else return -1.0f;										// wait for touching
		}
	else //non-rigid ... target hits
		{
		if (bnv * bnd >= 0)										// outside-receding || inside-approaching
			{
			if ((m_ObjType != eTrigger) ||						// no a trigger
			    (!pball->m_vpVolObjs) ||
			    (fabsf(bnd) >= (float)(PHYS_SKIN/2.0)) ||		// not to close ... nor to far away
			    (inside != (pball->m_vpVolObjs->IndexOf(m_pObj) < 0))) // ...ball outside and hit set or  ball inside and no hit set
				return -1.0f;
			
			hittime = 0;
			bUnHit = !inside;	// ball on outside is UnHit, otherwise it's a Hit
			}
		else
			hittime = bnd/(-bnv);	
		}

	if (infNaN(hittime) || hittime < 0 || hittime > dtime) return -1.0f; // time is outside this frame ... no collision

	const float btv = ballvx*normal.y - ballvy*normal.x;				 //ball velocity tangent to segment with respect to direction from V1 to V2
	const float btd = (ballx - v1.x)*normal.y - (bally - v1.y)*normal.x  // ball tangent distance 
					+ btv * hittime;								     // ball tangent distance (projection) (initial position + velocity * hitime)

	if (btd < -C_TOL_ENDPNTS || btd > length + C_TOL_ENDPNTS) // is the contact off the line segment??? 
		return -1.0f;

	if (!rigid)												  // non rigid body collision? return direction
		phitnormal[1].x = bUnHit ? 1.0f : 0.0f;				  // UnHit signal is receding from outside target
	
	const float ballr = pball->radius;
	const float hitz = pball->z - ballr + pball->vz*hittime;  // check too high or low relative to ball rolling point at hittime

	if (hitz + ballr * 1.5f < m_rcHitRect.zlow				  // check limits of object's height and depth  
		|| hitz + ballr * 0.5f > m_rcHitRect.zhigh)
		return -1.0f;

	phitnormal->x = normal.x;				// hit normal is same as line segment normal
	phitnormal->y = normal.y;
		
	pball->m_HitDist = bnd;					// actual contact distance ... 
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = rigid;				// collision type

	return hittime;
	}	

float LineSeg::HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal) 
	{															// normal face, lateral, rigid
	return HitTestBasic(pball, dtime, phitnormal, true, true, true);
	}

float HitCircle::HitTestBasicRadius(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal,
									const bool direction, const bool lateral, const bool rigid)
	{
	if (!m_fEnabled || pball->fFrozen) return -1.0f;	

	const float x = center.x;
	const float y = center.y;	
	
	const float dx = pball->x - x;	// form delta components (i.e. translate coordinate frame)
	const float dy = pball->y - y;

	const float dvx = pball->vx;	// delta velocity from ball's coordinate frame
	const float dvy = pball->vy;

	float targetRadius,z,dz,dvz;
	bool capsule3D;
	
	if (!lateral && pball->z > zhigh)
		{
		capsule3D = true;										// handle ball over target? 
		//const float hcap = radius*(float)(1.0/5.0);			// cap height to hit-circle radius ratio
		//targetRadius = radius*radius/(hcap*2.0f) + hcap*0.5f;	// c = (r^2+h^2)/(2*h)
		targetRadius = radius*(float)(13.0/5.0);				// optimized version of above code
		//z = zhigh - (targetRadius - hcap);					// b = c - h
		z = zhigh - radius*(float)(12.0/5.0);					// optimized version of above code
		dz = pball->z - z;										// ball rolling point - capsule center height 			
		dvz = pball->vz;										// differential velocity
		}
	else
		{
		capsule3D = false;
		targetRadius = radius;
		if(lateral)
			targetRadius += pball->radius;
		z = dz = dvz = 0.0f;
		}
	
	const float bcddsq = dx*dx + dy*dy + dz*dz;	// ball center to circle center distance ... squared

	const float bcdd = sqrtf(bcddsq);			//distance center to center
	if (bcdd <= 1.0e-6f) return -1.0f;			// no hit on exact center

	float b = dx*dvx + dy*dvy + dz*dvz;			//inner product

	const float bnv = b/bcdd;					//ball normal velocity

	if (direction && bnv > C_LOWNORMVEL) return -1.0f; // clearly receding from radius

	const float bnd = bcdd - targetRadius;		// ball normal distance to 

	const float a = dvx*dvx + dvy*dvy +dvz*dvz;	// square of the delta velocity (outer product)

	float hittime = 0;
	bool fUnhit = false;
	// Kicker is special.. handle ball stalled on kicker, commonly hit while receding, knocking back into kicker pocket
	if (m_ObjType == eKicker && bnd <= 0 && bnd >= -radius && a < C_CONTACTVEL*C_CONTACTVEL )	
		{
		if (pball->m_vpVolObjs) pball->m_vpVolObjs->RemoveElement(m_pObj);	// cause capture
		}

	if (rigid && bnd < (float)PHYS_TOUCH)		// positive: contact possible in future ... Negative: objects in contact now
		{
		if (bnd < (float)(-PHYS_SKIN)) return -1.0f;	

		if ((fabsf(bnv) > C_CONTACTVEL)			// >fast velocity, return zero time
												//zero time for rigid fast bodies
		|| (bnd <= (float)(-PHYS_TOUCH)))
			hittime = 0;						// slow moving but embedded
		else
			hittime = bnd*(float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f;	// don't compete for fast zero time events
		}
	else if (m_ObjType >= eTrigger // triggers & kickers
		     && pball->m_vpVolObjs && (bnd < 0 == pball->m_vpVolObjs->IndexOf(m_pObj) < 0))
		{ // here if ... ball inside and no hit set .... or ... ball outside and hit set

		if (fabsf(bnd-radius) < 0.05f)	 // if ball appears in center of trigger, then assumed it was gen'ed there
			{
			if (pball->m_vpVolObjs) pball->m_vpVolObjs->AddElement(m_pObj);	//special case for trigger overlaying a kicker
			}										// this will add the ball to the trigger space without a Hit
		else
			{
			hittime = 0;
			fUnhit = (bnd > 0);	// ball on outside is UnHit, otherwise it's a Hit
			}
		}
	else
		{
		if((!rigid && bnd * bnv > 0) ||	// (outside and receding) or (inside and approaching)
		   (a < 1.0e-8f)) return -1.0f;	//no hit ... ball not moving relative to object

		const float c = bcddsq - targetRadius*targetRadius; // contact distance ... square delta distance (outer product)

		b += b;									// twice the (inner products)

		float result = b*b - 4.0f*a*c;			// inner products minus the outer products

		if (result < 0) return -1.0f;			// contact impossible 
			
		result = sqrtf(result);
		
		const float inv_a = (-0.5f)/a;
		const float time1 = (b - result)* inv_a;
		const float time2 = (b + result)* inv_a;
		
		fUnhit = (time1*time2 < 0);
		hittime = fUnhit ? max(time1,time2) : min(time1,time2); // ball is inside the circle

		if (infNaN(hittime) || hittime < 0 || hittime > dtime) return -1.0f; // contact out of physics frame
		}
	
	const float hitz = pball->z - pball->radius + pball->vz * hittime; //rolling point

	if(((hitz + pball->radius *1.5f) < zlow) ||
	   (!capsule3D && (hitz + pball->radius*0.5f) > zhigh) ||
	   (capsule3D && (pball->z + pball->vz * hittime) < zhigh)) return -1.0f;
		
	const float hitx = pball->x + pball->vx*hittime;
	const float hity = pball->y + pball->vy*hittime;

	const float sqrlen = (hitx - x)*(hitx - x)+(hity - y)*(hity - y);

	 if (sqrlen > 1.0e-8f) // over center???
		{//no
		const float inv_len = 1.0f/sqrtf(sqrlen);
		phitnormal->x = (hitx - x)*inv_len;
		phitnormal->y = (hity - y)*inv_len;
		}
	 else 
		{//yes over center
		phitnormal->x = 0; // make up a value, any direction is ok
		phitnormal->y = 1.0f;
		}
	
	if (!rigid)											// non rigid body collision? return direction
		phitnormal[1].x = fUnhit ? 1.0f : 0.0f;			// UnHit signal	is receding from target

	pball->m_HitDist = bnd;					//actual contact distance ... 
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = rigid;				// collision type

	return hittime;
	}

float HitCircle::HitTestRadius(Ball *pball, float dtime, Vertex3Ds *phitnormal)
	{	
													//normal face, lateral, rigid
	return HitTestBasicRadius(pball, dtime, phitnormal, true, true, true);		
	}	

void LineSeg::Collide(Ball * const pball, Vertex3Ds * const phitnormal)
	{
	const float dot = phitnormal->x * pball->vx + phitnormal->y * pball->vy;

	pball->CollideWall(phitnormal, m_elasticity, m_antifriction, m_scatter);

	if (m_pfe)
		{			
		if (dot <= -m_threshold)
			{
			const float dx = pball->m_Event_Pos.x - pball->x; // is this the same place as last event????
			const float dy = pball->m_Event_Pos.y - pball->y; // if same then ignore it
			const float dz = pball->m_Event_Pos.z - pball->z;

			if (dx*dx + dy*dy + dz*dz > 0.25f)// must be a new place if only by a little
				{
				m_pfe->FireGroupEvent(DISPID_HitEvents_Hit);
				}
			}
		
		pball->m_Event_Pos.x = pball->x; 
		pball->m_Event_Pos.y = pball->y; 
		pball->m_Event_Pos.z = pball->z; //remember last collide position
		}
	}

void LineSeg::CalcNormal()
	{
	const Vertex2D vT(v1.x - v2.x, v1.y - v2.y);

	// Set up line normal
	length = sqrtf(vT.x*vT.x + vT.y*vT.y);
	const float inv_length = 1.0f/length;
	normal.x =  vT.y * inv_length;
	normal.y = -vT.x * inv_length;
	}

Joint::Joint()
	{
	radius = 0.0f;
	}

void Joint::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left = center.x;
	m_rcHitRect.right = center.x;
	m_rcHitRect.top = center.y;
	m_rcHitRect.bottom = center.y;
	
	zlow = m_rcHitRect.zlow; //!!?
	zhigh = m_rcHitRect.zhigh;
	}

float Joint::HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal)
	{
	if (!m_fEnabled)
		return -1.0f;

	return HitTestRadius(pball, dtime, phitnormal);	
	}

void Joint::Collide(Ball * const pball, Vertex3Ds * const phitnormal)
	{
	const float dot = phitnormal->x * pball->vx + phitnormal->y * pball->vy;

	pball->CollideWall(phitnormal, m_elasticity, m_antifriction, m_scatter);

	if (m_pfe)
		{			
		if (dot <= -m_threshold)
			{
			const float dx = pball->m_Event_Pos.x - pball->x; // is this the same place as last event????
			const float dy = pball->m_Event_Pos.y - pball->y; // if same then ignore it
			const float dz = pball->m_Event_Pos.z - pball->z;

			if (dx*dx + dy*dy + dz*dz > 0.25f) // must be a new place if only by a little
				{
				m_pfe->FireGroupEvent(DISPID_HitEvents_Hit);
				}
			}
		
		pball->m_Event_Pos.x = pball->x; 
		pball->m_Event_Pos.y = pball->y; 
		pball->m_Event_Pos.z = pball->z; //remember last collide position
		}
	}

void HitCircle::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left = center.x - radius;
	m_rcHitRect.right = center.x + radius;
	m_rcHitRect.top = center.y - radius;
	m_rcHitRect.bottom = center.y + radius;
	m_rcHitRect.zlow = zlow;
	m_rcHitRect.zhigh = zhigh;
	}

float HitCircle::HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal)
	{
	return HitTestRadius(pball, dtime, phitnormal);
	}

void HitCircle::Collide(Ball * const pball, Vertex3Ds * const phitnormal)
	{
	pball->CollideWall(phitnormal, m_elasticity, m_antifriction, m_scatter);
	}


// Perform the actual hittest between ball and hit object and update
// collision information if a hit occurred.
static void DoHitTest(Ball *pball, HitObject *pho)
{
    const float newtime = pho->HitTest(pball, pball->m_hittime, pball->m_hitnormal); // test for hit
    if ((newtime >= 0) && (newtime <= pball->m_hittime))
    {
        pball->m_pho = pho;
        pball->m_hittime = newtime;
        pball->m_hitx = pball->x + pball->vx*newtime;
        pball->m_hity = pball->y + pball->vy*newtime;
    }
}



HitKD::~HitKD()
{
	if(tmp != 0)
	    free(tmp);

	if(m_org_idx != 0)
	    free(m_org_idx);

#ifdef SSE_LEAFTEST
	if(l_r_t_b_zl_zh != 0)
	    _aligned_free(l_r_t_b_zl_zh);
#endif

	if(m_nodes != 0)
		free(m_nodes);
}

HitKDNode::~HitKDNode()
{
#ifdef HITLOG
	if (g_fWriteHitDeleteLog)
		{
		FILE * const file = fopen("c:\\log.txt", "a");
		fprintf(file,"Deleting %f %f %f %f %u\n", m_rectbounds.left, m_rectbounds.top, m_rectbounds.right, m_rectbounds.bottom, m_children); 
		fclose(file);
		}
#endif
}

void HitKDNode::CreateNextLevel()
{
	const unsigned int org_items = (m_items&0x3FFFFFFF);

	if(org_items <= 4) //!! magic (will not favor empty space enough for huge objects)
		return;

	const Vertex3Ds vdiag(m_rectbounds.right-m_rectbounds.left, m_rectbounds.bottom-m_rectbounds.top, m_rectbounds.zhigh-m_rectbounds.zlow);

	unsigned int axis;
	if((vdiag.x > vdiag.y) && (vdiag.x > vdiag.z))
	{
		if(vdiag.x < 6.66f) //!! magic (will not subdivide object soups enough)
			return;
		axis = 0;
	}
	else if(vdiag.y > vdiag.z)
	{
		if(vdiag.y < 6.66f)
			return;
		axis = 1;
	}
	else
	{
		if(vdiag.z < 6.66f)
			return;
		axis = 2;
	}

#ifdef _DEBUGPHYSICS
	g_pplayer->c_octNextlevels++;
#endif

	// create children, calc bboxes
	m_children = (HitKDNode*)(m_hitoct->m_nodes) + m_hitoct->m_num_nodes;
	m_hitoct->m_num_nodes += 2;
	m_children[0].m_rectbounds = m_rectbounds;
	m_children[1].m_rectbounds = m_rectbounds;

	const Vertex3Ds vcenter((m_rectbounds.left+m_rectbounds.right)*0.5f, (m_rectbounds.top+m_rectbounds.bottom)*0.5f, (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f);
	if(axis == 0)
	{
		m_children[0].m_rectbounds.left = m_rectbounds.left;
		m_children[0].m_rectbounds.right = vcenter.x;
		m_children[1].m_rectbounds.left = vcenter.x;
		m_children[1].m_rectbounds.right = m_rectbounds.right;
	}
	else if (axis == 1)
	{
		m_children[0].m_rectbounds.top = m_rectbounds.top;
		m_children[0].m_rectbounds.bottom = vcenter.y;
		m_children[1].m_rectbounds.top = vcenter.y;
		m_children[1].m_rectbounds.bottom = m_rectbounds.bottom;
	}
	else
	{
		m_children[0].m_rectbounds.zlow = m_rectbounds.zlow;
		m_children[0].m_rectbounds.zhigh = vcenter.z;
		m_children[1].m_rectbounds.zlow = vcenter.z;
		m_children[1].m_rectbounds.zhigh = m_rectbounds.zhigh;
	}

	m_children[0].m_hitoct = m_hitoct; //!! meh
	m_children[0].m_items = 0;
	m_children[0].m_children = NULL;
	m_children[1].m_hitoct = m_hitoct; //!! meh
	m_children[1].m_items = 0;
	m_children[1].m_children = NULL;

	// determine amount of items that cross splitplane, or are passed on to the children
	if(axis == 0)
	{
	for(unsigned int i = m_start; i < m_start+org_items; ++i)
	{
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);

		if (pho->m_rcHitRect.right < vcenter.x)
			m_children[0].m_items++;
		else if (pho->m_rcHitRect.left > vcenter.x)
			m_children[1].m_items++;
	}
	} 
	else if (axis==1)
	{
	for(unsigned int i = m_start; i < m_start+org_items; ++i)
	{
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);

		if (pho->m_rcHitRect.bottom < vcenter.y)
			m_children[0].m_items++;
		else if (pho->m_rcHitRect.top > vcenter.y)
			m_children[1].m_items++;
	}
	}
	else
	for(unsigned int i = m_start; i < m_start+org_items; ++i)
	{
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);

		if (pho->m_rcHitRect.zhigh < vcenter.z)
			m_children[0].m_items++;
		else if (pho->m_rcHitRect.zlow > vcenter.z)
			m_children[1].m_items++;
	}

	m_children[0].m_start = m_start + org_items-m_children[0].m_items-m_children[1].m_items;
	m_children[1].m_start = m_children[0].m_start + m_children[0].m_items;

	unsigned int items = 0;
    m_children[0].m_items = 0;
    m_children[1].m_items = 0;

	// sort items that cross splitplane in-place, the others are sorted into a temporary
	if(axis == 0)
	{
	for(unsigned int i = m_start; i < m_start+org_items; ++i)
	{
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);

		if (pho->m_rcHitRect.right < vcenter.x) {
			m_hitoct->tmp[m_children[0].m_start+m_children[0].m_items] = m_hitoct->m_org_idx[i];
			m_children[0].m_items++;
		}
		else if (pho->m_rcHitRect.left > vcenter.x) {
			m_hitoct->tmp[m_children[1].m_start+m_children[1].m_items] = m_hitoct->m_org_idx[i];
			m_children[1].m_items++;
		} else {
			m_hitoct->m_org_idx[m_start+items] = m_hitoct->m_org_idx[i];
			items++;
		}			
	}
	} 
	else if (axis==1)
	{
	for(unsigned int i = m_start; i < m_start+org_items; ++i)
	{
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);

		if (pho->m_rcHitRect.bottom < vcenter.y) {
			m_hitoct->tmp[m_children[0].m_start+m_children[0].m_items] = m_hitoct->m_org_idx[i];
			m_children[0].m_items++;
		}
		else if (pho->m_rcHitRect.top > vcenter.y) {
			m_hitoct->tmp[m_children[1].m_start+m_children[1].m_items] = m_hitoct->m_org_idx[i];
			m_children[1].m_items++;
		} else {
			m_hitoct->m_org_idx[m_start+items] = m_hitoct->m_org_idx[i];
			items++;
		}			
	}
	}
	else
	for(unsigned int i = m_start; i < m_start+org_items; ++i)
	{
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);

		if (pho->m_rcHitRect.zhigh < vcenter.z) {
			m_hitoct->tmp[m_children[0].m_start+m_children[0].m_items] = m_hitoct->m_org_idx[i];
			m_children[0].m_items++;
		}
		else if (pho->m_rcHitRect.zlow > vcenter.z) {
			m_hitoct->tmp[m_children[1].m_start+m_children[1].m_items] = m_hitoct->m_org_idx[i];
			m_children[1].m_items++;
		} else {
			m_hitoct->m_org_idx[m_start+items] = m_hitoct->m_org_idx[i];
			items++;
		}			
	}
	
	m_items = items|(axis<<30);

	// copy temporary back //!! could omit this by doing everything inplace
	memcpy(m_hitoct->m_org_idx+m_children[0].m_start, m_hitoct->tmp+m_children[0].m_start, m_children[0].m_items*4);
    memcpy(m_hitoct->m_org_idx+m_children[1].m_start, m_hitoct->tmp+m_children[1].m_start, m_children[1].m_items*4);

	m_children[0].CreateNextLevel();
	m_children[1].CreateNextLevel();
}

// build SSE boundary arrays of the local hit-object/m_vho HitRect list, generated for -full- list completely in the end!
void HitKD::InitSseArrays()
{
	if(!m_dynamic)
	{
		free(tmp);
		tmp = 0;
	}

#ifdef SSE_LEAFTEST
	const unsigned int padded = (m_num_items+3)&0xFFFFFFFC;
	if(!l_r_t_b_zl_zh)
		l_r_t_b_zl_zh = (float*)_aligned_malloc(sizeof(float) * padded * 6, 16);

    for(unsigned int j = 0; j < m_num_items; ++j)
    {
		const FRect3D r = m_org_vho->ElementAt(m_org_idx[j])->m_rcHitRect;
		l_r_t_b_zl_zh[j] = r.left;
		l_r_t_b_zl_zh[j+padded] = r.right;
		l_r_t_b_zl_zh[j+padded*2] = r.top;
		l_r_t_b_zl_zh[j+padded*3] = r.bottom;
		l_r_t_b_zl_zh[j+padded*4] = r.zlow;
		l_r_t_b_zl_zh[j+padded*5] = r.zhigh;
    }

	for(unsigned int j = m_num_items; j < padded; ++j)
	{
		l_r_t_b_zl_zh[j] = FLT_MAX;
		l_r_t_b_zl_zh[j+padded] = -FLT_MAX;
		l_r_t_b_zl_zh[j+padded*2] = FLT_MAX;
		l_r_t_b_zl_zh[j+padded*3] = -FLT_MAX;
		l_r_t_b_zl_zh[j+padded*4] = FLT_MAX;
		l_r_t_b_zl_zh[j+padded*5] = -FLT_MAX;
	}
#endif
}

/*  RLC

Hit logic needs to be expanded, during static and psudo-static conditions, multiple hits (multi-face contacts)
are possible and should be handled, with embedding (pentrations) some contacts persist for long periods
and may cause others not to be seen (masked because of their position in the object list).

A short term solution might be to rotate the object list on each collision round. Currently, its a linear array.
and some subscript magic might be needed, where the actually collision counts are used to cycle the starting position
for the next search. This could become a Ball property ... i.e my last hit object index, start at the next
and cycle around until the last hit object is the last to be tested ... this could be made complex due to 
scripts removing objects .... i.e. balls ... better study well before I start

The most effective would be to sort the search results, always moving the last hit to the end of it's grouping

At this instance, I'm reporting static contacts as random hitimes during the specific physics frame; the zero time
slot is not in the random time generator algorithm, it is offset by STATICTIME so not to compete with the fast moving
collisions

*/

void HitKDNode::HitTestBall(Ball * const pball) const
{
	const unsigned int org_items = (m_items&0x3FFFFFFF);
	const unsigned int axis = (m_items>>30);

	for (int i=m_start; i<m_start+org_items; i++)
	{
#ifdef _DEBUGPHYSICS
		g_pplayer->c_tested++;
#endif
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);
		if ((pball != pho) // ball can not hit itself
		       && fRectIntersect3D(pball->m_rcHitRect, pho->m_rcHitRect))
		{
#ifdef _DEBUGPHYSICS
			g_pplayer->c_deepTested++;
#endif
            DoHitTest(pball, pho);
		}
	}

	if (m_children) // not a leaf
	{
#ifdef _DEBUGPHYSICS
		g_pplayer->c_traversed++;
#endif
		if(axis == 0)
		{
			const float vcenter = (m_rectbounds.left+m_rectbounds.right)*0.5f;
			if(pball->m_rcHitRect.left <= vcenter)
				m_children[0].HitTestBallSse(pball);
			if(pball->m_rcHitRect.right >= vcenter)
				m_children[1].HitTestBallSse(pball);
		}
		else
		if(axis == 1)
		{
			const float vcenter = (m_rectbounds.top+m_rectbounds.bottom)*0.5f;
			if (pball->m_rcHitRect.top <= vcenter)
				m_children[0].HitTestBallSse(pball);
			if (pball->m_rcHitRect.bottom >= vcenter)
				m_children[1].HitTestBallSse(pball);
		}
		else
		{
			const float vcenter = (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f;

			if(pball->m_rcHitRect.zlow <= vcenter)
				m_children[0].HitTestBallSse(pball);
			if(pball->m_rcHitRect.zhigh >= vcenter)
				m_children[1].HitTestBallSse(pball);
		}
	}
}

#ifdef SSE_LEAFTEST
void HitKDNode::HitTestBallSseInner(Ball * const pball, const int i) const
{
  HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);

  // ball can not hit itself
  if (pball == pho)
    return;

#ifdef _DEBUGPHYSICS
  g_pplayer->c_deepTested++;
#endif
  DoHitTest(pball, pho);
}

void HitKDNode::HitTestBallSse(Ball * const pball) const
{
	const unsigned int org_items = (m_items&0x3FFFFFFF);
	const unsigned int axis = (m_items>>30);

    const unsigned int padded = (m_hitoct->m_num_items+3)&0xFFFFFFFC;

    // init SSE registers with ball bbox
    const __m128 bleft = _mm_set1_ps(pball->m_rcHitRect.left);
    const __m128 bright = _mm_set1_ps(pball->m_rcHitRect.right);
    const __m128 btop = _mm_set1_ps(pball->m_rcHitRect.top);
    const __m128 bbottom = _mm_set1_ps(pball->m_rcHitRect.bottom);
    const __m128 bzlow = _mm_set1_ps(pball->m_rcHitRect.zlow);
    const __m128 bzhigh = _mm_set1_ps(pball->m_rcHitRect.zhigh);

    const __m128* __restrict const pL = (__m128*)m_hitoct->l_r_t_b_zl_zh;
    const __m128* __restrict const pR = (__m128*)(m_hitoct->l_r_t_b_zl_zh+padded);
    const __m128* __restrict const pT = (__m128*)(m_hitoct->l_r_t_b_zl_zh+padded*2);
    const __m128* __restrict const pB = (__m128*)(m_hitoct->l_r_t_b_zl_zh+padded*3);
    const __m128* __restrict const pZl = (__m128*)(m_hitoct->l_r_t_b_zl_zh+padded*4);
    const __m128* __restrict const pZh = (__m128*)(m_hitoct->l_r_t_b_zl_zh+padded*5);

    // loop implements 4 collision checks at once
    // (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow)
    const unsigned int size = (m_start+org_items+3)/4;
    for (unsigned int i = m_start/4; i < size; ++i)
    {
#ifdef _DEBUGPHYSICS
	  g_pplayer->c_tested++; //!! +=4? or is this more fair?
#endif
      // comparisons set bits if bounds miss. if all bits are set, there is no collision. otherwise continue comparisons
      // bits set, there is a bounding box collision
      __m128 cmp = _mm_cmpge_ps(bright, pL[i]);
      int mask = _mm_movemask_ps(cmp);
      if (mask == 0) continue;

	  cmp = _mm_cmple_ps(bleft, pR[i]);
      mask &= _mm_movemask_ps(cmp);
      if (mask == 0) continue;

      cmp = _mm_cmpge_ps(bbottom, pT[i]);
      mask &= _mm_movemask_ps(cmp);
      if (mask == 0) continue;

      cmp = _mm_cmple_ps(btop, pB[i]);
      mask &= _mm_movemask_ps(cmp);
      if (mask == 0) continue;

      cmp = _mm_cmpge_ps(bzhigh, pZl[i]);
      mask &= _mm_movemask_ps(cmp);
      if (mask == 0) continue;

	  cmp = _mm_cmple_ps(bzlow, pZh[i]);
      mask &= _mm_movemask_ps(cmp);
      if (mask == 0) continue;

      // now there is at least one bbox collision
      if ((mask & 1) != 0) HitTestBallSseInner(pball, i*4);
      if ((mask & 2) != 0 && (i*4+1)<m_hitoct->m_num_items) HitTestBallSseInner(pball, i*4+1); // boundary checks not necessary (in theory, //!! strange bug with TOM mirror mod, cmp shows correct value, mask although not)
      if ((mask & 4) != 0 && (i*4+2)<m_hitoct->m_num_items) HitTestBallSseInner(pball, i*4+2); //  anymore as non-valid entries were
      if ((mask & 8) != 0 && (i*4+3)<m_hitoct->m_num_items) HitTestBallSseInner(pball, i*4+3); //  initialized to keep these maskbits 0
    }

	if (m_children) // not a leaf
	{
#ifdef _DEBUGPHYSICS
		g_pplayer->c_traversed++;
#endif
		if(axis == 0)
		{
			const float vcenter = (m_rectbounds.left+m_rectbounds.right)*0.5f;
			if(pball->m_rcHitRect.left <= vcenter)
				m_children[0].HitTestBallSse(pball);
			if(pball->m_rcHitRect.right >= vcenter)
				m_children[1].HitTestBallSse(pball);
		}
		else
		if(axis == 1)
		{
			const float vcenter = (m_rectbounds.top+m_rectbounds.bottom)*0.5f;
			if (pball->m_rcHitRect.top <= vcenter)
				m_children[0].HitTestBallSse(pball);
			if (pball->m_rcHitRect.bottom >= vcenter)
				m_children[1].HitTestBallSse(pball);
		}
		else
		{
			const float vcenter = (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f;

			if(pball->m_rcHitRect.zlow <= vcenter)
				m_children[0].HitTestBallSse(pball);
			if(pball->m_rcHitRect.zhigh >= vcenter)
				m_children[1].HitTestBallSse(pball);
		}
	}
}
#endif

void HitKDNode::HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit) const
{
	const unsigned int org_items = (m_items&0x3FFFFFFF);
	const unsigned int axis = (m_items>>30);

	for (int i=m_start; i<m_start+org_items; i++)
	{
#ifdef _DEBUGPHYSICS
		g_pplayer->c_tested++;
#endif
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);
		if ((pball != pho) && // ball cannot hit itself
			fRectIntersect3D(pball->m_rcHitRect,pho->m_rcHitRect))
		{
#ifdef _DEBUGPHYSICS
			g_pplayer->c_deepTested++;
#endif
			const float newtime = pho->HitTest(pball, pball->m_hittime, pball->m_hitnormal);
			if (newtime >= 0)
				pvhoHit->AddElement(pho);
		}
	}

	if (m_children) // not a leaf
	{
#ifdef _DEBUGPHYSICS
		g_pplayer->c_traversed++;
#endif
		if(axis == 0)
		{
			const float vcenter = (m_rectbounds.left+m_rectbounds.right)*0.5f;
			if(pball->m_rcHitRect.left <= vcenter)
				m_children[0].HitTestXRay(pball,pvhoHit);
			if(pball->m_rcHitRect.right >= vcenter)
				m_children[1].HitTestXRay(pball,pvhoHit);
		}
		else
		if(axis == 1)
		{
			const float vcenter = (m_rectbounds.top+m_rectbounds.bottom)*0.5f;
			if (pball->m_rcHitRect.top <= vcenter)
				m_children[0].HitTestXRay(pball,pvhoHit);
			if (pball->m_rcHitRect.bottom >= vcenter)
				m_children[1].HitTestXRay(pball,pvhoHit);
		}
		else
		{
			const float vcenter = (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f;

			if(pball->m_rcHitRect.zlow <= vcenter)
				m_children[0].HitTestXRay(pball,pvhoHit);
			if(pball->m_rcHitRect.zhigh >= vcenter)
				m_children[1].HitTestXRay(pball,pvhoHit);
		}
	}
}
