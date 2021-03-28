#pragma once

struct ID3D11SamplerState;
class RenderContext;

enum eSamplerType
{
	SAMPLER_POINT,
	SAMPLER_BILINEAR,
	SAMPLER_LINEAR,
	SAMPLER_ANISOTROPIC,
};


class Sampler
{
public:
	Sampler( RenderContext* ctx, eSamplerType type );
	~Sampler();

	inline ID3D11SamplerState* GetHandle() const { return m_handle; }

public:
	RenderContext* m_owner;
	ID3D11SamplerState* m_handle;
};
