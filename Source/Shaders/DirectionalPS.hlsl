struct PSIn
{
	float4 Position	: SV_POSITION;
	float2 UV		: TEXCOORD0;
};

struct PSOut
{
	float4 Color : SV_Target;
};

cbuffer VSConstantBuffer : register(b0)
{
	float4 lightColor;
	float4 lightDir;
};

Texture2D diffuseTex : register(t0);
Texture2D normalTex : register(t1);
SamplerState diffuseSampler;

PSOut main(PSIn input)
{
	PSOut output;

	const float3 diffuse = diffuseTex.Sample(diffuseSampler, input.UV);
	const float3 normal = normalTex.Sample(diffuseSampler, input.UV);

	const float3 ClampedAngle = clamp(dot(normal, -lightDir), 0, 1);
	output.Color.rgb = diffuse * lightColor.rgb * ClampedAngle;

	output.Color.a = 1.0f;

	return output;
}