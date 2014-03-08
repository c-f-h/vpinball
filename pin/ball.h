#pragma once

#include "pin/collide.h"


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
	virtual void Collide(CollisionEvent *coll);
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

	VectorVoid* m_vpVolObjs;// vector of triggers we are now inside

    CollisionEvent m_coll;  // collision information, may not be a actual hit if something else happens first

	int m_fDynamic;			// used to determine static ball conditions and velocity quenching, 

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
