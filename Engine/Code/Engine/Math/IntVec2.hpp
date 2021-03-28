#pragma once

struct IntVec2 
{
public:
	int x = 0;
	int y = 0;

	static const IntVec2 ZERO;
	static const IntVec2 ONE;

public:
	~IntVec2() = default;
	IntVec2()  = default;
	IntVec2( const IntVec2& copyFrom );
	explicit IntVec2( int initialX, int initialY );

	void SetFromText( const char* text );

	// Accessors (const methods)
	float			GetLength() const;
	int				GetLengthSquared() const;
	int				GetTaxicabLength() const;
	float			GetOrientationRadians() const;
	float			GetOrientationDegrees() const;
	const IntVec2	GetRotated90Degrees() const;
	const IntVec2	GetRotatedMinus90Degrees() const;

	// Mutators (non-const methods)
	void			Rotate90Degrees();
	void			RotateMinus90Degrees();

	//Operators ( const )
	bool			operator==( const IntVec2& compare ) const;			// ivec2 == ivec2
	bool			operator!=( const IntVec2& compare ) const;			// ivec2 != ivec2
	const IntVec2	operator+( const IntVec2& vecToAdd ) const;			// ivec2 + ivec2
	const IntVec2	operator-( const IntVec2& vecToSubtract ) const;	// ivec2 - ivec2
	const IntVec2	operator-() const;									// -ivec2, i.r. "unary negation"
	const IntVec2	operator*( int uniformScale ) const;				// ivec2 * int
	const IntVec2	operator*( const IntVec2& vecToMultiply ) const;	// ivec2 * ivec2


	// Operators (self-mutating / non-const)
	void			operator+=( const IntVec2& vecToAdd); 				// ivec2 +=  ivec2			
	void			operator-=( const IntVec2& vecToSubtract );			// ivec2 -=  ivec2	
	void			operator*=( const int uniformScale );				// ivec2 *= int
	void			operator=( const IntVec2& copyFrom );				// ivec2 = ivec2

	// Standalone "friend" functions that are conceptually, but not actually, part of IntVec2::
	friend const IntVec2 operator*( int uniformScale, const IntVec2& vecToScale ); // int * ivec2

};
