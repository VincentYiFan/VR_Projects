#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"

//------------------------------------------------------------------------------------
struct Vec2;

//------------------------------------------------------------------------------------
class Transform
{
public:
	void SetPosition( Vec3 pos );
	void SetScale( Vec3 scale );
	void SetYawDegrees( float yawDegrees );
	void SetRotationFromPitchRollYawDegrees( float pitch, float roll, float yaw );
	void AddRotationFromPitchRollYawDegrees( float pitch, float roll, float yaw );
	void Translate( Vec3 translation );

	Mat44 ToMatrix() const;
	Mat44 GetRotationMatrix() const;
	Vec2  GetPosition2D() const;

public:
	Vec3 m_position                     = Vec3::ZERO;
	Vec3 m_scale                        = Vec3( 1.f, 1.f, 1.f );

	float m_orientationPitch = 0.f;
	float m_orientationRoll = 0.f;
	float m_orientationYaw = 0.f;
};
