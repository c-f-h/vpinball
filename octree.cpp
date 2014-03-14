#include "stdafx.h"
#include "octree.h"

HitOctree::~HitOctree()
{
	if(tmp != 0)
	    free(tmp);

	if(m_org_idx != 0)
	    free(m_org_idx);

	if(l_r_t_b_zl_zh != 0)
	    _aligned_free(l_r_t_b_zl_zh);
}

HitOctreeNode::~HitOctreeNode()
{
	if (m_children)
		delete [] m_children;
}

#ifdef _DEBUGPHYSICS
U64 oct_nextlevels = 0;
#endif

void HitOctreeNode::CreateNextLevel()
{
	const Vertex3Ds vcenter((m_rectbounds.left+m_rectbounds.right)*0.5f, (m_rectbounds.top+m_rectbounds.bottom)*0.5f, (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f);

	m_children = new HitOctreeNode[8]; //!! global list instead?!
	for(unsigned int i = 0; i < 8; ++i)
	{
		m_children[i].m_rectbounds.left = (i&1) ? vcenter.x : m_rectbounds.left;
		m_children[i].m_rectbounds.top  = (i&2) ? vcenter.y : m_rectbounds.top;
		m_children[i].m_rectbounds.zlow = (i&4) ? vcenter.z : m_rectbounds.zlow;

		m_children[i].m_rectbounds.right  = (i&1) ? m_rectbounds.right  : vcenter.x;
		m_children[i].m_rectbounds.bottom = (i&2) ? m_rectbounds.bottom : vcenter.y;
		m_children[i].m_rectbounds.zhigh  = (i&4) ? m_rectbounds.zhigh  : vcenter.z;

		m_children[i].m_hitoct = m_hitoct; //!! meh
		m_children[i].m_items = 0;
	}

	unsigned int items = 0;

	for(unsigned int i = m_start; i < m_start+m_items; ++i)
	{
		int oct;
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);

		if (pho->m_rcHitRect.right < vcenter.x)
			oct = 0;
		else if (pho->m_rcHitRect.left > vcenter.x)
			oct = 1;
		else
			oct = 128;

		if (pho->m_rcHitRect.bottom < vcenter.y)
		{
			//oct |= 0;
		}
		else if (pho->m_rcHitRect.top > vcenter.y)
			oct |= 2;
		else
			oct |= 128;

		if ((oct & 128) == 0)
			m_children[oct].m_items++;
		else
			items++;
	}

	m_children[0].m_start = m_start+items;
	for(unsigned int i = 1; i < 8; ++i)
	    m_children[i].m_start = m_children[i-1].m_start + m_children[i-1].m_items;

	items = 0;
	for(unsigned int i = 0; i < 8; ++i)
	    m_children[i].m_items = 0;

	for(unsigned int i = m_start; i < m_start+m_items; ++i)
	{
		int oct;
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);

		if (pho->m_rcHitRect.right < vcenter.x)
			oct = 0;
		else if (pho->m_rcHitRect.left > vcenter.x)
			oct = 1;
		else
			oct = 128;

		if (pho->m_rcHitRect.bottom < vcenter.y)
		{
			//oct |= 0;
		}
		else if (pho->m_rcHitRect.top > vcenter.y)
			oct |= 2;
		else
			oct |= 128;

		if ((oct & 128) == 0) {
			m_hitoct->tmp[m_children[oct].m_start+m_children[oct].m_items] = m_hitoct->m_org_idx[i];
			m_children[oct].m_items++;
		} else {
			m_hitoct->tmp[m_start+items] = m_hitoct->m_org_idx[i];
			items++;
		}
	}

	m_items = items;

	//!! doublebuffer instead of copying around again?
	memcpy(m_hitoct->m_org_idx+m_start, m_hitoct->tmp+m_start, m_items*4);
	for(unsigned int i = 0; i < 8; ++i)
	    memcpy(m_hitoct->m_org_idx+m_children[i].m_start, m_hitoct->tmp+m_children[i].m_start, m_children[i].m_items*4);

	for(unsigned int i = 0; i < 8; ++i)
	{
		if(((i&1) && (vcenter.x - m_rectbounds.left > 66.6f)) ||        // BUG: will never subdivide lower left quadrant
		    ((i&2) && (vcenter.y - m_rectbounds.top > 66.6f)) ||
		    ((i&4) && (vcenter.z - m_rectbounds.zlow > 66.6f))) //!! magic (will not subdivide object soups enough)
			if(m_children[i].m_items > 1) { //!! magic (will not favor empty space enough for huge objects)
				m_children[i].CreateNextLevel();
#ifdef _DEBUGPHYSICS
				oct_nextlevels++;
#endif
			}
	}
}

// build SSE boundary arrays of the local hit-object/m_vho HitRect list, generated for -full- list completely in the end!
void HitOctree::InitSseArrays() //!! get rid of SSE madness completely nowadays?! -> rather reduce number of elements per node/leaf
{
	free(tmp);
	tmp = 0;

	const unsigned int padded = (m_all_items+3)&0xFFFFFFFC;
	l_r_t_b_zl_zh = (float*)_aligned_malloc(sizeof(float) * padded * 6, 16);

    for(unsigned int j = 0; j < m_all_items; ++j)
    {
		const FRect3D r = m_org_vho->ElementAt(m_org_idx[j])->m_rcHitRect;
		l_r_t_b_zl_zh[j] = r.left;
		l_r_t_b_zl_zh[j+padded] = r.right;
		l_r_t_b_zl_zh[j+padded*2] = r.top;
		l_r_t_b_zl_zh[j+padded*3] = r.bottom;
		l_r_t_b_zl_zh[j+padded*4] = r.zlow;
		l_r_t_b_zl_zh[j+padded*5] = r.zhigh;
    }

	for(unsigned int j = m_all_items; j < padded; ++j)
	{
		l_r_t_b_zl_zh[j] = FLT_MAX;
		l_r_t_b_zl_zh[j+padded] = -FLT_MAX;
		l_r_t_b_zl_zh[j+padded*2] = FLT_MAX;
		l_r_t_b_zl_zh[j+padded*3] = -FLT_MAX;
		l_r_t_b_zl_zh[j+padded*4] = FLT_MAX;
		l_r_t_b_zl_zh[j+padded*5] = -FLT_MAX;
	}
}

