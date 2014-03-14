#pragma once

class HitOctree
{
public:
	HitOctree(Vector<HitObject> *vho, const unsigned int num_items)
	{
		m_all_items = num_items;
		
		l_r_t_b_zl_zh = NULL;
		
		m_org_idx = (unsigned int*)malloc(num_items*4);
		tmp = (unsigned int*)malloc(num_items*4);

		m_org_vho = vho;
	}
	~HitOctree();

	void InitSseArrays();

	Vector<HitObject> *m_org_vho;

	unsigned int * __restrict m_org_idx;

	unsigned int m_all_items;
	unsigned int * __restrict tmp;
	
	float* __restrict l_r_t_b_zl_zh;
};

class HitOctreeNode
{
public:
	HitOctreeNode() { m_children = NULL; m_hitoct = NULL; }
	~HitOctreeNode();

	void HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit) const;

	void HitTestBall(Ball * const pball) const;
	void HitTestBallSse(Ball * const pball) const;
	void HitTestBallSseInner(Ball * const pball, const int i) const;

	void CreateNextLevel();

	FRect3D m_rectbounds;
	unsigned int m_start;
	unsigned int m_items;

	HitOctreeNode * __restrict m_children; // if NULL then this is a leaf, otherwise keeps the 8 children

	HitOctree * __restrict m_hitoct; //!! meh, stupid

#ifndef NDEBUG
    void DumpTree(int indentLevel)
    {
        char indent[256];
        for (int i = 0; i <= indentLevel; ++i)
            indent[i] = (i==indentLevel) ? 0 : ' ';
        char msg[256];
        sprintf(msg, "[%f %f %f %f %f %f], items=%u", m_rectbounds.left, m_rectbounds.right, m_rectbounds.top, m_rectbounds.bottom, m_rectbounds.zlow, m_rectbounds.zhigh, m_items);
        strcat(indent, msg);
        OutputDebugString(indent);
        if (m_children)
        {
            m_children[0].DumpTree(indentLevel + 1);
            m_children[1].DumpTree(indentLevel + 1);
            m_children[2].DumpTree(indentLevel + 1);
            m_children[3].DumpTree(indentLevel + 1);
        }
    }
#endif
};
