#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include <algorithm>


void Physics2D::StartUp()
{
	m_clock = new Clock( Clock::GetMaster() );
	m_stepTimer = new Timer();

	//m_stepTimer = new Timer();
	m_stepTimer->SetSeconds( m_clock, m_fixedDeltaTime );

	// Init collision layer matrix
	for( int i = 0; i < 32; ++i )
	{
		m_layerInteractions[i] = 0xFFFFFFFF;
	}

	DisableLayerInteraction( 0, 2 );
	//EnableLayerInteraction( 0, 2 );
}

void Physics2D::BeginFrame()
{
}

void Physics2D::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	while( m_stepTimer->CheckAndDecrement() ) {
		SimulationStep( m_fixedDeltaTime );
		UpdateMass();
	}
}

void Physics2D::EndFrame()
{
	CleanupDestroyedObjects();
}

Rigidbody2D* Physics2D::CreateRigidbody()
{
	Rigidbody2D* rb = new Rigidbody2D();
	rb->m_system = this;
	m_rigidBodies.push_back( rb );

	return rb;
}

void Physics2D::DestroyRigidbody( Rigidbody2D* rb )
{
	Collider2D* collider = rb->GetCollider();
	if( collider != nullptr ) {
		rb->TakeCollider( nullptr );
		collider->Destroy();
	}
}

DiscCollider2D* Physics2D::CreateDiscCollider( Vec2 localPosition, float radius )
{
	DiscCollider2D* discCollider = new DiscCollider2D();
	discCollider->m_localPosition = localPosition;
	discCollider->m_radius = radius;
	discCollider->m_type = COLLIDER2D_DISC;
	discCollider->m_system = this;

	// set colliderID
	int id = (int)m_colliders.size();
	discCollider->SetColliderID( id ); 
	m_colliders.push_back( discCollider );

	return discCollider;
}

PolygonCollider2D* Physics2D::CreatePolygonCollider( Vec2 localPosition, const std::vector<Vec2> points )
{
	PolygonCollider2D* polygonCollider = new PolygonCollider2D( points );

	polygonCollider->m_localPosition = localPosition;
	polygonCollider->m_type = COLLIDER2D_POLYGON;
	polygonCollider->m_system = this;

	// set colliderID
	int id = (int)m_colliders.size();
	polygonCollider->SetColliderID( id );
	m_colliders.push_back( polygonCollider );

	return polygonCollider;
}


void Physics2D::DestroyCollider( Collider2D* collider )
{
	collider->Destroy();
}

void Physics2D::SimulationStep( float deltaSeconds )
{
	SetFrameStartPos();
	ApplyEffectors( deltaSeconds ); 	// apply gravity to all dynamic objects
	MoveRigidbodies( deltaSeconds ); 	// apply an euler step to all rigidbodies, and reset per-frame data
	DetectCollisions();					// determine all pairs of intersecting colliders
	InformEventSystem();
	ResolveCollisions();				// resolve all collisions, firing appropraite events
	CalculateVerletVelocity();
}


void Physics2D::ApplyEffectors( float deltaSeconds )
{
	for( int rbIndex = 0; rbIndex < (int)m_rigidBodies.size(); rbIndex++ )
	{
		if( m_rigidBodies[rbIndex] != nullptr && m_rigidBodies[rbIndex]->m_simulatonMode == SIMULATION_MODE_DYNAMIC && m_rigidBodies[rbIndex]->m_isEnabled )
		{
			m_rigidBodies[rbIndex]->m_velocity += m_gravity * deltaSeconds;
			m_rigidBodies[rbIndex]->ApplyDragForce();
			
			if( m_rigidBodies[rbIndex]->GetMomentOfInertia() > 0.f )
			{
				m_rigidBodies[rbIndex]->m_angularVelocity += m_rigidBodies[rbIndex]->m_frameTorque * (1.f / m_rigidBodies[rbIndex]->GetMomentOfInertia()) * deltaSeconds; 
				//m_rigidBodies[rbIndex]->m_orientationDegrees += m_rigidBodies[rbIndex]->m_angularVelocity * deltaSeconds;
				m_rigidBodies[rbIndex]->m_rotationInRadians += m_rigidBodies[rbIndex]->m_angularVelocity * deltaSeconds; 
			}

			m_rigidBodies[rbIndex]->m_worldPosition += m_rigidBodies[rbIndex]->m_velocity * deltaSeconds;
			m_rigidBodies[rbIndex]->m_collider->UpdateWorldShape();
		}
	}
}

