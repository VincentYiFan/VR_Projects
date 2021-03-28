#include "Mat44.hpp"
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


Mat44::Mat44( float const* sixteenValuesBasisMajor )
{
	Ix = sixteenValuesBasisMajor[0];
	Iy = sixteenValuesBasisMajor[1];
	Iz = sixteenValuesBasisMajor[2];
	Iw = sixteenValuesBasisMajor[3];
	Jx = sixteenValuesBasisMajor[4];
	Jy = sixteenValuesBasisMajor[5];
	Jz = sixteenValuesBasisMajor[6];
	Jw = sixteenValuesBasisMajor[7];
	Kx = sixteenValuesBasisMajor[8];
	Ky = sixteenValuesBasisMajor[9];
	Kz = sixteenValuesBasisMajor[10];
	Kw = sixteenValuesBasisMajor[11];
	Tx = sixteenValuesBasisMajor[12];
	Ty = sixteenValuesBasisMajor[13];
	Tz = sixteenValuesBasisMajor[14];
	Tw = sixteenValuesBasisMajor[15];
}

const Mat44 Mat44::IDENTITY = Mat44();

Mat44::Mat44( const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& translation2D )
{
	Ix = iBasis2D.x;
	Iy = iBasis2D.y;

	Jx = jBasis2D.x;
	Jy = jBasis2D.y;

	Tx = translation2D.x;
	Ty = translation2D.y;
}

Mat44::Mat44( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& translation3D )
{
	Ix = iBasis3D.x;
	Iy = iBasis3D.y;
	Iz = iBasis3D.z;

	Jx = jBasis3D.x;
	Jy = jBasis3D.y;
	Jz = jBasis3D.z;

	Kx = kBasis3D.x;
	Ky = kBasis3D.y;
	Kz = kBasis3D.z;

	Tx = translation3D.x;
	Ty = translation3D.y;
	Tz = translation3D.z;
}

Mat44::Mat44( const Vec4& iBasisHomogeneous, const Vec4& jBasisHomogeneous, const Vec4& kBasisHomogeneous, const Vec4& translationHomogeneous )
{
	Ix = iBasisHomogeneous.x;
	Iy = iBasisHomogeneous.y;
	Iz = iBasisHomogeneous.z;
	Iw = iBasisHomogeneous.w;

	Jx = jBasisHomogeneous.x;
	Jy = jBasisHomogeneous.y;
	Jz = jBasisHomogeneous.z;
	Jw = jBasisHomogeneous.w;

	Kx = kBasisHomogeneous.x;
	Ky = kBasisHomogeneous.y;
	Kz = kBasisHomogeneous.z;
	Kw = kBasisHomogeneous.w;

	Tx = translationHomogeneous.x;
	Ty = translationHomogeneous.y;
	Tz = translationHomogeneous.z;
	Tw = translationHomogeneous.w;
}

const Vec2 Mat44::TransformVector2D( const Vec2& vectorQuantity ) const
{
	return Vec2( Ix * vectorQuantity.x + Jx * vectorQuantity.y, 
				 Iy * vectorQuantity.x + Jy * vectorQuantity.y );
}

const Vec3 Mat44::TransformVector3D( const Vec3& vectorQuantity ) const
{
	return Vec3( Ix * vectorQuantity.x + Jx * vectorQuantity.y + Kx * vectorQuantity.z, 
				 Iy * vectorQuantity.x + Jy * vectorQuantity.y + Ky * vectorQuantity.z,
				 Iz * vectorQuantity.x + Jz * vectorQuantity.y + Kz * vectorQuantity.z );
}

const Vec2 Mat44::TransformPosition2D( const Vec2& position ) const
{
	return Vec2( (Ix * position.x + Jx * position.y + Tx ), ( Iy * position.x + Jy * position.y + Ty ) );
}

const Vec3 Mat44::TransformPosition3D( const Vec3& position ) const
{
	return Vec3( Ix * position.x + Jx * position.y + Kx * position.z + Tx,
				 Iy * position.x + Jy * position.y + Ky * position.z + Ty,
				 Iz * position.x + Jz * position.y + Kz * position.z + Tz );
}

