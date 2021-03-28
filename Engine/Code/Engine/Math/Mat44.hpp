#pragma once

//-----------------------------------------------------------------------------------------------
struct Vec2;
struct Vec3;
struct Vec4;
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------

struct Mat44
{
	float  Ix = 1.f;
	float  Iy = 0.f;
	float  Iz = 0.f;
	float  Iw = 0.f;
	float  Jx = 0.f;
	float  Jy = 1.f;
	float  Jz = 0.f;
	float  Jw = 0.f;
	float  Kx = 0.f;
	float  Ky = 0.f;
	float  Kz = 1.f;
	float  Kw = 0.f;
	float  Tx = 0.f;
	float  Ty = 0.f;
	float  Tz = 0.f;
	float  Tw = 1.f;

	const static Mat44 IDENTITY; // const STATIC Mat44 Mat44::IDENTITY;

public:
	// Construction methods
	Mat44() = default; // Default constructor sets matrix to identity
	explicit Mat44( float const* sixteenValuesBasisMajor );
	explicit Mat44( const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& translation2D );
	explicit Mat44( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& translation3D );
	explicit Mat44( const Vec4& iBasisHomogeneous, const Vec4& jBasisHomogeneous, const Vec4& kBasisHomogeneous, const Vec4& translationHomogeneous );

	// Transforming positions & vector quantities using this matrix
	const Vec2	TransformVector2D( const Vec2& vectorQuantity ) const;  // Assume z=0, w=0
	const Vec3	TransformVector3D( const Vec3& vectorQuantity ) const;  // Assume w=0
	const Vec2	TransformPosition2D( const Vec2& position )	const;		// Assume z=0, w=1
	const Vec3	TransformPosition3D( const Vec3& position ) const;		// Assume w=1
	const Vec4	TransformHomogeneousPoint3D( const Vec4& homogeneousPoint ) const; // explicit w=0, 1, or other 

	// Basic accessors
	const float*	GetAsFloatArray() const		{ return &Ix; }
	float*			GetAsFloatArray()			{ return &Ix; }
	const Vec2		GetIBasis2D() const;
	const Vec2		GetJBasis2D() const;
	const Vec2		GetTranslation2D() const;
	const Vec3		GetIBasis3D() const;
	const Vec3		GetJBasis3D() const;
	const Vec3		GetKBasis3D() const;
	const Vec3		GetTranslation3D() const;
	const Vec4		GetIBasis4D() const;
	const Vec4		GetJBasis4D() const;
	const Vec4		GetKBasis4D() const;
	const Vec4		GetTranslation4D() const;

	// Basic mutators
	void SetTranslation2D( const Vec2& translation2D );
	void SetTranslation3D( const Vec3& translation3D );
	void SetBasisVectors2D( const Vec2& iBasis2D, const Vec2& jBasis2D );
	void SetBasisVectors2D( const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& tranlation2D );
	void SetBasisVectors3D( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D );
	void SetBasisVectors3D( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& tranlation3D);
	void SetBasisVectors4D( const Vec4& iBasis4D, const Vec4& jBasis4D, const Vec4& kBasis4D, const Vec4& tranlation4D);
	
	// Transformation mutators; append (concatenate) a new transform to this matrix 
	void RotateXDegrees( float degreesAboutX ); // PushRotateByX
	void RotateYDegrees( float degreesAboutY );
	void RotateZDegrees( float degreesAboutZ );
	void Translate2D( const Vec2& translation2D );
	void Translate3D( const Vec3& translation3D );
	void ScaleUniform2D( float uniformScaleXY ); // why this not const float?
	void ScaleNonUniform2D( const Vec2& scaleFactorsXY );
	void ScaleUniform3D( float uniformScaleXYZ );
	void ScaleNonUniform3D( const Vec3& scaleFactorsXYZ );
	void TransformBy( const Mat44& arbitraryTransformationToAppend );
	// void PushMatrix 

	// Static creation methods to create a matrix of a certain transformation type
	static const Mat44		CreateXRotationDegrees( float degreesAboutX );
	static const Mat44		CreateYRotationDegrees( float degreesAboutY );
	static const Mat44		CreateZRotationDegrees( float degreesAboutZ );
	static const Mat44		CreateRotationAroundAxis( float degressAboutAxis, Vec3 axis );
	static const Mat44		CreateTranslationXY( const Vec2& translationXY );
	static const Mat44		CreateTranslation3D( const Vec3& translation3D );
	static const Mat44		CreateUniformScaleXY( float uniformScaleXY );
	static const Mat44		CreateNonUniformScaleXY( const Vec2& scaleFactorsXY );
	static const Mat44		CreateUniformScale3D( float uniformScale3D );
	static const Mat44		CreateNonUniformScale3D( const Vec3& scaleFactorsXYZ );	

	static const Mat44		CreateLookAtMatrix( const Vec3 from, const Vec3 to );
	static const Mat44		CreateTransformMatrixFromCoordConvention();

	// Projection
	static const Mat44		CreateOrthographicProjection( const Vec3& min, const Vec3& max );	

	static const Mat44		FromRotationTranslation( Vec3 euler, Vec3 position );
	static const Mat44		FromPitchRollYaw( Vec3 euler ); 

private:
	const Mat44 operator*( const Mat44& rhs ) const = delete;
};
