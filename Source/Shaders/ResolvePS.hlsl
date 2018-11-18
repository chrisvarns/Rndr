struct PSIn
{
	float4 Position	: SV_POSITION;
	float2 UV		: TEXCOORD0;
};

struct PSOut
{
	float4 Color : SV_Target;
};

Texture2D diffuseTex;
SamplerState diffuseSampler;

PSOut main(PSIn input)
{
	PSOut output;
	output.Color = diffuseTex.Sample(diffuseSampler, input.UV);
	return output;
}