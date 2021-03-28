#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void Rigidbody2D::UpdateFrameDeltaPos( float deltaSeconds )
{
	m_verletVelocity = (m_worldPosition - m_frameStartPos) / deltaSeconds;
}


void Rigidbody2D::Destroy()
{
	m_isGarbage = true;
	m_system->DestroyRigidbody( this );
}

void Rigidbody2D::TakeCollider( Collider2D* collider )
{
	if( m_collider != nullptr ) 
	{
		m_system->DestroyCollider( m_collider ); // #ToDo: Make sure destroy function is correct
		m_collider = nullptr;
	}

	m_collider = collider;

	if( m_collider )
	{
		m_collider->m_rigidbody = this;
		m_collider->UpdateWorldShape();
		CalculateMoment();
	}
}

void Rigidbody2D::SetPosition( Vec2 position )
{
	m_worldPosition = position;

	if( m_collider != nullptr ) {	
		m_collider->UpdateWorldShape();
	}

	//Initialize m_lastFramePos
	if( !m_isLastFramePosInitialize ) {
		m_lastFramePos = m_worldPosition;
		m_isLastFramePosInitialize = true;
	}
}

void Rigidbody2D::SetIsEnabled( const bool isEnabled )
{
	m_isEnabled = isEnabled;
}

void Rigidbody2D::SetVelocity( Vec2 velocity )
{
	m_velocity = velocity;
}

float Rigidbody2D::GetMomentOfInertia() const
{
	switch( m_simulatonMode ) {
	case SIMULATION_MODE_DYNAMIC: return m_moment;	break;
	case SIMULATION_MODE_KINEMATIC: 
	case SIMULATION_MODE_STATIC: 
	return std::numeric_limits<float>::infinity();	break;

	default: ERROR_AND_DIE( "Unknown simulation mode" ); break;
	}
}

void Rigidbody2D::SetSimulationMode( eSimulationMode simulationMode )
{
	m_simulatonMode = simulationMode;

	// Reset velocity of static objects
	if( m_simulatonMode == SIMULATION_MODE_STATIC ) {
		m_velocity = Vec2::ZERO;
	}

	switch( m_simulatonMode ) {
	case SIMULATION_MODE_DYNAMIC: m_collider->m_mass = 1.f;	break;
	case SIMULATION_MODE_KINEMATIC:
	case SIMULATION_MODE_STATIC:
		m_collider->m_mass = std::numeric_limits<float>::infinity();	break;

	default: ERROR_AND_DIE( "Unknown simulation mode" ); break;
	}
}

void Rigidbody2D::SetLayer( uint layerIndex )
{
	if( layerIndex < 0 || layerIndex > 31 ) {
		ERROR_AND_DIE("Invalid layer index!");
	}
	m_layerIndex = layerIndex;
}

void Rigidbody2D::ApplyDragForce()
{
	Vec2 velocity = GetVelocity();
	Vec2 dragForce = -velocity * m_drag;
	AddForce( dragForce );
}

void Rigidbody2D::AddForce( Vec2 force )
{
	if( m_collider == nullptr ) {
		return; // Mass is on the collider 
	}

	if( m_collider->GetMass() > 0.f )
	{
		Vec2 acc = force / m_collider->GetMass();
		m_velocity += acc * m_system->m_fixedDeltaTime;
	}
}



void Rigidbody2D::ApplyImpulseAt( Vec2 point, Vec2 impulse )
{
	m_velocity += m_collider->GetInverseMass() * impulse;

	Vec2 disp = point - m_worldPosition; 
	
	Vec2 tan = disp.GetRotated90Degrees();
	float impulseTorque = DotProduct2D( impulse, tan ); 

	m_angularVelocity += impulseTorque / m_moment;
	//m_angularVelocity += CrossProduct2D( disp, impulse ) / GetMomentOfInertia();
}

float Rigidbody2D::GetMass() const
{
	return m_collider->GetMass();
}

void Rigidbody2D::CalculateMoment()
{
	if( m_collider != nullptr )
	{
		m_moment = m_collider->Calculatemoment( GetMass() );
	}
}

Vec2 Rigidbody2D::GetImpactVelocity( const Vec2& r )
{
	Vec2 tangent = r.GetRotated90Degrees();

	Vec2 rotationalVelocity = tangent * m_angularVelocity;

	Vec2 impactVelocity = m_velocity + rotationalVelocity;

	return impactVelocity;
}

float Rigidbody2D::GetOrientationDegrees() const 
{
	float degrees = ConvertRadiansToDegrees( m_rotationInRadians );

	return (float)( (int)degrees % 360 ) + ( (int)degrees < 0 ? 360 : 0 );
}

Rigidbody2D::~Rigidbody2D()
{ 
	GUARANTEE_OR_DIE( m_collider == nullptr, "Failed! collider isn't destoryed" ); 
}

void Rigidbody2D::SetUserData( uint type, void* data )
{
	m_userDataType = type;
	m_userData = data;
}
