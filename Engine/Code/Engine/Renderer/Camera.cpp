#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"


Camera::~Camera()
{
	m_depthStencilTarget = nullptr;
}


Vec2 Camera::ClientToWorldPosition( Vec2 clientPos )
{
	Vec2 worldPos = Vec2( ( clientPos.x * m_orthoTopRight.x ), ( clientPos.y * m_orthoTopRight.y ) );
	return worldPos;
}

float Camera::GetOutputAspectRatio()
{
	//return GetColorTargetSize().GetRatio();
	return 0.f;
}

void Camera::SetPosition( const Vec3& position )
{
	m_orthoButtomLeft = Vec2( m_transform.m_position.x, m_transform.m_position.y ) - (m_outputSize * 0.5f);
	m_orthoTopRight = Vec2( m_transform.m_position.x, m_transform.m_position.y ) + (m_outputSize * 0.5f);
	m_transform.SetPosition( position );
}

void Camera::Translate( const Vec3& translation )
{
	m_position += translation;
}

void Camera::SetProjectionOrthographic( float height, float nearZ, float farZ )
{
	m_nearZ = nearZ;
	m_farZ = farZ;
	float width =  m_aspectRatio * height;
	m_outputSize = Vec2( width, height );

	m_orthoMin = Vec2( m_position.x, m_position.y ) - Vec2( width, height ) * 0.5f;
	m_orthoMax = Vec2( m_position.x, m_position.y ) + Vec2( width, height ) * 0.5f;
}

void Camera::SetProjectionPerspective( float fovDegrees, float nearZClip, float farZClip )
{
	// float fov is the field of view you want
	// float farZ and nearZ are the depth range you want to see
	// -> do **not** span zero here.

	// Goal is to...
	// - setup a default "depth" where (1, 1) == (1, 1) after projection (1 / tan(fov * .5f))
	// - map z to w, so the z divide happens (we go along negative, so we want a negative w-divide)
	// - map nearZ to 0, farZ to -farZ, since a -Z divide will happen
	//   and this will result in mapping nearZ to 0, and farZ to 1 in NDC space 
	//   -> ((z - nz) / (fz - nz)) * fz + 0
	//   -> fz / (fz - nz) * z      + (-fz * nz) / (fz - nz)

	float height = 1.0f / tanf( ConvertDegreesToRadians(fovDegrees * .5f) ); // how far away are we for the perspective point to be "one up" from our forward line. 
	float zrange = farZClip - nearZClip;
	float q = 1.0f / zrange;

	float projection[] = {
		height/m_aspectRatio,	0.f,		   0.f,							0.f,
		0.f,					height,        0.f,							0.f,
		0.f,					0.f,           -farZClip * q,				-1.f,
		0.f,					0.f,           nearZClip * farZClip * q,	0.f
	};

	m_projection = Mat44(projection);

	// Transform by Transformation Matrix (facing iBasis by default)
	//Mat44 newMat = Mat44::CreateTransformMatrixFromCoordConvention();
	/*Mat44 mat;
	mat.SetBasisVectors3D(
		Vec3( 0.f, 0.f, 1.f ),
		Vec3( 0.f, 1.f, 0.f ),
		Vec3( 1.f, 0.f, 0.f ) );*/

	//m_projection.TransformBy( /*Mat44::CreateTransformMatrixFromCoordConvention()*/mat );
}

void Camera::SetDepthStencilBuffer( Texture* texture )
{
	m_depthStencilTarget = texture;
}

void Camera::SetCanClearDepthBuffer( bool canClearDepth )
{
	m_canClearDepthBuffer = canClearDepth;
}

void Camera::SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight )
{
	m_orthoButtomLeft = bottomLeft;
	m_orthoTopRight = topRight;

	m_projection = MakeOrthographicProjectionMatrixD3D( Vec3( bottomLeft.x, bottomLeft.y, 0.f ), Vec3( topRight.x,topRight.y, 1.f ) );
}

void Camera::SetOutputSize( Vec2 size )
{
	m_aspectRatio = size.x / size.y;
	m_outputSize = size;
}


void Camera::SetProjectionMatrix( Mat44 projectionMatrix )
{
	m_projection = projectionMatrix;
}

Mat44 Camera::GetViewMatrix() const
{
	switch( m_convention )
	{
	case eWorldBasisConvention::WORLD_BASIS_XRIGHT_YUP_ZBACK:
	{
		Mat44 viewMatrix;
		viewMatrix.TransformBy( Mat44::CreateZRotationDegrees( m_transform.m_orientationRoll ) );
		viewMatrix.TransformBy( Mat44::CreateXRotationDegrees( -m_transform.m_orientationPitch ) );
		viewMatrix.TransformBy( Mat44::CreateYRotationDegrees( -m_transform.m_orientationYaw ) );
		viewMatrix.Translate3D( -m_transform.m_position );
		return viewMatrix;
	}	break;

	case eWorldBasisConvention::WORLD_BASIS_XFORWARD_YLEFT_ZUP:
	{
		Mat44 viewMatrix;
		viewMatrix.SetBasisVectors3D(
			Vec3( 0.f, 0.f, -1.f ),
			Vec3( -1.f, 0.f, 0.f ),
			Vec3( 0.f, 1.f, 0.f ) );

		viewMatrix.TransformBy( Mat44::CreateXRotationDegrees( m_transform.m_orientationRoll ) );
		viewMatrix.TransformBy( Mat44::CreateYRotationDegrees( -m_transform.m_orientationPitch ) );
		viewMatrix.TransformBy( Mat44::CreateZRotationDegrees( -m_transform.m_orientationYaw ) );
		viewMatrix.Translate3D( -m_transform.m_position );
		return viewMatrix;
	}	break;
		
	default: ERROR_AND_DIE( "Unknown World Basis Convention" );
	}

}

