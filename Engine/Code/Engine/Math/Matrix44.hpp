#pragma once

////-----------------------------------------------------------------------------------------------
//struct Vec2;
//struct Vec3;
////struct Vec4;
//
////-----------------------------------------------------------------------------------------------
//struct Mat44
//{
//	float m_values[ 16 ];
//	enum { Ix, Iy, Iz, Iw,  Jx, Jy, Jz, Jw,  Kx, Ky, Kz, Kw,  Tx, Ty, Tz, Tw };
//
//	const static Mat44 IDENTITY; // const STATIC Mat44 Mat44::IDENTITY;
//
//public:
//	// Construction methods
//	Mat44();
//	explicit Mat44( float* sixteenValuesVasisMajor );
//	explicit Mat44( const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& translation2D );
//	explicit Mat44( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& translation3D );
//	//explicit Mat44( const Vec4& iBasisHomogeneous, const Vec4& iBasisHomogeneous, const Vec4& kBasisHomogeneous, const Vec4& translationHomogeneous );
//
//	const Vec2	TransformVector2D( const Vec2& vectorQuantity ) const; // Assume z=0, w=0
//	const Vec3	TransformVector3D( const Vec3& vectorQuantity ) const; // Assume w=0
//
//	// Basic accessors
//	const float*	GetAsFloatArray() const		{ return &m_values[0]; }
//	float*			GetAsFloatArray()			{ return &m_values[0]; }
//	const Vec2		GetIBasis2D() const;	// Could also return const Vec2
//	const Vec2		GetJBasis2D() const;
//	const Vec3&		GetTranslation3D() const;
//
//	void TransformBy( const Mat44& arbitraryTransformationToAppend );
//
//
//	// Static creation methods to create a matrix of a certain transformation type
//	static const Mat44		CreateXRotationDegrees( float degreesAboutX );
//	static const Mat44		CreateYRotationDegrees( float degrresAnoutY );
//	static const Mat44		CreateZRotationDegrees( float degrresAnoutZ );
//	static const Mat44		CreateTranslationXY( const Vec2& translationXY );
//	static const Mat44		CreateTranslation3D( const Vec3& translation3D );
//	static const Mat44		CreateUniformScaleXY( float uniformScaleXY );
//	static const Mat44		CreateNonUniformScaleXY( const Vec2& scaleFctorsXY );
//	//static const Mat44		
//	//static const Mat44	
//
//private:
//	const Mat44 operator*( const Mat44& rhs ) const = delete;
//};