void Physics2D::MoveRigidbodies( float deltaSeconds )
{
	for( int rbIndex = 0; rbIndex < (int)m_rigidBodies.size(); rbIndex++ )
	{
		if( m_rigidBodies[rbIndex] != nullptr && m_rigidBodies[rbIndex]->m_simulatonMode == SIMULATION_MODE_KINEMATIC && m_rigidBodies[rbIndex]->m_isEnabled )
		{
			/*m_rigidBodies[rbIndex]->m_velocity += m_acceleration * deltaSeconds; */

			m_rigidBodies[rbIndex]->m_worldPosition += m_rigidBodies[rbIndex]->m_velocity * deltaSeconds;
			m_rigidBodies[rbIndex]->m_collider->UpdateWorldShape();
		}
	}
}


void Physics2D::UpdateMass()
{
	for( int idx = 0; idx < (int)m_colliders.size(); ++idx )
	{
		if( m_colliders[idx] == nullptr ) {
			return;
		}

		switch( m_colliders[idx]->m_rigidbody->m_simulatonMode ) {
		case		SIMULATION_MODE_DYNAMIC:	/*m_colliders[idx]->m_mass = 1.f;*/		break;
		case		SIMULATION_MODE_KINEMATIC:	
		case		SIMULATION_MODE_STATIC:			
													m_colliders[idx]->m_mass = 999999.f; break;
		default:	ERROR_AND_DIE("Physics UpdateMass: Wrong simulation type");			 break;
		}
	}
}

void Physics2D::InformEventSystem()
{
	// call OnCollision Events
	for( Collision2D col : m_frameCollisions )
	{
		bool isCollisionNew = true;
		Collision2D invCol = col.GetInverse();

		// compare collision ID 
		for( Collision2D colLastFrame : m_lastFrameCollisions ) {
			if( col.GetID() == colLastFrame.GetID() ) {
				isCollisionNew = false;
			}
		}

		// call events on rigidbodies 
		if( isCollisionNew ) {
			col.me->m_rigidbody->OnOverlapEnter( col );
			col.them->m_rigidbody->OnOverlapEnter( invCol );
		}
		else {
			col.me->m_rigidbody->OnOverlapStay( col );
			col.them->m_rigidbody->OnOverlapStay( invCol );
		}
	}

	for( Collision2D lastCol : m_lastFrameCollisions )
	{
		bool isCollisionOld = true;
		Collision2D invCol = lastCol.GetInverse();

		// compare collision ID 
		for( Collision2D col : m_frameCollisions ) {
			if( lastCol.GetID() == col.GetID() ) {
				isCollisionOld = false;
			}
		}

		// call events on rigidbodies 
		if( isCollisionOld ) {
			lastCol.me->m_rigidbody->OnOverlapExit( lastCol );
			lastCol.them->m_rigidbody->OnOverlapExit( invCol );
		}
	}

	// call OnTrigger events
	for( Trigger2D trigger : m_frameTriggers )
	{
		bool isTriggerNew = true;
		Trigger2D invTrigger = trigger.GetInverse();

		// compare collision ID 
		for( Trigger2D triggerLastFrame : m_lastFrameTriggers ) {
			if( trigger.triggerID == triggerLastFrame.triggerID ) {
				isTriggerNew = false;
			}
		}

		// call events on colliders 
		if( isTriggerNew ) {
			trigger.me->OnTriggerEnter( trigger );
			trigger.them->OnTriggerEnter( invTrigger );
		}
		else {
			trigger.me->OnTriggerStay( trigger );
			trigger.them->OnTriggerStay( invTrigger );
		}
	}

	for( Trigger2D lastFrameTrigger : m_lastFrameTriggers )
	{
		bool isTriggerOld = true;
		Trigger2D invTrigger = lastFrameTrigger.GetInverse();

		// compare collision ID 
		for( Trigger2D trigger : m_frameTriggers ) {
			if( lastFrameTrigger.triggerID == trigger.triggerID ) {
				isTriggerOld = false;
			}
		}

		// call events on rigidbodies 
		if( isTriggerOld ) {
			lastFrameTrigger.me->OnTriggerExit( lastFrameTrigger );
			lastFrameTrigger.them->OnTriggerExit( invTrigger );
		}
	}
}


