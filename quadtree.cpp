
HitQuadtree::~HitQuadtree()
{
    if(lefts != 0)
    {
        _aligned_free(lefts);
        _aligned_free(rights);
        _aligned_free(tops);
        _aligned_free(bottoms);
        _aligned_free(zlows);
        _aligned_free(zhighs);
    }

    if (!m_fLeaf)
    {
        for (int i=0; i<4; i++)
        {
            delete m_children[i];
        }
    }
}

void HitQuadtree::CreateNextLevel()
{
    m_fLeaf = false;

    m_vcenter.x = (m_rectbounds.left + m_rectbounds.right)*0.5f;
    m_vcenter.y = (m_rectbounds.top + m_rectbounds.bottom)*0.5f;
    m_vcenter.z = (m_rectbounds.zlow + m_rectbounds.zhigh)*0.5f;

    Vector<HitObject> vRemain; // hit objects which did not go to a quadrant

    for (int i=0; i<4; i++)
    {
        m_children[i] = new HitQuadtree();

        m_children[i]->m_rectbounds.left = (i&1) ? m_vcenter.x : m_rectbounds.left;
        m_children[i]->m_rectbounds.top  = (i&2) ? m_vcenter.y : m_rectbounds.top;
        m_children[i]->m_rectbounds.zlow = m_rectbounds.zlow;

        m_children[i]->m_rectbounds.right  = (i&1) ? m_rectbounds.right  : m_vcenter.x;
        m_children[i]->m_rectbounds.bottom = (i&2) ? m_rectbounds.bottom : m_vcenter.y;
        m_children[i]->m_rectbounds.zhigh  = m_rectbounds.zhigh;

        m_children[i]->m_fLeaf = true;
    }

    // sort items into appropriate child nodes
    for (int i=0;i<m_vho.Size();i++)
    {
        int oct;
        HitObject * const pho = m_vho.ElementAt(i);

        if (pho->m_rcHitRect.right < m_vcenter.x)
            oct = 0;
        else if (pho->m_rcHitRect.left > m_vcenter.x)
            oct = 1;
        else
            oct = 128;

        if (pho->m_rcHitRect.bottom < m_vcenter.y)
            oct |= 0;
        else if (pho->m_rcHitRect.top > m_vcenter.y)
            oct |= 2;
        else
            oct |= 128;

        if ((oct & 128) == 0)
            m_children[oct]->m_vho.AddElement(pho);
        else
            vRemain.AddElement(pho);
    }

    m_vho.RemoveAllElements();
    for (int i=0; i<vRemain.Size(); i++)
    {
        m_vho.AddElement(vRemain.ElementAt(i));
    }

    if (m_vcenter.x - m_rectbounds.left > 125.0f)       // TODO: bad heuristic, improve this
    {
        for (int i=0; i<4; ++i)
        {
            m_children[i]->CreateNextLevel();
        }
    }

    InitSseArrays();
    for (int i=0; i<4; ++i)
        m_children[i]->InitSseArrays();
}

