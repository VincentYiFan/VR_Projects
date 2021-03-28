#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Delegate.hpp"

typedef unsigned int uint;

//----------------------------------------------------------------------------------------------------------------------------
class Physics2D;
class Collider2D;
struct Collision2D;
//----------------------------------------------------------------------------------------------------------------------------

enum eSimulationMode: unsigned char
{
	SIMULATION_MODE_STATIC,
	SIMULATION_MODE_KINEMATIC,
	SIMULATION_MODE_DYNAMIC,

	NUM_SIMULATION_MODES
};



class Rigidbody2D
{
	friend class Physics2D;

public:
	~Rigidbody2D();           // assert the collider is already null 
	void		UpdateFrameDeltaPos( float deltaSeconds );
	void		Destroy();                             // mark self for destruction, and mark collider as destruction
	void		TakeCollider( Collider2D* collider );  // takes ownership of a collider (destroying my current one if present)


	void		SetPosition( Vec2 position );          // update my position, and my collider's world position
	void		SetIsEnabled( const bool isEnabled );
	void		SetVelocity( Vec2 velocity );
	void		SetSimulationMode( eSimulationMode simulationMode ); 
	void		SetLayer( uint layerIndex );
	

	uint		GetLayer() const { return m_layerIndex; }
	Vec2		GetVelocity() const { return m_velocity; }
	Vec2		GetVerletVelocity() const { return m_verletVelocity; }
	Vec2		GetImpactVelocity( const Vec2& r );
	float		GetRotationInRadians() const { return m_rotationInRadians; }
	float		GetAngularVelocity() const { return m_angularVelocity; }
	float		GetFrameTorque() const { return m_frameTorque; }
	float		GetMomentOfInertia() const;	
	float		GetMass() const;
	float		GetOrientationDegrees() const;
	Collider2D* GetCollider() { return m_collider; }


	void		ApplyDragForce();
	void		AddForce( Vec2 force );
	void		ApplyImpulseAt( Vec2 point, Vec2 impulse );
	void		CalculateMoment();

	void		SetUserData( uint type, void* data );
	void*		GetUserData( uint type ) const { return (type == m_userDataType) ? m_userData : nullptr; }

public:
	// collision events
	Delegate<Collision2D const&> OnOverlapEnter;  // called on frames a contact happens, but it wasn't their the frame before
	Delegate<Collision2D const&> OnOverlapStay;   // called on frames a contact happens and it happened the frame before
	Delegate<Collision2D const&> OnOverlapExit;   // called on frames where no contact happened but we had one the frame before
	Delegate<Collision2D const&> OnCollision;	  // called after collision has been resolved this fixed step


public:
	Physics2D* m_system = nullptr;     // which scene created/owns this object
	Collider2D* m_collider = nullptr;

	Vec2 m_worldPosition = Vec2::ZERO;     // where in the world is this rigidbody
	Vec2 m_lastFramePos = Vec2::ZERO;
	Vec2 m_frameStartPos = Vec2::ZERO;

	bool m_isGarbage = false;
	bool m_isEnabled = true;

	float m_drag = 0.f;

	eSimulationMode m_simulatonMode = SIMULATION_MODE_STATIC;

public:
	Vec2 m_velocity = Vec2::ZERO;
	Vec2 m_lastFrameVelcity = Vec2::ZERO;
	Vec2 m_verletVelocity = Vec2::ZERO;
	Vec2 m_frameDeltaPos = Vec2::ZERO;	

	bool m_isLastFramePosInitialize = false;
	bool m_isLastFrameVelInitialize = false;

	// Angular
	float m_rotationInRadians = 0.f;
	float m_angularVelocity = 0.f;
	float m_frameTorque = 0.f;
	float m_moment;		// Moment of Inertia
	float m_orientationDegrees = 0.f;

	uint m_layerIndex = 0;

public:
	void* m_userData = nullptr;
	uint m_userDataType = 0;
};
