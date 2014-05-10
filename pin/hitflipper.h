#pragma once

//#define DEBUG_FLIPPERS

class FlipperAnimObject : public AnimObject
	{
public:
    FlipperAnimObject(const Vertex2D& center, float baser, float endr, float flipr, float angleStart, float angleEnd,
                      float zlow, float zhigh, float strength, float mass, float returnRatio);

	void SetObjects(const float angle);
	virtual void UpdateDisplacements(const float dtime);
	virtual void UpdateVelocities();

	virtual bool FMover() const {return true;}

	virtual void Check3D()                              { }

    void SetSolenoidState(bool s);
    float GetStrokeRatio() const;

    // rigid body functions
    Vertex3Ds SurfaceVelocity(const Vertex3Ds& surfP) const;
    Vertex3Ds SurfaceAcceleration(const Vertex3Ds& surfP) const;

    float GetHitTime() const;

    void ApplyImpulse(const Vertex3Ds& surfP, const Vertex3Ds& impulse);

	Flipper *m_pflipper;

	LineSeg m_lineseg1;
	LineSeg m_lineseg2;
	HitCircle m_hitcircleEnd;
	HitCircle m_hitcircleBase;
	float m_endradius;
	float faceNormOffset; 

	// New Flipper motion basis, uses Green's transform to rotate these valuse to curAngle
	Vertex2D m_leftFaceNormal, m_rightFaceNormal, m_leftFaceBase, m_rightFaceBase;
	Vertex2D m_endRadiusCenter;

    float m_angularMomentum;
    float m_angularAcceleration;
	float m_anglespeed;
	float m_angleCur;

	float m_flipperradius;
	float m_force;
    float m_returnRatio;

	float m_height;

    int m_dir;
    bool m_solState;        // is solenoid enabled?
    bool m_isInContact;
    float m_curTorque;
    float m_contactTorque;
    float m_torqueRampupSpeed;

	float m_angleStart, m_angleEnd;
	float m_angleMin, m_angleMax;

	float m_inertia;	//moment of inertia

	int m_EnableRotateEvent;

	Vertex2D zeroAngNorm; // base norms at zero degrees	

	bool m_fEnabled;
   bool m_fVisible;
	bool m_lastHitFace;
   bool m_fCompatibility;

#ifdef DEBUG_FLIPPERS
    U32 m_startTime;
#endif
	};

class HitFlipper :
	public HitObject
	{
public:
	Vertex2D v;
	//float rad1, rad2;

	HitFlipper(const Vertex2D& center, float baser, float endr, float flipr, float angleStart, float angleEnd,
		       float zlow, float zhigh, float strength, float mass, float returnRatio);
	~HitFlipper();

	virtual float HitTestFlipperFace(const Ball * pball, const float dtime, CollisionEvent& coll, const bool face1);

	virtual float HitTestFlipperEnd(const Ball * pball, const float dtime, CollisionEvent& coll);

    float GetHitTime() const;
	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
	
	virtual int GetType() const {return eFlipper;}

	virtual void Collide(CollisionEvent *coll);
    void Contact(CollisionEvent& coll, float dtime);

	virtual AnimObject *GetAnimObject() {return &m_flipperanim;}

	virtual void CalcHitRect();

	Flipper *m_pflipper;

	FlipperAnimObject m_flipperanim;
	int m_last_hittime;
	};