void HitQuadtree::InitSseArrays()
{
    // build SSE boundary arrays of the local hit-object list
    // (don't init twice)
    const int padded = ((m_vho.Size()+3)/4)*4;
    const int ssebytes = sizeof(float) * padded;
    if (ssebytes > 0 && lefts == 0)
    {
        lefts = (float*)_aligned_malloc(ssebytes, 16);
        rights = (float*)_aligned_malloc(ssebytes, 16);
        tops = (float*)_aligned_malloc(ssebytes, 16);
        bottoms = (float*)_aligned_malloc(ssebytes, 16);
        zlows = (float*)_aligned_malloc(ssebytes, 16);
        zhighs = (float*)_aligned_malloc(ssebytes, 16);

        for (int j=0; j<m_vho.Size(); j++)
        {
            const FRect3D r = m_vho.ElementAt(j)->m_rcHitRect;
            lefts[j] = r.left;
            rights[j] = r.right;
            tops[j] = r.top;
            bottoms[j] = r.bottom;
            zlows[j] = r.zlow;
            zhighs[j] = r.zhigh;
        }

        for (int j=m_vho.Size(); j<padded; j++)
        {
            lefts[j] = FLT_MAX;
            rights[j] = -FLT_MAX;
            tops[j] = FLT_MAX;
            bottoms[j] = -FLT_MAX;
            zlows[j] = FLT_MAX;
            zhighs[j] = -FLT_MAX;
        }
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

void HitQuadtree::HitTestBall(Ball * const pball) const
{
    for (int i=0; i<m_vho.Size(); i++)
    {
#ifdef LOG
        cTested++;
#endif
        if ((pball != m_vho.ElementAt(i)) // ball can not hit itself
                && fRectIntersect3D(pball->m_rcHitRect, m_vho.ElementAt(i)->m_rcHitRect))
        {
#ifdef LOG
            cDeepTested++;
#endif
            DoHitTest(pball, m_vho.ElementAt(i));
        }
    }//end for loop

    if (!m_fLeaf)
    {
        const bool fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
        const bool fRight = (pball->m_rcHitRect.right >= m_vcenter.x);

#ifdef LOG
        cTested++;
#endif
        if (pball->m_rcHitRect.top <= m_vcenter.y) // Top
        {
            if (fLeft)
                m_children[0]->HitTestBallSse(pball);
            if (fRight)
                m_children[1]->HitTestBallSse(pball);
        }
        if (pball->m_rcHitRect.bottom >= m_vcenter.y) // Bottom
        {
            if (fLeft)
                m_children[2]->HitTestBallSse(pball);
            if (fRight)
                m_children[3]->HitTestBallSse(pball);
        }
    }
}


void HitQuadtree::HitTestBallSseInner(Ball * const pball, const int i) const
{
    // ball can not hit itself
    if (pball == m_vho.ElementAt(i))
        return;

    DoHitTest(pball, m_vho.ElementAt(i));
}

void HitQuadtree::HitTestBallSse(Ball * const pball) const
{
    if (lefts != 0)
    {
        // init SSE registers with ball bbox
        const __m128 bleft = _mm_set1_ps(pball->m_rcHitRect.left);
        const __m128 bright = _mm_set1_ps(pball->m_rcHitRect.right);
        const __m128 btop = _mm_set1_ps(pball->m_rcHitRect.top);
        const __m128 bbottom = _mm_set1_ps(pball->m_rcHitRect.bottom);
        const __m128 bzlow = _mm_set1_ps(pball->m_rcHitRect.zlow);
        const __m128 bzhigh = _mm_set1_ps(pball->m_rcHitRect.zhigh);

        const __m128* const pL = (__m128*)lefts;
        const __m128* const pR = (__m128*)rights;
        const __m128* const pT = (__m128*)tops;
        const __m128* const pB = (__m128*)bottoms;
        const __m128* const pZl = (__m128*)zlows;
        const __m128* const pZh = (__m128*)zhighs;

        // loop implements 4 collision checks at once
        // (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow)
        const int size = (m_vho.Size()+3)/4;
        for (int i=0; i<size; ++i)
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
    }

    if (!m_fLeaf)
    {
        const bool fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
        const bool fRight = (pball->m_rcHitRect.right >= m_vcenter.x);

        if (pball->m_rcHitRect.top <= m_vcenter.y) // Top
        {
            if (fLeft)
                m_children[0]->HitTestBallSse(pball);
            if (fRight)
                m_children[1]->HitTestBallSse(pball);
        }
        if (pball->m_rcHitRect.bottom >= m_vcenter.y) // Bottom
        {
            if (fLeft)
                m_children[2]->HitTestBallSse(pball);
            if (fRight)
                m_children[3]->HitTestBallSse(pball);
        }
    }
}

void HitQuadtree::HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit) const
{
    for (int i=0; i<m_vho.Size(); i++)
    {
#ifdef LOG
        cTested++;
#endif
        if ((pball != m_vho.ElementAt(i)) && fRectIntersect3D(pball->m_rcHitRect, m_vho.ElementAt(i)->m_rcHitRect))
        {
#ifdef LOG
            cDeepTested++;
#endif
            const float newtime = m_vho.ElementAt(i)->HitTest(pball, pball->m_coll.hittime, pball->m_coll);
            if (newtime >= 0)
            {
                pvhoHit->AddElement(m_vho.ElementAt(i));
            }
        }
    }

    if (!m_fLeaf)
    {
        const bool fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
        const bool fRight = (pball->m_rcHitRect.right >= m_vcenter.x);

#ifdef LOG
        cTested++;
#endif

        if (pball->m_rcHitRect.top <= m_vcenter.y) // Top
        {
            if (fLeft)
                m_children[0]->HitTestXRay(pball, pvhoHit);
            if (fRight)
                m_children[1]->HitTestXRay(pball, pvhoHit);
        }
        if (pball->m_rcHitRect.bottom >= m_vcenter.y) // Bottom
        {
            if (fLeft)
                m_children[2]->HitTestXRay(pball, pvhoHit);
            if (fRight)
                m_children[3]->HitTestXRay(pball, pvhoHit);
        }
    }
}