Mat44 Camera::GetProjectionMatrix() const
{
	return m_projection;
}


void Camera::SetPitchRollYawRotation( float pitch, float roll, float yaw )
{
	m_transform.SetRotationFromPitchRollYawDegrees( pitch, roll, yaw );
}

void Camera::SetClearMode( unsigned int clearFlags, Rgba8 color, float depth, unsigned int stencil )
{
	UNUSED( depth );
	UNUSED( stencil );

	m_clearMode = clearFlags;
	m_clearColor = color;
}

void Camera::ClearDepth( Texture* depthStencilTexture, float depth, uint stencil )
{
	m_clearDepth = depth;
	m_clearStencil = stencil;

	TextureView* view = depthStencilTexture->GetOrCreateShaderResourceView();
	ID3D11DepthStencilView* dsv = view->GetAsDSV();
	UNUSED( dsv ); //ToDo: Implement depth function here
	//m_cameraUBO->m_owner->m_device->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH, depth, 0 );
}

Vec3 Camera::ClientToWorld( Vec2 client, float ndcZ )
{
	Vec3 worldPos = Vec3( (client.x * m_orthoMax.x), (client.y * m_orthoMax.y), ndcZ );
	return worldPos;
}

Vec3 Camera::WorldToClient( Vec3 worldPos )
{
	return Vec3();
}

void Camera::UpdateUBO()
{
	camera_data_t cameraData;
	cameraData.projection = m_projection;
	cameraData.view = GetViewMatrix();
	cameraData.camera_position = m_transform.m_position;

	if( m_cameraUBO ) {
		m_cameraUBO->Update( &cameraData, sizeof(cameraData), sizeof(cameraData) );
	}
}

void Camera::IntialUBO( RenderContext* ctx )
{
	m_cameraUBO = new RenderBuffer( ctx, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
}

void Camera::CleanUBO()
{
	delete m_cameraUBO;
	m_cameraUBO = nullptr;
}

int Camera::GetColorTargetCount() const
{
	return (int)m_colorTargets.size();
}

Texture* Camera::GetColorTarget() const
{
	if( m_colorTargets.size() == 0 ) {
		return nullptr;
	}
	else {
		return m_colorTargets[0];
	}
}

Texture* Camera::GetColorTarget( int index ) const
{
	if( m_colorTargets.size() == 0 ) {
		return nullptr;
	}
	else {
		return m_colorTargets[index];
	}
}

Vec3 Camera::GetOrthoMin()
{
	Vec4 ndc( -1.f, -1.f, 0.f, 1.f );

	Mat44 projection = m_projection;
	projection.TransformBy( GetViewMatrix() ); // world to renderable space

	Mat44 viewProjection = projection;
	Mat44 clipToWorld = GetInvert( viewProjection ); // renderable space to world

	Vec4 world = clipToWorld.TransformHomogeneousPoint3D( ndc );	// convert our bottom left corner
	world /= world.w; // find the representitive point (w == 1)

	return Vec3( world.x, world.y, world.z );
}

Vec3 Camera::GetForwardVector() const
{
	Vec3 cameraForward = Vec3( 1.f, 0.f, 0.f );
	cameraForward = m_transform.GetRotationMatrix().TransformVector3D( cameraForward );
	return cameraForward.GetNormalized();
}

Vec3 Camera::GetForwardVectorXY() const
{
	Vec3 cameraForward = Vec3( 1.f, 0.f, 0.f );
	cameraForward = m_transform.GetRotationMatrix().TransformVector3D( cameraForward );
	Vec3 cameraForwardXY = Vec3( cameraForward.x, cameraForward.y, 0.f ).GetNormalized();
	return cameraForwardXY;
}

Vec2 Camera::GetForwardVector2D() const
{
	Vec3 forward = GetForwardVectorXY();
	return Vec2( forward.x, forward.y );
}

void Camera::SetColorTarget( Texture* texture )
{
	SetColorTarget( 0, texture );
}

void Camera::SetColorTarget( int index, Texture* target )
{
	if( index >= m_colorTargets.size() ) {
		m_colorTargets.resize( index + 1 );
	}
	m_colorTargets[index] = target;
}

void Camera::SetDepthStencilTarget( Texture* texture )
{
	m_depthStencilTarget = texture;
}

