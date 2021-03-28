#include "AnalogJoystick.hpp"
#include <math.h>


AnalogJoystick::AnalogJoystick( float innerDeadZoneFraction, float outerDeadZoneFraction )
	:m_innerDeadZoneFraction(innerDeadZoneFraction)
	,m_outerDeadZoneFraction(outerDeadZoneFraction)
{
	
}

void AnalogJoystick::Reset()
{
	m_rawPosition = Vec2(0.f, 0.f);
	m_correctedPosition = Vec2( 0.f, 0.f );
	m_correctedDegrees		= 0.f;
	m_correctedMagnitude	= 0.f;
}

void AnalogJoystick::UpdatePosition( float rawNormailizedX, float rawNormailizedY )
{
	
	
	Vec2 m_rawNormalizedPosition = Vec2( rawNormailizedX, rawNormailizedY );

	if( m_rawNormalizedPosition.GetLength() < m_innerDeadZoneFraction ) 
	{
		m_rawNormalizedPosition = Vec2( 0.f, 0.f );
	}
	else if( m_rawNormalizedPosition.GetLength() > m_outerDeadZoneFraction ) 
	{
		m_rawNormalizedPosition.SetLength( 1.f ); 
	}
	else 
	{
		m_rawNormalizedPosition *= (( m_rawNormalizedPosition.GetLength() - m_innerDeadZoneFraction) / (1 - m_innerDeadZoneFraction));
	}
	m_correctedPosition		= m_rawNormalizedPosition;
	m_correctedDegrees		= m_rawNormalizedPosition.GetAngleDegrees();
	m_correctedMagnitude	= m_rawNormalizedPosition.GetLength();
}
