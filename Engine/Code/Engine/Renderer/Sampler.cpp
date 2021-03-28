#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer//RenderContext.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Sampler::Sampler( RenderContext* ctx, eSamplerType type )
	: m_owner( ctx )
{
	D3D11_SAMPLER_DESC desc;

	switch( type ) {
	case SAMPLER_LINEAR:		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;			break;
	case SAMPLER_POINT:			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;			break;
	case SAMPLER_BILINEAR:		desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;	break;
	case SAMPLER_ANISOTROPIC:	desc.Filter = D3D11_FILTER_ANISOTROPIC;					break;
	default:					ERROR_AND_DIE( "Unknown sampler type!" );				break;
	}

	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	if( type == SAMPLER_ANISOTROPIC ) {
		desc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	}
	else {
		desc.MaxAnisotropy = 0;
	}

	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	//desc.BorderColor[0] = 0.f;
	//desc.BorderColor[1] = 0.f;
	//desc.BorderColor[2] = 0.f;
	//desc.BorderColor[3] = 0.f;
	desc.MipLODBias = 0.f;
	desc.MinLOD = 0.f;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11Device* device = ctx->m_device;
	HRESULT hr = device->CreateSamplerState( &desc, &m_handle );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Failed to create sampler state!" );
}

Sampler::~Sampler()
{
	DX_SAFE_RELEASE( m_handle );
}
