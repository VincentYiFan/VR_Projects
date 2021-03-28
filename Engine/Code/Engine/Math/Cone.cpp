#include "Engine/Math/Cone.hpp"

Cone::Cone( const Vec3& center, float radius, const Vec3& apexPoint, float height )
    :m_center( center )
    ,m_radius( radius )
    ,m_apexPoint( apexPoint )
    ,m_height( height )
{
}

Vec3 Cone::GetApexPoint() const
{
    return m_apexPoint;
}