#ifdef LOG
extern int cTested;
extern int cDeepTested;
#endif


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

void HitOctreeNode::HitTestBall(Ball * const pball) const
{
	for (unsigned i=m_start; i<m_start+m_items; i++)
	{
#ifdef LOG
		cTested++;
#endif
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);
		if ((pball != pho) // ball can not hit itself
		       && fRectIntersect3D(pball->m_rcHitRect, pho->m_rcHitRect))
		{
            DoHitTest(pball, pho);
		}
	}

	if (m_children) // not a leaf
	{
		const Vertex3Ds vcenter((m_rectbounds.left+m_rectbounds.right)*0.5f, (m_rectbounds.top+m_rectbounds.bottom)*0.5f, (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f);

		const bool fLeft = (pball->m_rcHitRect.left <= vcenter.x);
		const bool fRight = (pball->m_rcHitRect.right >= vcenter.x);

#ifdef LOG
		cTested++;
#endif
		if (pball->m_rcHitRect.top <= vcenter.y) // Top
		{
			if (fLeft)
				m_children[0].HitTestBallSse(pball);
			if (fRight)
				m_children[1].HitTestBallSse(pball);
		}
		if (pball->m_rcHitRect.bottom >= vcenter.y) // Bottom
		{
			if (fLeft)
				m_children[2].HitTestBallSse(pball);
			if (fRight)
				m_children[3].HitTestBallSse(pball);
		}
	}
}


void HitOctreeNode::HitTestBallSseInner(Ball * const pball, const int i) const
{
  HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);

  // ball can not hit itself
  if (pball == pho)
    return;

  DoHitTest(pball, pho);
}

void HitOctreeNode::HitTestBallSse(Ball * const pball) const
{
    const unsigned int padded = (m_hitoct->m_all_items+3)&0xFFFFFFFC;

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
    const unsigned int size = (m_start+m_items+3)/4;
    for (unsigned int i = m_start/4; i < size; ++i) //!! are double hits possible because of the 'overlap' to next items??
    {
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
      if ((mask & 2) != 0 /*&& (i*4+1)<m_vho.Size()*/) HitTestBallSseInner(pball, i*4+1); // boundary checks not necessary
      if ((mask & 4) != 0 /*&& (i*4+2)<m_vho.Size()*/) HitTestBallSseInner(pball, i*4+2); //  anymore as non-valid entries were
      if ((mask & 8) != 0 /*&& (i*4+3)<m_vho.Size()*/) HitTestBallSseInner(pball, i*4+3); //  initialized to keep these maskbits 0
    }

	if (m_children) // not a leaf
	{
		const Vertex3Ds vcenter((m_rectbounds.left+m_rectbounds.right)*0.5f, (m_rectbounds.top+m_rectbounds.bottom)*0.5f, (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f);

		const bool fLeft = (pball->m_rcHitRect.left <= vcenter.x);
		const bool fRight = (pball->m_rcHitRect.right >= vcenter.x);

		if (pball->m_rcHitRect.top <= vcenter.y) // Top
		{
			if (fLeft)
				m_children[0].HitTestBallSse(pball);
			if (fRight)
				m_children[1].HitTestBallSse(pball);
		}
		if (pball->m_rcHitRect.bottom >= vcenter.y) // Bottom
		{
			if (fLeft)
				m_children[2].HitTestBallSse(pball);
			if (fRight)
				m_children[3].HitTestBallSse(pball);
		}
	}
}

void HitOctreeNode::HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit) const
{
	for (unsigned i=m_start; i<m_start+m_items; i++)
	{
#ifdef LOG
		cTested++;
#endif
		HitObject * const pho = m_hitoct->m_org_vho->ElementAt(m_hitoct->m_org_idx[i]);
		if ((pball != pho) && // ball cannot hit itself
			fRectIntersect3D(pball->m_rcHitRect,pho->m_rcHitRect))
		{
#ifdef LOG
			cDeepTested++;
#endif
			const float newtime = pho->HitTest(pball, pball->m_coll.hittime, pball->m_coll);
			if (newtime >= 0)
				pvhoHit->AddElement(pho);
		}
	}

	if (m_children) // not a leaf
	{
		const Vertex3Ds vcenter((m_rectbounds.left+m_rectbounds.right)*0.5f, (m_rectbounds.top+m_rectbounds.bottom)*0.5f, (m_rectbounds.zlow+m_rectbounds.zhigh)*0.5f);

		const bool fLeft = (pball->m_rcHitRect.left <= vcenter.x);
		const bool fRight = (pball->m_rcHitRect.right >= vcenter.x);

#ifdef LOG
		cTested++;
#endif

		if (pball->m_rcHitRect.top <= vcenter.y) // Top
		{
			if (fLeft)
				m_children[0].HitTestXRay(pball, pvhoHit);
			if (fRight)
				m_children[1].HitTestXRay(pball, pvhoHit);
		}
		if (pball->m_rcHitRect.bottom >= vcenter.y) // Bottom
		{
			if (fLeft)
				m_children[2].HitTestXRay(pball, pvhoHit);
			if (fRight)
				m_children[3].HitTestXRay(pball, pvhoHit);
		}
	}
}
