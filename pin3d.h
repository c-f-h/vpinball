#pragma once
#include "RenderDevice.h"
#include "Texture.h"

extern int NumVideoBytes;

enum
{
	eLightProject1 = 1,
	ePictureTexture = 0
};

enum
{
	TEXTURE_MODE_POINT,				// Point sampled (aka no) texture filtering.
	TEXTURE_MODE_BILINEAR,			// Bilinar texture filtering. 
	TEXTURE_MODE_TRILINEAR,			// Trilinar texture filtering. 
	TEXTURE_MODE_ANISOTROPIC		// Anisotropic texture filtering. 
};

class Matrix3D : public D3DMATRIX
	{
public:
    // premultiply the given matrix, i.e., result = mult * (*this)
	inline void Multiply(const Matrix3D &mult, Matrix3D &result) const
	{
	Matrix3D matrixT;
	for (int i=0; i<4; ++i)
		{
		for (int l=0; l<4; ++l)
			{
			matrixT.m[i][l] = (m[0][l] * mult.m[i][0]) + (m[1][l] * mult.m[i][1]) +
						      (m[2][l] * mult.m[i][2]) + (m[3][l] * mult.m[i][3]);
			}
		}
	result = matrixT;
	}
    void SetTranslation(float tx, float ty, float tz)
    {
        SetIdentity();
        _41 = tx;
        _42 = ty;
        _43 = tz;
    }
    void SetScaling(float sx, float sy, float sz)
    {
        SetIdentity();
        _11 = sx;
        _22 = sy;
        _33 = sz;
    }
	inline void RotateXMatrix(const GPINFLOAT x)
	{
	SetIdentity();
	_22 = _33 = (float)cos(x);
	_23 = (float)sin(x);
	_32 = -_23;
	}
	inline void RotateYMatrix(const GPINFLOAT y)
	{
	SetIdentity();
	_11 = _33 = (float)cos(y);
	_31 = (float)sin(y);
	_13 = -_31;
	}
	inline void RotateZMatrix(const GPINFLOAT z)
	{
	SetIdentity();
	_11 = _22 = (float)cos(z);
	_12 = (float)sin(z);
	_21 = -_12;
	}
	inline void SetIdentity()
	{
	_11 = _22 = _33 = _44 = 1.0f;
	_12 = _13 = _14 = _41 =
	_21 = _23 = _24 = _42 =
	_31 = _32 = _34 = _43 = 0.0f;
	}
	inline void Scale(const float x, const float y, const float z)
	{
	_11 *= x;
	_12 *= x;
	_13 *= x;
	_21 *= y;
	_22 *= y;
	_23 *= y;
	_31 *= z;
	_32 *= z;
	_33 *= z;
	}

    // extract the matrix corresponding to the 3x3 rotation part
    void GetRotationPart(Matrix3D& rot)
    {
        rot._11 = _11; rot._12 = _12; rot._13 = _13; rot._14 = 0.0f;
        rot._21 = _21; rot._22 = _22; rot._23 = _23; rot._24 = 0.0f;
        rot._31 = _31; rot._32 = _32; rot._33 = _33; rot._34 = 0.0f;
        rot._41 = rot._42 = rot._43 = 0.0f; rot._44 = 1.0f;
    }

    // generic multiply function for everything that has .x, .y and .z
    template <class VecIn, class VecOut>
    void MultiplyVector(const VecIn& vIn, VecOut& vOut) const
    {
        // Transform it through the current matrix set
        const float xp = _11*vIn.x + _21*vIn.y + _31*vIn.z + _41;
        const float yp = _12*vIn.x + _22*vIn.y + _32*vIn.z + _42;
        const float zp = _13*vIn.x + _23*vIn.y + _33*vIn.z + _43;
        const float wp = _14*vIn.x + _24*vIn.y + _34*vIn.z + _44;

        const float inv_wp = 1.0f/wp;
        vOut.x = xp*inv_wp;
        vOut.y = yp*inv_wp;
        vOut.z = zp*inv_wp;
    }

	inline Vertex3Ds MultiplyVector(const Vertex3Ds &v) const
	{
	// Transform it through the current matrix set
	const float xp = _11*v.x + _21*v.y + _31*v.z + _41;
	const float yp = _12*v.x + _22*v.y + _32*v.z + _42;
	const float zp = _13*v.x + _23*v.y + _33*v.z + _43;
	const float wp = _14*v.x + _24*v.y + _34*v.z + _44;

	const float inv_wp = 1.0f/wp;
	Vertex3Ds pv3DOut;
	pv3DOut.x = xp*inv_wp;
	pv3DOut.y = yp*inv_wp;
	pv3DOut.z = zp*inv_wp;
	return pv3DOut;
	}

	inline Vertex3Ds MultiplyVectorNoTranslate(const Vertex3Ds &v) const
	{
	// Transform it through the current matrix set
	const float xp = _11*v.x + _21*v.y + _31*v.z;
	const float yp = _12*v.x + _22*v.y + _32*v.z;
	const float zp = _13*v.x + _23*v.y + _33*v.z;

	Vertex3Ds pv3DOut;
	pv3DOut.x = xp;
	pv3DOut.y = yp;
	pv3DOut.z = zp;
	return pv3DOut;
	}

    template <class VecIn, class VecOut>
    void MultiplyVectorNoTranslate(const VecIn& vIn, VecOut& vOut) const
    {
        // Transform it through the current matrix set
        const float xp = _11*vIn.x + _21*vIn.y + _31*vIn.z;
        const float yp = _12*vIn.x + _22*vIn.y + _32*vIn.z;
        const float zp = _13*vIn.x + _23*vIn.y + _33*vIn.z;

        vOut.x = xp;
        vOut.y = yp;
        vOut.z = zp;
    }

	void Invert();
	};