const Vec4 Mat44::TransformHomogeneousPoint3D( const Vec4& homogeneousPoint ) const
{
	return Vec4( Ix * homogeneousPoint.x + Jx * homogeneousPoint.y + Kx * homogeneousPoint.z + Tx * homogeneousPoint.w,
				 Iy * homogeneousPoint.x + Jy * homogeneousPoint.y + Ky * homogeneousPoint.z + Ty * homogeneousPoint.w,
				 Iz * homogeneousPoint.x + Jz * homogeneousPoint.y + Kz * homogeneousPoint.z + Tz * homogeneousPoint.w,
				 Iw * homogeneousPoint.x + Jw * homogeneousPoint.y + Kw * homogeneousPoint.z + Tw * homogeneousPoint.w );
}

const Vec2 Mat44::GetIBasis2D() const
{
	return Vec2( Ix, Iy );
}

const Vec2 Mat44::GetJBasis2D() const
{
	return Vec2( Jx, Jy );
}

const Vec2 Mat44::GetTranslation2D() const
{
	return Vec2( Tx, Ty );
}

const Vec3 Mat44::GetIBasis3D() const
{
	return Vec3( Ix, Iy, Iz );
}

const Vec3 Mat44::GetJBasis3D() const
{
	return Vec3( Jx, Jy, Jz );
}

const Vec3 Mat44::GetKBasis3D() const
{
	return Vec3( Kx, Ky, Kz );
}

const Vec3 Mat44::GetTranslation3D() const
{
	return Vec3( Tx, Ty, Tz );
}

const Vec4 Mat44::GetIBasis4D() const
{
	return Vec4( Ix, Iy, Iz, Iw );
}

const Vec4 Mat44::GetJBasis4D() const
{
	return Vec4( Jx, Jy, Jz, Jw);
}

const Vec4 Mat44::GetKBasis4D() const
{
	return Vec4( Kx, Ky, Kz, Kw );
}

const Vec4 Mat44::GetTranslation4D() const
{
	return Vec4( Tx, Ty, Tz, Tw );
}

void Mat44::SetTranslation2D( const Vec2& translation2D )
{
	Tx = translation2D.x;
	Ty = translation2D.y;
}

void Mat44::SetTranslation3D( const Vec3& translation3D )
{
	Tx = translation3D.x;
	Ty = translation3D.y;
	Tz = translation3D.z;
}

void Mat44::SetBasisVectors2D( const Vec2& iBasis2D, const Vec2& jBasis2D )
{
	Ix = iBasis2D.x;
	Iy = iBasis2D.y;

	Jx = jBasis2D.x;
	Jy = jBasis2D.y;
}

void Mat44::SetBasisVectors2D( const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& tranlation2D )
{
	Ix = iBasis2D.x;
	Iy = iBasis2D.y;

	Jx = jBasis2D.x;
	Jy = jBasis2D.y;

	Tx = tranlation2D.x;
	Ty = tranlation2D.y;
}

void Mat44::SetBasisVectors3D( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D )
{
	Ix = iBasis3D.x;
	Iy = iBasis3D.y;
	Iz = iBasis3D.z;

	Jx = jBasis3D.x;
	Jy = jBasis3D.y;
	Jz = jBasis3D.z;

	Kx = kBasis3D.x;
	Ky = kBasis3D.y;
	Kz = kBasis3D.z;
}

void Mat44::SetBasisVectors3D( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& tranlation3D )
{
	Ix = iBasis3D.x;
	Iy = iBasis3D.y;
	Iz = iBasis3D.z;

	Jx = jBasis3D.x;
	Jy = jBasis3D.y;
	Jz = jBasis3D.z;

	Kx = kBasis3D.x;
	Ky = kBasis3D.y;
	Kz = kBasis3D.z;

	Tx = tranlation3D.x;
	Ty = tranlation3D.y;
	Tz = tranlation3D.z;
}

void Mat44::SetBasisVectors4D( const Vec4& iBasis4D, const Vec4& jBasis4D, const Vec4& kBasis4D, const Vec4& tranlation4D )
{
	Ix = iBasis4D.x;
	Iy = iBasis4D.y;
	Iz = iBasis4D.z;
	Iw = iBasis4D.w;
			   
	Jx = jBasis4D.x;
	Jy = jBasis4D.y;
	Jz = jBasis4D.z;
	Jw = jBasis4D.w;

			   
	Kx = kBasis4D.x;
	Ky = kBasis4D.y;
	Kz = kBasis4D.z;
	Kw = kBasis4D.w;

	Tx = tranlation4D.x;
	Ty = tranlation4D.y;
	Tz = tranlation4D.z;
	Tw = tranlation4D.w;
}

