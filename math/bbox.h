#pragma once

class FRect
{
public:
   float left, top, right, bottom;
};

class FRect3D
{
public:
   float left, top, right, bottom, zlow, zhigh;

   void Clear()
   {
       left = FLT_MAX;  right = -FLT_MAX;
       top = FLT_MAX;   bottom = -FLT_MAX;
       zlow = FLT_MAX;  zhigh = -FLT_MAX;
   }

   void Extend(const FRect3D& other)
   {
       left = min(left, other.left);
       right = max(right, other.right);
       top = min(top, other.top);
       bottom = max(bottom, other.bottom);
       zlow = min(zlow, other.zlow);
       zhigh = max(zhigh, other.zhigh);
   }
};

inline bool Intersect(const RECT &rc, const int width, const int height, const POINT &p, const bool rotated) // width & height in percent/[0..100]-range
{
	if(!rotated)
		return (p.x >= rc.left*width/100 && p.x <= rc.right*width/100 && p.y >= rc.top*height/100 && p.y <= rc.bottom*height/100);
	else
		return (p.x >= rc.top*width/100 && p.x <= rc.bottom*width/100 && p.y <= height-rc.left*height/100 && p.y >= height-rc.right*height/100);
}

inline bool fRectIntersect3D(const FRect3D &rc1, const FRect3D &rc2)
{
	const __m128 rc1128 = _mm_loadu_ps(&rc1.left); // this shouldn't use loadu, but doesn't matter anymore nowadays anyhow
	const __m128 rc1sh = _mm_shuffle_ps(rc1128,rc1128,_MM_SHUFFLE(1, 0, 3, 2));
	const __m128 test = _mm_cmpge_ps(rc1sh,_mm_loadu_ps(&rc2.left));
	const int mask = _mm_movemask_ps(test);
//   return ((mask == (1|(1<<1)|(0<<2)|(0<<3))) && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow); //!! use SSE, too?
	return ((mask == 3) && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow); //!! use SSE, too?

	//return (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow);
}

