#pragma once

//-----------------------------------------------------------------------------------------------
struct Vec4
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

public:
	// Construction/Destruction
	~Vec4() {}												// destructor (do nothing)
	Vec4() {}												// default constructor (do nothing)
	Vec4( const Vec4& copyFrom );							// copy constructor (from another vec3)
	explicit Vec4( float initialX, float initialY, float initialZ, float initialW );		// explicit constructor (from x, y, z, w)


																			// Accessors (const methods)
	//float GetLength() const;
	//float GetLengthXY() const;
	//float GetLengthSquared() const;
	//float GetLengthXYSquared() const;
	//float GetAngleAboutZRadians() const;
	//float GetAngleAboutZDegrees() const;
	//const Vec4 GetRotatedAboutZRadians( float deltaRadians ) const;
	//const Vec4 GetRotatedAboutZDegrees( float deltaDegrees ) const;
	//const Vec4 GetClamped( float maxLength ) const;
	//const Vec4 GetNormalized() const;


	// Operators (const)
	//bool		operator==( const Vec4& compare ) const;		
	//bool		operator!=( const Vec4& compare ) const;		
	//const Vec4	operator+( const Vec4& vecToAdd ) const;		
	//const Vec4	operator-( const Vec4& vecToSubtract ) const;	
	//const Vec4	operator-() const;								
	//const Vec4	operator*( float uniformScale ) const;			
	const Vec4	operator*( const Vec4& vecToMultiply ) const;	
	//const Vec4	operator/( float inverseScale ) const;			


	//void		operator+=( const Vec4& vecToAdd );
	//void		operator-=( const Vec4& vecToSubtract );
	//void		operator*=( const float uniformScale );
	void		operator/=( const float uniformDivisor );
	//void		operator=( const Vec4& copyFrom );

	//															// Standalone "friend" functions that are conceptually, but not actually, part of Vec3::
	//friend const Vec4 operator*( float uniformScale, const Vec4& vecToScale );	// float * vec4
};
