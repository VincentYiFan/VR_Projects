#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/Delegate.hpp"
//----------------------------------------------------------------------------------------------------------------------------
class RenderContext;
class Rigidbody2D;
struct Rgba8;
struct Vec2;
//----------------------------------------------------------------------------------------------------------------------------

enum eCollider2DType
{
	COLLIDER2D_DISC,
	COLLIDER2D_POLYGON,

	NUM_COLLIDER2D_TYPES
};

struct PhysicsMaterial
{
	float restitution = 1.f;
	float friction = 0.1f;
};

// Interface for all Collider objects used with our Physics system
class Collider2D
{
	friend class Physics2D;

public: // Interface 
		// cache off the world shape representation of this object
		// taking into account the owning rigidbody (if no owner, local is world)
	virtual void UpdateWorldShape()                             = 0;

	// queries 
	virtual AABB2 GetWorldBounds() const						= 0;
	virtual Vec2 GetClosestPoint( Vec2 pos ) const              = 0;
	virtual bool Contains( Vec2 pos ) const                     = 0;
	virtual void Destroy()	= 0;
	virtual float Calculatemoment( float mass )					= 0;

	bool Intersects( Collider2D const* other ) const;

	// Translate function
	void Move( Vec2 translation );

	// trigger events
	Delegate<Trigger2D const&> OnTriggerEnter;
	Delegate<Trigger2D const&> OnTriggerStay;
	Delegate<Trigger2D const&> OnTriggerExit;
	
	// getters
	Manifold2		GetManifold( Collider2D const* other ) const;
	eCollider2DType GetType() const { return m_type; }
	float			GetMass() const;
	float			GetInverseMass() const { return 1.f / m_mass; }
	float			GetBounceWith( Collider2D const* other );
	float			GetFrictionWith( Collider2D const* other );
	int				GetColliderID() const { return m_colliderID; }
	bool			IsTrigger() { return m_isTrigger; }

	// setters
	void			SetMass( float newMass );
	void			SetColliderID( int id );
	void			SetColliderIsTrigger( bool isTrigger );

	// debug helpers
	virtual void DebugRender( RenderContext* ctx, Rgba8 const& borderColor, Rgba8 const& fillColor ) = 0;


	// Mid-phase check
	static bool DoesAABB2sOverlap( AABB2 a, AABB2 b );

protected:
	virtual ~Collider2D(); // private - make sure this is virtual so correct deconstructor gets called

public:
	eCollider2DType m_type;                  // keep track of the type - will help with collision later
	Physics2D* m_system = nullptr;                   // system who created or destroy
	Rigidbody2D* m_rigidbody = nullptr;    // owning rigidbody, used for calculating world shape
	PhysicsMaterial m_material;

	bool m_isGarbage = false;
	bool m_isTrigger = false;

	float m_mass = 1.f;
	int m_colliderID = -1;

	//NamedProperties m_userProperties;
};
