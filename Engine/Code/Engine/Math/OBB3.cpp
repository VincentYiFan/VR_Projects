#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/MathUtils.hpp"

const OBB3 OBB3::ZERO_TO_ONE( Vec3( 0.5f, 0.5f, 0.5f ), Vec3( 1.f, 1.f, 1.f ), Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ) );

OBB3::OBB3( const Vec3& center, const Vec3& fullDimensions, const Vec3& iBasisNormal, const Vec3& jBasisNormal )
{
	m_center = center;
	m_iBasis = iBasisNormal;
	m_jBasis = jBasisNormal;

	m_halfDimensions = 0.5f * fullDimensions;

	// Fix
	m_halfDimensions.x = fabsf( m_halfDimensions.x );
	m_halfDimensions.y = fabsf( m_halfDimensions.y );
	m_halfDimensions.z = fabsf( m_halfDimensions.y );

	m_iBasis = m_iBasis.GetNormalized();
	m_jBasis = m_jBasis.GetNormalized();
	m_kBasis = CrossProduct( m_iBasis, m_jBasis );
}

std::vector<Vec3> OBB3::GetVertsPositions()
{
	std::vector<Vec3> verts;
	Vec3 p0 = m_center - (m_halfDimensions.x * m_iBasis) - (m_halfDimensions.y * m_jBasis) - (m_halfDimensions.z * m_kBasis);
	Vec3 p1 = m_center + (m_halfDimensions.x * m_iBasis) - (m_halfDimensions.y * m_jBasis) - (m_halfDimensions.z * m_kBasis);
	Vec3 p2 = m_center + (m_halfDimensions.x * m_iBasis) + (m_halfDimensions.y * m_jBasis) - (m_halfDimensions.z * m_kBasis);
	Vec3 p3 = m_center - (m_halfDimensions.x * m_iBasis) + (m_halfDimensions.y * m_jBasis) - (m_halfDimensions.z * m_kBasis);

	Vec3 p4 = m_center - (m_halfDimensions.x * m_iBasis) - (m_halfDimensions.y * m_jBasis) + (m_halfDimensions.z * m_kBasis);
	Vec3 p5 = m_center + (m_halfDimensions.x * m_iBasis) - (m_halfDimensions.y * m_jBasis) + (m_halfDimensions.z * m_kBasis);
	Vec3 p6 = m_center + (m_halfDimensions.x * m_iBasis) + (m_halfDimensions.y * m_jBasis) + (m_halfDimensions.z * m_kBasis);
	Vec3 p7 = m_center - (m_halfDimensions.x * m_iBasis) + (m_halfDimensions.y * m_jBasis) + (m_halfDimensions.z * m_kBasis);

	verts.push_back( p0 );
	verts.push_back( p1 );
	verts.push_back( p2 );
	verts.push_back( p3 );

	verts.push_back( p4 );
	verts.push_back( p5 );
	verts.push_back( p6 );
	verts.push_back( p7 );

	return verts;
}
