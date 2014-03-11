#pragma once

enum
	{
	eNull,
	eLineSeg,
	eJoint,
	eCircle,
	eFlipper,
	ePlunger,
	eSpinner,
	eBall,
	e3DPoly,
	eTriangle,
	e3DLine,
	eGate,
	eTextbox,
    eDispReel,
	eLightSeq,
	ePrimitive,
	eTrigger,	// this value and greater are volume set tested, add rigid or non-volume set above
	eKicker		// this is done to limit to one test
	};

extern float c_hardFriction; 
extern float c_hardScatter; 
extern float c_maxBallSpeedSqr; 
extern float c_dampingFriction;

extern float c_plungerNormalize;  //Adjust Mech-Plunger, useful for component change or weak spring etc.
extern bool c_plungerFilter;

class Ball;
class HitObject;
class AnimObject;

HitObject *CreateCircularHitPoly(const float x, const float y, const float z, const float r, const int sections);

class HitObject
	{
public:

	HitObject();
	virtual ~HitObject() {}

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal) = 0;

	virtual int GetType() const = 0;

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal) = 0;

	virtual void CalcHitRect() = 0;
	
	virtual AnimObject *GetAnimObject() {return NULL;}

	IFireEvents *m_pfe;
	float m_threshold;
	
	//IDispatch *m_pdisp;
	IFireEvents *m_pfedebug;

	FRect3D m_rcHitRect;

	BOOL  m_fEnabled;
	int   m_ObjType;
	void* m_pObj;
	float m_elasticity;
	float m_antifriction;
	float m_scatter;
	};

class AnimObject
	{
public:
	virtual bool FMover() const {return false;}
	virtual void UpdateDisplacements(const float dtime) {}
	virtual void UpdateVelocities() {}

    virtual void Check3D() {}
	virtual ObjFrame *Draw3D(const RECT * const prc) {return NULL;}
	virtual void Reset() {}
	};

class LineSeg : public HitObject
	{
public:
	virtual float HitTestBasic(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal, const bool direction, const bool lateral, const bool rigid);
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);
	virtual int GetType() const {return eLineSeg;}
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);
	void CalcNormal();
	void CalcLength();
	virtual void CalcHitRect();

	Vertex2D normal;
	Vertex2D v1, v2;
	float length;
	};

class HitCircle : public HitObject
	{
public:
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	float HitTestBasicRadius(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal,
									const bool direction, const bool lateral, const bool rigid);

	float HitTestRadius(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	virtual int GetType() const {return eCircle;}

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual void CalcHitRect();

	Vertex2D center;
	float radius;
	float zlow;
	float zhigh;
	};

class Joint : public HitCircle
	{
public:
	Joint();

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	virtual int GetType() const {return eJoint;}

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual void CalcHitRect();

	//Vertex2D v;
	Vertex2D normal;
	};




// Callback for the broadphase collision test.
// Perform the actual hittest between ball and hit object and update
// collision information if a hit occurred.
void DoHitTest(Ball *pball, HitObject *pho);

