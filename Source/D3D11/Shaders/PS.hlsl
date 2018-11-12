struct PSIn
{
	float4 Position	: SV_POSITION;
	float4 Normal	: NORMAL;
	float4 UV		: TEXCOORD0;
};

struct PSOut
{
	float4 Color: SV_Target0;
	float4 Normal: SV_Target1;
};

Texture2D diffuseTex;
SamplerState diffuseSampler;

PSOut main(PSIn input)
{
	PSOut output;

	output.Color = diffuseTex.Sample(diffuseSampler, input.UV.xy);
	output.Normal = float4(normalize(input.Normal.xyz) * 0.5 + 0.5, 1);
	return output;
}

////Diffuse
//if (RenderMode.x == 0)
//{
//	output.Color = diffuseTex.Sample(diffuseSampler, input.UV.xy);
//}
////Normals
//else if (RenderMode.x == 1)
//{
//	output.Color = float4(normalize(input.Normal.xyz) * 0.5 + 0.5, 1);
//}
////UV
//else if (RenderMode.x == 2)
//{
//	output.Color = float4(frac(input.UV.xy), 0, 1);
//}
////Depth
//else //(RenderMode.x == 3)
//{
//	float3 sRGB = input.Position.zzz;
//	float3 RGB = sRGB * (sRGB * (sRGB * 0.305306011 + 0.682171111) + 0.012522878);
//	output.Color = float4(RGB, 1);
//}