void Mat44::RotateXDegrees( float degreesAboutX )
{
	Mat44 mat44 = Mat44::IDENTITY;
	float c = CosDegrees(degreesAboutX);
	float s = SinDegrees(degreesAboutX);

	mat44.Jy = c;
	mat44.Kz = c;
	mat44.Jz = s;
	mat44.Ky = -s;
	TransformBy(mat44);
}

void Mat44::RotateYDegrees( float degreesAboutY )
{
	Mat44 mat44 = Mat44::IDENTITY;
	float c = CosDegrees( degreesAboutY );
	float s = SinDegrees( degreesAboutY );

	mat44.Ix = c;
	mat44.Kz = c;
	mat44.Kx = s;
	mat44.Iz = -s;
	TransformBy(mat44);
}

void Mat44::RotateZDegrees( float degreesAboutZ )
{
	Mat44 mat44 = Mat44::IDENTITY;
	float c = CosDegrees( degreesAboutZ );
	float s = SinDegrees( degreesAboutZ );

	mat44.Ix = c;
	mat44.Jy = c;
	mat44.Iy = s;
	mat44.Jx = -s;
	TransformBy(mat44);
}

void Mat44::Translate2D( const Vec2& translation2D )
{
	Mat44 newMat44 = Mat44::IDENTITY;
	newMat44.SetTranslation2D(translation2D);
	TransformBy(newMat44);
}

void Mat44::Translate3D( const Vec3& translation3D )
{
	Mat44 newMat44 = Mat44::IDENTITY;
	newMat44.SetTranslation3D( translation3D );
	TransformBy(newMat44);
}

void Mat44::ScaleUniform2D( float uniformScaleXY )
{
	Mat44 newMat44 = Mat44::IDENTITY;
	newMat44.Ix = uniformScaleXY;
	newMat44.Jy = uniformScaleXY;
	TransformBy(newMat44);
}

void Mat44::ScaleNonUniform2D( const Vec2& scaleFactorsXY )
{
	Mat44 newMat44 = Mat44::IDENTITY;
	newMat44.Ix = scaleFactorsXY.x;
	newMat44.Jy = scaleFactorsXY.y;
	TransformBy(newMat44);
}

void Mat44::ScaleUniform3D( float uniformScaleXYZ )
{
	Mat44 newMat44 = Mat44::IDENTITY;
	newMat44.Ix = uniformScaleXYZ;
	newMat44.Jy = uniformScaleXYZ;
	newMat44.Kz = uniformScaleXYZ;
	TransformBy(newMat44);
}

void Mat44::ScaleNonUniform3D( const Vec3& scaleFactorsXYZ )
{
	Mat44 newMat44 = Mat44::IDENTITY;
	newMat44.Ix = scaleFactorsXYZ.x;
	newMat44.Jy = scaleFactorsXYZ.y;
	newMat44.Kz = scaleFactorsXYZ.z;
	TransformBy(newMat44);
}

//const Vec3& Mat44::GetTranslation3D() const
//{
//	Vec3* theRealTranslation = (Vec3*) &Tx; // Fix
//	return *theRealTranslation;
//}

