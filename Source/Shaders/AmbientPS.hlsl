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
	float4 ambientCol;
};

Texture2D diffuseTex;
SamplerState diffuseSampler;

PSOut main(PSIn input)
{
	PSOut output;

	float4 diffuse = diffuseTex.Sample(diffuseSampler, input.UV);
	output.Color = diffuse * ambientCol;

	return output;
}