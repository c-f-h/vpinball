#pragma once
class PlungerAnimObject : public AnimObject
	{
public:
	virtual void UpdateDisplacements(const float dtime);
	virtual void UpdateVelocities();

	virtual BOOL FMover() const {return fTrue;}

	virtual void Check3D();
	virtual ObjFrame *Draw3D(const RECT * const prc);

	void SetObjects(const float len);

	float mechPlunger() const; // Returns mechanical plunger position 0 at rest, +1 pulled (fully extended)

	LineSeg m_linesegBase;
	LineSeg m_linesegEnd;
	LineSeg m_linesegSide[2];

	Joint m_jointBase[2];
	Joint m_jointEnd[2];

	Vector<ObjFrame> m_vddsFrame;

	int m_iframe; //Frame index that this flipper is currently displaying

	float m_speed;
	float m_pos;
	float m_posdesired;
	float m_posFrame; // Location of plunger at beginning of frame
	bool  m_fAcc;
	float m_mass;

	float m_x,m_x2,m_y;

	float m_force;

	float m_frameStart;
	float m_frameEnd;
	int m_mechTimeOut;

	bool recock;
	float err_fil;	// integrate error over multiple update periods 

	float m_parkPosition;
	float m_scatterVelocity;
	float m_breakOverVelocity;
	Plunger* m_plunger;
	};

class HitPlunger :
	public HitObject
	{
public:

	HitPlunger(const float x, const float y, const float x2, const float pos, const float zheight, Plunger * const pPlunger);
	~HitPlunger() {}

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	virtual int GetType() const {return ePlunger;}

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual void CalcHitRect();

	virtual AnimObject *GetAnimObject() {return &m_plungeranim;}

	//Vector<HitObject> m_vho;

	//float m_acc;

	PlungerAnimObject m_plungeranim;

	Plunger *m_pplunger;
	};
