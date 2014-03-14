#pragma once

#include "collide.h"

class HitQuadtree
{
public:
    HitQuadtree()
    {
        m_fLeaf = true;
        lefts = 0;
        rights = 0;
        tops = 0;
        bottoms = 0;
        zlows = 0;
        zhighs = 0;
    }

    ~HitQuadtree();

    void HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit) const;

    void HitTestBall(Ball * const pball) const;
    void HitTestBallSse(Ball * const pball) const;
    void HitTestBallSseInner(Ball * const pball, const int i) const;

    void CreateNextLevel();

    Vector<HitObject> m_vho;
    FRect3D m_rectbounds;

private:

    HitQuadtree * __restrict m_children[4];

    // helper arrays for SSE boundary checks
    void InitSseArrays();
    float* __restrict lefts;
    float* __restrict rights;
    float* __restrict tops;
    float* __restrict bottoms;
    float* __restrict zlows;
    float* __restrict zhighs;

    Vertex3Ds m_vcenter;

    bool m_fLeaf;

#ifndef NDEBUG
public:
    void DumpTree(int indentLevel)
    {
        char indent[256];
        for (int i = 0; i <= indentLevel; ++i)
            indent[i] = (i==indentLevel) ? 0 : ' ';
        char msg[256];
        sprintf(msg, "[%f %f %f %f %f %f], items=%u", m_rectbounds.left, m_rectbounds.right, m_rectbounds.top, m_rectbounds.bottom, m_rectbounds.zlow, m_rectbounds.zhigh, m_vho.Size());
        strcat(indent, msg);
        OutputDebugString(indent);
        if (!m_fLeaf)
        {
            m_children[0]->DumpTree(indentLevel + 1);
            m_children[1]->DumpTree(indentLevel + 1);
            m_children[2]->DumpTree(indentLevel + 1);
            m_children[3]->DumpTree(indentLevel + 1);
        }
    }
#endif
};