void Physics2D::SetSceneGravity( Vec2 gravity )
{
	m_gravity = gravity;
}


void Physics2D::CleanupDestroyedObjects()
{
	// cleanup destroyed objects

	for( int rbIndex = 0; rbIndex < (int)m_rigidBodies.size(); rbIndex++ )
	{
		if( m_rigidBodies[rbIndex] && m_rigidBodies[rbIndex]->m_isGarbage )
		{
			delete m_rigidBodies[rbIndex];
			m_rigidBodies[rbIndex] = nullptr;
		}
	}

	for( int colliderIndex = 0; colliderIndex < (int)m_colliders.size(); colliderIndex++ )
	{
		if( m_colliders[colliderIndex] && m_colliders[colliderIndex]->m_isGarbage )
		{
			delete m_colliders[colliderIndex];
			m_colliders[colliderIndex] = nullptr;
		}
	}
}

Vec2 Physics2D::GetFrameAcceleration() const
{
	return m_acceleration;
}

void Physics2D::DetectCollisions()
{
	// For loop through colliders and detect if there's collision
	// Skip collisions between static objects
	for( int i = 0; i < (int)m_colliders.size(); ++i ) {
		for( int j = 0; j < (int)m_colliders.size(); ++j ) {
			if( (m_colliders[i] != nullptr) && (m_colliders[j] != nullptr) 
				&& ( m_colliders[i] != m_colliders[j] ) && m_colliders[i]->Intersects( m_colliders[j] ) ) {

				bool isBothStatic = m_colliders[i]->m_rigidbody->m_simulatonMode == SIMULATION_MODE_STATIC &&
									m_colliders[j]->m_rigidbody->m_simulatonMode == SIMULATION_MODE_STATIC;

				bool isBothCollider = (m_colliders[i]->m_isTrigger == false) && (m_colliders[j]->m_isTrigger == false);

				bool doLayersInteract = DoLayersInteract( m_colliders[i]->m_rigidbody->GetLayer(), m_colliders[j]->m_rigidbody->GetLayer() );
				
				// collider vs. collider
				if( !isBothStatic && isBothCollider && doLayersInteract ) 
				{
					Collision2D collision;
					collision.me = m_colliders[i];
					collision.them = m_colliders[j];
					collision.manifold = m_colliders[i]->GetManifold( m_colliders[j] );
					collision.m_collisionID = IntVec2( min( collision.me->GetColliderID(), collision.them->GetColliderID() ),
													   max( collision.me->GetColliderID(), collision.them->GetColliderID() ) );

					if( (int)m_frameCollisions.size() < 1 ) {
						m_frameCollisions.push_back( collision ); }
					else {
						// Avoid same collision added twice
						for( int collisionIdx = 0; collisionIdx < (int)m_frameCollisions.size(); ++collisionIdx ) {
							if( (collision.me != m_frameCollisions[collisionIdx].them && collision.them != m_frameCollisions[collisionIdx].me )
							 && (collision.me != m_frameCollisions[collisionIdx].me   && collision.them != m_frameCollisions[collisionIdx].them) )
							{
								m_frameCollisions.push_back( collision );
							}
						}
					}		
				}

				// trigger vs. trigger, trigger vs. collider, collider vs. trigger
				if( isBothCollider == false && doLayersInteract ) 
				{
					Trigger2D trigger;
					trigger.me = m_colliders[i];
					trigger.them = m_colliders[j];
					trigger.triggerID = IntVec2( min(trigger.me->GetColliderID(), trigger.them->GetColliderID() ),
												 max(trigger.me->GetColliderID(), trigger.them->GetColliderID() ) );

					m_frameTriggers.push_back( trigger );
				}
			}
		}
	}
}

