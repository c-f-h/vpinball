#pragma once

// 3x3 matrix for representing linear transformation of 3D vectors
class Matrix3
{
public:
   Matrix3()
   {
      Identity();
   }

   void scaleX(const float factor)
   {
      m_d[0][0] *= factor;
   }
   void scaleY(const float factor)
   {
      m_d[1][1] *= factor;
   }
   void scaleZ(const float factor)
   {
      m_d[2][2] *= factor;
   }

   void CreateSkewSymmetric(const Vertex3Ds &pv3D)
   {
      m_d[0][0] = 0; m_d[0][1] = -pv3D.z; m_d[0][2] = pv3D.y;
      m_d[1][0] = pv3D.z; m_d[1][1] = 0; m_d[1][2] = -pv3D.x;
      m_d[2][0] = -pv3D.y; m_d[2][1] = pv3D.x; m_d[2][2] = 0;
   }

   void MultiplyScalar(const float scalar)
   {
      for (int i=0; i<3; ++i)
         for (int l=0; l<3; ++l)
            m_d[i][l] *= scalar;
   }

   template <class VecType>
   Vertex3Ds operator* (const VecType& v) const
   {
      return Vertex3Ds(
         m_d[0][0] * v.x + m_d[0][1] * v.y + m_d[0][2] * v.z,
         m_d[1][0] * v.x + m_d[1][1] * v.y + m_d[1][2] * v.z,
         m_d[2][0] * v.x + m_d[2][1] * v.y + m_d[2][2] * v.z);
   }

   template <class VecType>
   Vertex3Ds MultiplyVector(const VecType& v) const
     { return (*this) * v; }

   void MultiplyMatrix(const Matrix3 * const pmat1, const Matrix3 * const pmat2)
   {
      Matrix3 matans;
      for(int i=0; i<3; ++i)
         for(int l=0; l<3; ++l)
            matans.m_d[i][l] = pmat1->m_d[i][0] * pmat2->m_d[0][l] +
            pmat1->m_d[i][1] * pmat2->m_d[1][l] +
            pmat1->m_d[i][2] * pmat2->m_d[2][l];

      // Copy the final values over later.  This makes it so pmat1 and pmat2 can
      // point to the same matrix.
      for(int i=0; i<3; ++i)
         for (int l=0; l<3; ++l)
            m_d[i][l] = matans.m_d[i][l];
   }

   void AddMatrix(const Matrix3 * const pmat1, const Matrix3 * const pmat2)
   {
      for (int i=0; i<3; ++i)
         for (int l=0; l<3; ++l)
            m_d[i][l] = pmat1->m_d[i][l] + pmat2->m_d[i][l];
   }

   void OrthoNormalize()
   {
      Vertex3Ds vX(m_d[0][0], m_d[1][0], m_d[2][0]);
      Vertex3Ds vY(m_d[0][1], m_d[1][1], m_d[2][1]);
      vX.Normalize();
      Vertex3Ds vZ = CrossProduct(vX, vY);
      vZ.Normalize();
      vY = CrossProduct(vZ, vX);
      vY.Normalize();

      m_d[0][0] = vX.x; m_d[0][1] = vY.x; m_d[0][2] = vZ.x;
      m_d[1][0] = vX.y; m_d[1][1] = vY.y; m_d[1][2] = vZ.y;
      m_d[2][0] = vX.z; m_d[2][1] = vY.z; m_d[2][2] = vZ.z;
   }

   void Transpose(Matrix3 * const pmatOut) const
   {
      for(int i=0; i<3; ++i)
      {
         pmatOut->m_d[0][i] = m_d[i][0];
         pmatOut->m_d[1][i] = m_d[i][1];
         pmatOut->m_d[2][i] = m_d[i][2];
      }
   }

   void Identity(const float value = 1.0f)
   {
      m_d[0][0] = m_d[1][1] = m_d[2][2] = value;
      m_d[0][1] = m_d[0][2] = 
         m_d[1][0] = m_d[1][2] = 
         m_d[2][0] = m_d[2][1] = 0.0f;
   }

   float m_d[3][3];
};


////////////////////////////////////////////////////////////////////////////////


// 4x4 matrix for representing affine transformations of 3D vectors
class Matrix3D : public D3DMATRIX
{
public:
    // premultiply the given matrix, i.e., result = mult * (*this)
    void Multiply(const Matrix3D &mult, Matrix3D &result) const
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
    void SetTranslation(const Vertex3Ds& t)
    {
        SetTranslation(t.x, t.y, t.z);
    }

    void SetScaling(float sx, float sy, float sz)
    {
        SetIdentity();
        _11 = sx;
        _22 = sy;
        _33 = sz;
    }

    void RotateXMatrix(const GPINFLOAT x)
    {
        SetIdentity();
        _22 = _33 = (float)cos(x);
        _23 = (float)sin(x);
        _32 = -_23;
    }

    void RotateYMatrix(const GPINFLOAT y)
    {
        SetIdentity();
        _11 = _33 = (float)cos(y);
        _31 = (float)sin(y);
        _13 = -_31;
    }

    void RotateZMatrix(const GPINFLOAT z)
    {
        SetIdentity();
        _11 = _22 = (float)cos(z);
        _12 = (float)sin(z);
        _21 = -_12;
    }

    void SetIdentity()
    {
        _11 = _22 = _33 = _44 = 1.0f;
        _12 = _13 = _14 = _41 =
        _21 = _23 = _24 = _42 =
        _31 = _32 = _34 = _43 = 0.0f;
    }
    void Scale(const float x, const float y, const float z)
    {
        _11 *= x; _12 *= x; _13 *= x;
        _21 *= y; _22 *= y; _23 *= y;
        _31 *= z; _32 *= z; _33 *= z;
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

    Vertex3Ds MultiplyVector(const Vertex3Ds &v) const
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

    Vertex3Ds MultiplyVectorNoTranslate(const Vertex3Ds &v) const
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

