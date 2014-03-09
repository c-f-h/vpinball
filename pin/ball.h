#pragma once
class HitObject;
class Ball;

class BallAnimObject : public AnimObject
{
public:
	virtual bool FMover() const {return false;} // We add ourselves to the mover list.  
											    // If we allow the table to do that, we might get added twice, 
											    // if we get created in Init code
	virtual void UpdateDisplacements(const float dtime);
	virtual void UpdateVelocities();

	Ball *m_pball;
};

int NumBallsInitted();

class Ball : public HitObject 
{
public:
	Ball();
	~Ball();

    static int GetBallsInUse();
	void Init();
    void RenderSetup();

	virtual void UpdateDisplacements(const float dtime);
	virtual void UpdateVelocities();

	// From HitObject
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);	
	virtual int GetType() const {return eBall;}
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);
	virtual void CalcHitRect();
	virtual AnimObject *GetAnimObject() {return &m_ballanim;}

	//semi-generic collide methods
	void CollideWall(const Vertex3Ds * const phitnormal, const float elasticity, float antifriction, float scatter_angle, bool collide3D=false);
	void Collide3DWall(const Vertex3Ds * const phitnormal, const float elasticity, float antifriction, float scatter_angle)
      { CollideWall(phitnormal, elasticity, antifriction, scatter_angle, true); }

	void AngularAcceleration(const Vertex3Ds& hitnormal);

	void EnsureOMObject();

	Vertex3D_NoTex2 m_rgv3DShadow[4];			// Last vertices of the ball shadow

	COLORREF m_color;

	// Per frame info
	CCO(BallEx) *m_pballex; // Object model version of the ball

	char m_szImage[MAXTOKEN];
	char m_szImageFront[MAXTOKEN];
	char m_szImageBack[MAXTOKEN];

	Texture *m_pin;
	Texture *m_pinFront;
	Texture *m_pinBack;

	HitObject *m_pho;		//pointer to hit object trial, may not be a actual hit if something else happens first
	VectorVoid* m_vpVolObjs;// vector of triggers we are now inside
	float m_hittime;		// time at which this ball will hit something
	float m_hitx, m_hity;	// position of the ball at hit time (saved to avoid floating point errors with multiple time slices)
	float m_HitDist;		// hit distance 
	float m_HitNormVel;		// hit normal Velocity
	int m_fDynamic;			// used to determine static ball conditions and velocity quenching, 
	Vertex3Ds m_hitnormal[5];// 0: hit normal, 1: hit object velocity, 2: monent and angular rate, 4: contact distance

    Vertex3D_NoTex2 vertices[4];
    Vertex3D_NoTex2 logoVertices[4];
    Vertex3D_NoTex2 reflectVerts[4];
    Vertex3D_NoTex2 logoFrontVerts[4];
    Vertex3D_NoTex2 logoBackVerts[4];

    static VertexBuffer *vertexBuffer;
    Material shadowMaterial;
    Material logoMaterial;
    Material material;

	BallAnimObject m_ballanim;

	float x;
	float y;
	float z;
	float defaultZ;   //normal height of the ball //!! remove

	Vertex3Ds oldpos[10]; // for the optional ball trails
	unsigned int ringcounter_oldpos;

	float vx;
	float vy;
	float vz;

	float drsq;	// square of distance moved

	float radius;
	float collisionMass;

//	float x_min, x_max;	// world limits on ball displacements
//	float y_min, y_max;
	float z_min, z_max;

	Vertex3Ds m_Event_Pos;
	
	Matrix3 m_orientation;
	Vertex3Ds m_angularmomentum;
	Vertex3Ds m_angularvelocity;
	Matrix3 m_inverseworldinertiatensor;
	Matrix3 m_inversebodyinertiatensor;

	bool m_HitRigid;	// Rigid = 1, Non-Rigid = 0

	bool fFrozen;

	bool m_disableLighting;
   
    static int ballsInUse;
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