void Mat44::TransformBy( const Mat44& arbitraryTransformationToAppend )
{
	const Mat44 a = *this;
	const Mat44& b = arbitraryTransformationToAppend; // Nickname
	Ix = (a.Ix * b.Ix) + (a.Jx * b.Iy) + (a.Kx * b.Iz) + (a.Tx * b.Iw);
	Jx = (a.Ix * b.Jx) + (a.Jx * b.Jy) + (a.Kx * b.Jz) + (a.Tx * b.Jw);
	Kx = (a.Ix * b.Kx) + (a.Jx * b.Ky) + (a.Kx * b.Kz) + (a.Tx * b.Kw);
	Tx = (a.Ix * b.Tx) + (a.Jx * b.Ty) + (a.Kx * b.Tz) + (a.Tx * b.Tw);

	Iy = (a.Iy * b.Ix) + (a.Jy * b.Iy) + (a.Ky * b.Iz) + (a.Ty * b.Iw);
	Jy = (a.Iy * b.Jx) + (a.Jy * b.Jy) + (a.Ky * b.Jz) + (a.Ty * b.Jw);
	Ky = (a.Iy * b.Kx) + (a.Jy * b.Ky) + (a.Ky * b.Kz) + (a.Ty * b.Kw);
	Ty = (a.Iy * b.Tx) + (a.Jy * b.Ty) + (a.Ky * b.Tz) + (a.Ty * b.Tw);

	Iz = (a.Iz * b.Ix) + (a.Jz * b.Iy) + (a.Kz * b.Iz) + (a.Tz * b.Iw);
	Jz = (a.Iz * b.Jx) + (a.Jz * b.Jy) + (a.Kz * b.Jz) + (a.Tz * b.Jw);
	Kz = (a.Iz * b.Kx) + (a.Jz * b.Ky) + (a.Kz * b.Kz) + (a.Tz * b.Kw);
	Tz = (a.Iz * b.Tx) + (a.Jz * b.Ty) + (a.Kz * b.Tz) + (a.Tz * b.Tw);

	Iw = (a.Iw * b.Ix) + (a.Jw * b.Iy) + (a.Kw * b.Iz) + (a.Tw * b.Iw);
	Jw = (a.Iw * b.Jx) + (a.Jw * b.Jy) + (a.Kw * b.Jz) + (a.Tw * b.Jw);
	Kw = (a.Iw * b.Kx) + (a.Jw * b.Ky) + (a.Kw * b.Kz) + (a.Tw * b.Kw);
	Tw = (a.Iw * b.Tx) + (a.Jw * b.Ty) + (a.Kw * b.Tz) + (a.Tw * b.Tw);
}

const Mat44 Mat44::CreateXRotationDegrees( float degreesAboutX )
{
	Mat44 newMat44;
	newMat44.RotateXDegrees( degreesAboutX );
	return newMat44;
}

const Mat44 Mat44::CreateYRotationDegrees( float degreesAboutY )
{
	Mat44 newMat44;
	newMat44.RotateYDegrees( degreesAboutY );
	return newMat44;
}

const Mat44 Mat44::CreateZRotationDegrees( float degreesAboutZ )
{
	Mat44 newMat44;
	newMat44.RotateZDegrees( degreesAboutZ );
	return newMat44;
}

const Mat44 Mat44::CreateRotationAroundAxis( float degressAboutAxis, Vec3 axis )
{
	float c = CosDegrees( degressAboutAxis );
	float s = SinDegrees( degressAboutAxis );

	Mat44 newMat;
	newMat.Ix = c + ( axis.x * axis.x ) * ( 1 - c );
	newMat.Iy = axis.y * axis.x * ( 1 - c ) + axis.z * s;
	newMat.Iz = axis.z * axis.x * ( 1 - c ) - axis.y * s;

	return newMat;
	// #ToDo: finish the matrix
}

const Mat44 Mat44::CreateTranslationXY( const Vec2& translationXY )
{
	Mat44 newMat44;
	newMat44.SetTranslation2D(translationXY);
	return newMat44;

}

const Mat44 Mat44::CreateTranslation3D( const Vec3& translation3D )
{
	Mat44 newMat44;
	newMat44.SetTranslation3D( translation3D );
	return newMat44;
}

const Mat44 Mat44::CreateUniformScaleXY( float uniformScaleXY )
{
	Mat44 newMat44;
	newMat44.ScaleUniform2D( uniformScaleXY );
	return newMat44;
}

const Mat44 Mat44::CreateNonUniformScaleXY( const Vec2& scaleFactorsXY )
{
	Mat44 newMat44;
	newMat44.ScaleNonUniform2D( scaleFactorsXY );
	return newMat44;
}

const Mat44 Mat44::CreateUniformScale3D( float uniformScale3D )
{
	Mat44 newMat44;
	newMat44.ScaleUniform3D( uniformScale3D );
	return newMat44;
}

const Mat44 Mat44::CreateNonUniformScale3D( const Vec3& scaleFactorsXYZ )
{
	Mat44 newMat44;
	newMat44.ScaleNonUniform3D( scaleFactorsXYZ );
	return newMat44;
}

