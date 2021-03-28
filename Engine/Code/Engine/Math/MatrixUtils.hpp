#pragma once

struct Mat44;
struct Vec3;

const Mat44 MakeOrthographicProjectionMatrixD3D( const Vec3& min, const Vec3& max );

Mat44 MakePerpsectiveProjectionMatrixD3D( float fovDegrees, float aspectRatio, float nearZ, float farZ );


 // Utilities
void MatrixTranspose( Mat44& mat );
void MatrixInvertOrthoNormal( Mat44& mat );
void MatrixInvert( Mat44& mat );

Mat44 GetInvert( Mat44 const& mat );