void Physics2D::CorrectObjectsInCollision( Collision2D const& collision )
{
	float myMass = collision.me->GetMass();
	float theirMass = collision.them->GetMass();
	float pushMe = theirMass / (myMass + theirMass);
	float pushThem = 1.f - pushMe;

	eSimulationMode& myMode = collision.me->m_rigidbody->m_simulatonMode;
	eSimulationMode& theirMode = collision.them->m_rigidbody->m_simulatonMode;

	// Dynamic vs Dynamic (push each other)
	// Kinematic vs Kinematic (push each other)
	if( (myMode == SIMULATION_MODE_DYNAMIC   && theirMode == SIMULATION_MODE_DYNAMIC) ||
		(myMode == SIMULATION_MODE_KINEMATIC && theirMode == SIMULATION_MODE_KINEMATIC) )
	{
		//Vec2 normal = collision.GetNormal(); 
		//float pen = collision.GetPeneration(); 
		collision.me->Move( pushMe * collision.GetNormal() * collision.GetPeneration() * 0.5f );
		collision.them->Move( -pushThem * collision.GetNormal() * collision.GetPeneration() * 0.5f );

		//const float k_slop = 0.01f; // Penetration allowance
		//const float percent = 0.2f; // Penetration percentage to correct
		//Vec2 correction = (std::max( pen - k_slop, 0.f ) / (collision.me->GetInverseMass() + collision.them->GetInverseMass() ) ) * percent * normal; 
		//collision.me->m_rigidbody->m_worldPosition -= collision.me->GetInverseMass() * correction;
		//collision.them->m_rigidbody->m_worldPosition += collision.them->GetInverseMass() * correction;
	}

	// Dynamics vs (Kinematic || Static) -> Only push dyanmic 100%
	if( myMode == SIMULATION_MODE_DYNAMIC && (theirMode == SIMULATION_MODE_KINEMATIC || theirMode == SIMULATION_MODE_STATIC) )
	{
		collision.me->Move( collision.GetNormal() * collision.GetPeneration() );
	}

	// (Kinematic || Static) vs Dynamics
	if( (myMode == SIMULATION_MODE_KINEMATIC || myMode == SIMULATION_MODE_STATIC)
		&& theirMode == SIMULATION_MODE_DYNAMIC )
	{
		collision.them->Move( -collision.GetNormal() * collision.GetPeneration() );
	}

	// Kinematic vs Static -> Only push kinematic 100%
	if( myMode == SIMULATION_MODE_KINEMATIC &&
		theirMode == SIMULATION_MODE_STATIC )
	{
		collision.me->Move( collision.GetNormal() * collision.GetPeneration() );
	}

	// Static vs Kinematic 
	if( myMode == SIMULATION_MODE_STATIC &&
		theirMode == SIMULATION_MODE_KINEMATIC )
	{
		collision.them->Move( -collision.GetNormal() * collision.GetPeneration() );
	}
}

void Physics2D::CalculcateCollisionImpulse()
{
	//                                    j = -(1 + epsilon) * N dot Vrel
	// -----------------------------------------------------------------------------------------------------
	//        invM_1 + invM_2 + ((rA cross n)^2 / InertiaTensorA) + ((rB cross n)^2 / InertiaTensorB)

	for( int collisionIndex = 0; collisionIndex < (int)m_frameCollisions.size(); ++collisionIndex )
	{
		Collision2D& col = m_frameCollisions[collisionIndex];
		Vec2 contactPointMe = col.GetContactPoint( col.me );
		Vec2 contactPointThem = col.GetContactPoint( col.them );

		g_theDebugRenderSystem->DebugAddScreenPoint( contactPointMe, 1.f, Rgba8::MAGENTA, 0.1f );
		g_theDebugRenderSystem->DebugAddScreenPoint( contactPointThem, 1.f, Rgba8::MAGENTA, 0.1f );

		Vec2 myImpactVelocity = col.me->m_rigidbody->GetImpactVelocity( contactPointMe - col.me->m_rigidbody->m_worldPosition );
		Vec2 theirImpactVelocity = col.them->m_rigidbody->GetImpactVelocity( contactPointThem - col.them->m_rigidbody->m_worldPosition );
		Vec2 relativeVelocity = theirImpactVelocity - myImpactVelocity;
		Vec2 normal = col.GetNormal();
		float contactVel = DotProduct2D( relativeVelocity, normal );

		// DebugRender
		//g_theDebugRenderSystem->DebugAddScreenPoint( contactPoint, 2.f,Rgba8::RED, 0.1f );

		// Do not resolve if velocities are separating
		if( contactVel <= 0 ) {
			return; }

		// Calculate restitution
		float e = col.me->GetBounceWith( col.them );

		// Calculate (rA cross n)^2 / InertiaTensorA
		Vec2 rA = (contactPointMe - col.me->m_rigidbody->m_worldPosition).GetRotated90Degrees();
		float a = (DotProduct2D( rA, normal ) * DotProduct2D( rA, normal )) / col.me->m_rigidbody->GetMomentOfInertia();


		// Calculate (rB cross n)^2 / InertiaTensorB
		Vec2 rB = (contactPointThem - col.them->m_rigidbody->m_worldPosition).GetRotated90Degrees();
		float b = (DotProduct2D( rB, normal ) * DotProduct2D( rB, normal )) / col.them->m_rigidbody->GetMomentOfInertia();


		// Calculate impulse scalar
		float j = -( 1.f + e ) * contactVel;
		j /= ( col.me->GetInverseMass() + col.them->GetInverseMass() + a + b );

		
		//Vec2 tangent = relativeVelocity - DotProduct2D( relativeVelocity, normal ) *  normal;
		Vec2 tangent = normal.GetRotated90Degrees();
		tangent.Normalize();

	
		float c = ( DotProduct2D( rA, tangent ) * DotProduct2D( rA, tangent ) ) / col.me->m_rigidbody->GetMomentOfInertia();
		float d = ( DotProduct2D( rB, tangent ) * DotProduct2D( rB, tangent ) ) / col.them->m_rigidbody->GetMomentOfInertia();

		float jt = -( 1.f + e ) * DotProduct2D( relativeVelocity, tangent );
		jt /= ( col.me->GetInverseMass() + col.them->GetInverseMass() + c + d );

		float friction = col.me->GetFrictionWith( col.them );

		// Coulomb's Law: the force of friction is always less than or equal to the normal force multiplied by some constant 
		if( fabsf( jt ) > j * friction ) {
			jt = -SignFloat( jt ) * j * friction;
		}
	

		// Apply impulse
		Vec2 impulse = j * normal + jt * tangent;

		if( col.me->m_rigidbody->m_simulatonMode == SIMULATION_MODE_DYNAMIC ) {
			col.me->m_rigidbody->ApplyImpulseAt( contactPointMe, -impulse );
		}
		if( col.them->m_rigidbody->m_simulatonMode == SIMULATION_MODE_DYNAMIC ) {
			col.them->m_rigidbody->ApplyImpulseAt( contactPointThem, impulse );
		}
	}
}