const Mat44 Mat44::CreateLookAtMatrix( const Vec3 from, const Vec3 to )
{
	Vec3 forward = ( to - from ).GetNormalized();

	Vec3 randomVec = Vec3( 0.f, 1.f, 0.f ); // Must be normalized
	Vec3 right = CrossProduct( randomVec, forward );
	if( right.GetLengthSquared() == 0.f )
	{
		right = CrossProduct( Vec3( 0.f, 0.f, 1.f ), forward );
	}

	Vec3 up = CrossProduct( forward, right );

	Mat44 mat;
	mat.SetBasisVectors3D( right, up, forward );
	mat.SetTranslation3D( from );

	return mat;
}

const Mat44 Mat44::CreateTransformMatrixFromCoordConvention()
{
	const std::string coordinateStr = g_gameConfigBlackboard.GetValue( "coordinate", "XnZY" ); 
	if( coordinateStr == "XRIGHT_YUP_ZBACK" ) {
		return Mat44::IDENTITY;
	}
	else if( coordinateStr == "XFORWARD_YLEFT_ZUP" ) {
		Mat44 mat;

		mat.SetBasisVectors3D(
			Vec3( 0.f, 0.f, -1.f ),
			Vec3( -1.f, 0.f, 0.f ),
			Vec3( 0.f, 1.f, 0.f ) );

		return mat;
	}
	else {
		ERROR_AND_DIE( "Unknown type of coordinate convention!(Mat44::CreateTransformMatrixFromCoordConvention)" );
	}

	/*switch( coordConvention )
	{
	case eCoordinateConvention::RIGHT_HANDED_XnZY:	return Mat44::IDENTITY;	break;
	case eCoordinateConvention::RIGHT_HANDED_XYZ: {
		Mat44 mat;
		mat.SetBasisVectors3D( Vec3( 1.f, 0.f, 0.f ),
							   Vec3( 0.f, 0.f, -1.f ),
							   Vec3( 0.f, 1.f, 0.f ) );

		return mat;
	} break;

	default: ERROR_AND_DIE( "Unknown type of coordinate convention!(Mat44::CreateTransformMatrixFromCoordConvention)" );
	}*/
}

const Mat44 Mat44::CreateOrthographicProjection( const Vec3& min, const Vec3& max )
{
	// think of x 

	// min.x, max.x -> ( -1, 1 )
	//ndc.x =  (x - min.x) / (max.x - min.x) * ( 1.0f - (-1.0f) ) + (-1)
	//ndc.x = x / (max.x - min.x) - (min.x / ( max.x - min.x )* 2.f + (-1.f)
	// a = 2.0f / ( max.x - min.x )
	// b = ( -2.f * min.x - max.x + min.x ) / ( max.x - min.x )
	// = -( max.x + min.x ) / ( max.x - min.x )

	Vec3 diff = max - min;
	Vec3 sum = max + min;

	float mat[] = {
		2.0f/diff.x,	0.f,			0.f,			0.f,
		0.f,			2.0f/diff.y,	0.f,			0.f,
		0.f,			0.f,			1.0f/diff.z,	0.f,
		-sum.x/diff.x,	-sum.y/diff.y,	-min.z/diff.z,	1.f
	};

	return Mat44(mat);
}

const Mat44 Mat44::FromRotationTranslation( Vec3 euler, Vec3 position )
{
	//Mat44 mat = CreateTransformMatrixFromCoordConvention();
	//mat.Translate3D( position );
	
	Mat44 mat = Mat44::CreateTranslation3D(position);
	mat.TransformBy(Mat44::FromPitchRollYaw( euler ) );
	return mat;
}

const Mat44 Mat44::FromPitchRollYaw( Vec3 pitchRollYaw )
{
	Mat44 mat = Mat44::IDENTITY;
	//Mat44 mat = Mat44::CreateTransformMatrixFromCoordConvention();
	mat.TransformBy( CreateZRotationDegrees( pitchRollYaw.z ) );
	mat.TransformBy( CreateYRotationDegrees( pitchRollYaw.x ) );
	mat.TransformBy( CreateXRotationDegrees( pitchRollYaw.y ) );

	return mat;
}

