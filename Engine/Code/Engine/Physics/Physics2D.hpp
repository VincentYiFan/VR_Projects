#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Delegate.hpp"
#include <vector>
//----------------------------------------------------------------------------------------------------------------------------
class Rigidbody2D;
class Collider2D;
class DiscCollider2D;
class PolygonCollider2D;
class Timer;
class Clock;
struct Collision2D;
//----------------------------------------------------------------------------------------------------------------------------

typedef uint LayerMask;

struct Trigger2D
{
	Collider2D* me;
	Collider2D* them;
	IntVec2 triggerID;

	Trigger2D GetInverse() const
	{
		Trigger2D trigger;
		trigger.me = them;
		trigger.them = me;
		trigger.triggerID = triggerID;
		return trigger;
	}
};

class Physics2D
{
public:
	void StartUp();
	void BeginFrame();
	void Update( float deltaSeconds );   
	void EndFrame();    // cleanup destroyed objects

	// factory style create/destroy
	Rigidbody2D* CreateRigidbody();
	void DestroyRigidbody( Rigidbody2D* rb );

	DiscCollider2D*		CreateDiscCollider( Vec2 localPosition, float radius );
	PolygonCollider2D*	CreatePolygonCollider( Vec2 localPosition, const std::vector<Vec2> points );

	void DestroyCollider( Collider2D* collider );
	void CleanupDestroyedObjects();

	void SimulationStep( float deltaSeconds );
	void ApplyEffectors( float deltaSeconds );
	void MoveRigidbodies( float deltaSeconds );
	void UpdateMass();
	void InformEventSystem(); // before resolving collisions... do pre-step

	Vec2 GetFrameAcceleration() const;

	// This is finding all manifolds and putting them in m_frameCollisions
	void DetectCollisions();
	void ResolveCollisions();
	void ResolveCollision( Collision2D const& collision );

	void CorrectObjectsInCollision( Collision2D const& collision );
	void CalculcateCollisionImpulse();
	void CalculateVerletVelocity();
	//Vec2 GetContactPoint( Collision2D const & collision, Collider2D const* collider );

	void SetClock( Clock* clock );
	void SetFixedDeltaTime( float fixedDeltaTime );
	void SetSceneGravity( Vec2 gravity );
	void SetFrameStartPos();

	// Layers
	void EnableLayerInteraction( uint layerIdx0, uint layerIdx1 );
	void DisableLayerInteraction( uint layerIdx0, uint layerIdx1 );
	bool DoLayersInteract( uint layerIdx0, uint layerIdx1 ) const; 

public:
	// storage for all rigidbodies
	std::vector<Rigidbody2D*> m_rigidBodies;

	// storage for all colliders
	std::vector<Collider2D*> m_colliders;

	// storage for all collisions
	std::vector<Collision2D> m_frameCollisions;
	std::vector<Collision2D> m_lastFrameCollisions;

	// storage for all triggers
	std::vector<Trigger2D> m_frameTriggers;
	std::vector<Trigger2D> m_lastFrameTriggers;

	// call once for every step of the physics system
	Delegate<float> OnFixedUpdate;

	Vec2 m_gravity = Vec2( 0.f, -30.f ); // In meters( 9.8 m/s^2 )
	Vec2 m_acceleration = Vec2::ZERO; 

	Clock* m_clock = nullptr;
	Timer* m_stepTimer = nullptr;

	float m_fixedDeltaTime = 1.f / 120.f;
	float m_drag = 0.f;

	uint m_layerInteractions[32] = {};
	LayerMask m_layerMask = 0xFFFFFFFF;
};
