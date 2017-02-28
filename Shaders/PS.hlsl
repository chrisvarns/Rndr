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

PSOut main(PSIn input)
{
	PSOut output;

	//Solid Colour
	if (RenderMode.x == 0)
	{
		output.Colour = float4(0.62852854784553653, 0.95568463956856, 0.495393874586, 1);
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
	else if (RenderMode.x == 3)
	{
		output.Colour = float4(input.Position.zzz, 1);
	}
	return output;
}