void Physics2D::ResolveCollisions()
{
	for( int colIdx = 0; colIdx < m_frameCollisions.size(); ++colIdx )
	{
		ResolveCollision( m_frameCollisions[colIdx] );
	}
	// copy all the collisions into last frame collisions
	m_lastFrameCollisions = m_frameCollisions;

	// copy all the triggers into last frame triggers
	m_lastFrameTriggers = m_frameTriggers;

	m_frameCollisions.clear();
	m_frameTriggers.clear();
}

void Physics2D::ResolveCollision( Collision2D const& collision )
{
	CorrectObjectsInCollision( collision );
	CalculcateCollisionImpulse();
}


void Physics2D::SetClock( Clock* clock )
{
	m_clock = clock;
}

void Physics2D::SetFixedDeltaTime( float fixedDeltaTime )
{
	m_fixedDeltaTime = fixedDeltaTime;
}

void Physics2D::SetFrameStartPos()
{
	for( int i = 0; i < (int)m_rigidBodies.size(); ++i )
	{
		if( m_rigidBodies[i] && m_fixedDeltaTime > 0.f )
		{
			m_rigidBodies[i]->m_frameStartPos = m_rigidBodies[i]->m_worldPosition;
		}
	}
}

void Physics2D::EnableLayerInteraction( uint layerIdx0, uint layerIdx1 )
{
	m_layerInteractions[layerIdx0] |= (1 << layerIdx1);
	m_layerInteractions[layerIdx1] |= (1 << layerIdx0);
}

void Physics2D::DisableLayerInteraction( uint layerIdx0, uint layerIdx1 )
{
	m_layerInteractions[layerIdx0] = ~(1 << layerIdx1);
	m_layerInteractions[layerIdx1] = ~(1 << layerIdx0);
}

bool Physics2D::DoLayersInteract( uint layerIdx0, uint layerIdx1 ) const
{
	return (m_layerInteractions[layerIdx0] & (1 << layerIdx1) ) != 0; 
}

void Physics2D::CalculateVerletVelocity()
{
	for( int i = 0; i < (int)m_rigidBodies.size(); ++i )
	{
		if( m_rigidBodies[i] && m_fixedDeltaTime > 0.f )
		{
			m_rigidBodies[i]->UpdateFrameDeltaPos( m_fixedDeltaTime );
		}
	}
}

