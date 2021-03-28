#include "Engine/Physics/GameObject.hpp"


GameObject::~GameObject()
{
	if( m_mesh )
	{
		delete m_mesh;
		m_mesh = nullptr;
	}

	// delete m_rigidbody; // this should be an error if setup right
	if( m_rigidbody )
	{
		m_rigidbody->Destroy(); // destroys through the system;  
		m_rigidbody = nullptr;
	}
	
}

void GameObject::SetMesh( GPUMesh* mesh ) 
{
	m_mesh = mesh;
}