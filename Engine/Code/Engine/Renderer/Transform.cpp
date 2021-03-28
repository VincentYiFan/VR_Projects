#include "Engine/Renderer/Transform.hpp"
#include "Engine/Math/Vec2.hpp"


void Transform::SetPosition( Vec3 pos )
{
	m_position = pos;
}

void Transform::SetRotationFromPitchRollYawDegrees( float pitch, float roll, float yaw )
{
	//m_rotationPitchRollYawDegrees = Vec3( pitch, roll, yaw );
	m_orientationPitch = pitch;
	m_orientationRoll = roll;
	m_orientationYaw = yaw;
}

void Transform::AddRotationFromPitchRollYawDegrees( float pitch, float roll, float yaw )
{
	//m_rotationPitchRollYawDegrees += Vec3( pitch, roll, yaw );
	m_orientationPitch += pitch;
	m_orientationRoll += roll;
	m_orientationYaw += yaw;
}


void Transform::SetScale( Vec3 scale )
{
	m_scale = scale;
}

void Transform::SetYawDegrees( float yawDegrees )
{
	m_orientationYaw = yawDegrees;
}

void Transform::Translate( Vec3 translation )
{
	m_position += translation;
}

Mat44 Transform::ToMatrix() const
{
	Mat44 mat;
	mat.TransformBy( Mat44::CreateTranslation3D( m_position ) );									 
	mat.TransformBy( Mat44::FromPitchRollYaw( Vec3( m_orientationPitch, m_orientationRoll, m_orientationYaw ) ) );					 
	mat.ScaleNonUniform3D( m_scale );
	return mat;
}

Mat44 Transform::GetRotationMatrix() const
{
	Mat44 mat;
	mat.TransformBy( Mat44::FromPitchRollYaw( Vec3( m_orientationPitch, m_orientationRoll, m_orientationYaw ) ) );
	return mat;
}

Vec2 Transform::GetPosition2D() const
{
	return Vec2( m_position.x, m_position.y );
}
