struct PSIn
{
	float4 Position	: SV_POSITION;
	float4 Normal	: NORMAL;
	float4 UV		: TEXCOORD0;
};

struct PSOut
{
	float4 Colour : SV_TARGET;
};

cbuffer VSConstantBuffer : register(b0)
{
	matrix MvpMatrix;
	int4 RenderMode;
};

Texture2D diffuseTex;
SamplerState diffuseSampler;

PSOut main(PSIn input)
{
	PSOut output;

	//Diffuse
	if (RenderMode.x == 0)
	{
        output.Colour = diffuseTex.Sample(diffuseSampler, input.UV.xy);
	}
	//Normals
	else if (RenderMode.x == 1)
	{
		output.Colour = float4(normalize(input.Normal.xyz) * 0.5 + 0.5, 1);
	}
	//UV
	else if (RenderMode.x == 2)
	{
		output.Colour = float4(frac(input.UV.xy), 0, 1);
	}
	//Depth
	else //(RenderMode.x == 3)
	{
		float3 sRGB = input.Position.zzz;
		float3 RGB = sRGB * (sRGB * (sRGB * 0.305306011 + 0.682171111) + 0.012522878);
		output.Colour = float4(RGB, 1);
	}
	return output;
}