class PinProjection
	{
public:
	void Scale(const float x, const float y, const float z);
	void Multiply(const Matrix3D& mat);
	void Rotate(const GPINFLOAT x, const GPINFLOAT y, const GPINFLOAT z);
	void Translate(const float x, const float y, const float z);
	void FitCameraToVertices(Vector<Vertex3Ds> * const pvvertex3D, const GPINFLOAT aspect, const GPINFLOAT rotation, const GPINFLOAT inclination, const GPINFLOAT FOV, const float xlatez);
	void CacheTransform();      // compute m_matrixTotal = m_World * m_View * m_Proj
	void TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const;
	void SetFieldOfView(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar);
	void SetupProjectionMatrix(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar);

    void ComputeNearFarPlane(const Vector<Vertex3Ds>& verts);

	Matrix3D m_matWorld;
	Matrix3D m_matView;
	Matrix3D m_matProj;
	Matrix3D m_matrixTotal;

	RECT m_rcviewport;

	GPINFLOAT m_rznear, m_rzfar;
	Vertex3Ds m_vertexcamera;
	};

class Pin3D
	{
public:
	Pin3D();
	~Pin3D();

	HRESULT InitPin3D(const HWND hwnd, const bool fFullScreen, const int screenwidth, const int screenheight, const int colordepth, int &refreshrate, const int VSync, const bool useAA, const bool stereo3DFXAA);

	void InitLayout(const float left, const float top, const float right, const float bottom, const float inclination, const float FOV, const float rotation, const float scalex, const float scaley, const float xlatex, const float xlatey, const float xlatez, const float layback, const float maxSeparation, const float ZPD);

	void TransformVertices(const Vertex3D_NoTex2 * rgv,     const WORD * rgi, int count, Vertex2D * rgvout) const;

   Vertex3Ds Unproject( Vertex3Ds *point );
   Vertex3Ds Get3DPointFrom2D( POINT *p );

    void Flip(const int offsetx, const int offsety, bool vsync);

	void SetRenderTarget(RenderTarget* pddsSurface, RenderTarget* pddsZ) const;
	void SetTextureFilter(const int TextureNum, const int Mode) const;
	
    void SetTexture(Texture* pTexture);
	void SetBaseTexture(DWORD texUnit, BaseTexture* pddsTexture);

    void EnableLightMap(const float z);
    void DisableLightMap();

	void EnableAlphaTestReference(DWORD alphaRefValue) const;
    void EnableAlphaBlend( DWORD alphaRefValue, BOOL additiveBlending=fFalse );
    void DisableAlphaBlend();

    void DrawBackground();
    void RenderPlayfieldGraphics();

	void CalcShadowCoordinates(Vertex3D * const pv, const unsigned int count) const;

    const Matrix3D& GetWorldTransform() const   { return m_proj.m_matWorld; }
    const Matrix3D& GetViewTransform() const    { return m_proj.m_matView; }

private:
    void InitRenderState();
    void InitLights();

	void Identity();
   
	BaseTexture* CreateShadow(const float height);
	void CreateBallShadow();

	// Data members
public:
	RenderDevice* m_pd3dDevice;
	RenderTarget* m_pddsBackBuffer;

	D3DTexture* m_pdds3DBackBuffer;
	D3DTexture* m_pdds3DZBuffer;

	RenderTarget* m_pddsZBuffer;

	RenderTarget* m_pddsStatic;
	RenderTarget* m_pddsStaticZ;

	Texture ballTexture;
	MemTexture *ballShadowTexture;
	Texture lightTexture[2]; // 0=bumper, 1=lights
	Texture m_pddsLightWhite;

    PinProjection m_proj;

	RECT m_rcScreen;
	int m_dwRenderWidth;
	int m_dwRenderHeight;

	float skewX;
	float skewY;

	int m_width;
	int m_height;
	HWND m_hwnd;

	float m_rotation, m_inclination, m_layback;
	float m_scalex, m_scaley;
	float m_xlatex, m_xlatey;

    Vertex3Ds m_viewVec;        // direction the camera is facing

    //bool fullscreen;
	float m_maxSeparation, m_ZPD;
    ViewPort vp;

private:
    VertexBuffer *backgroundVBuffer;
    VertexBuffer *tableVBuffer;
    IndexBuffer *tableIBuffer;
    std::map<int, MemTexture*> m_xvShadowMap;

};
