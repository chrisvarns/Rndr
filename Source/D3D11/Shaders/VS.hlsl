struct VSIn
{
	float4 Position : POSITION;
	float4 Normal	: NORMAL;
	float4 UV		: TEXCOORD0;
};

struct VSOut
{
	float4 Position	: SV_POSITION;
	float4 Normal	: NORMAL;
	float4 UV		: TEXCOORD0;
};

cbuffer VSConstantBuffer : register(b0)
{
	matrix MvpMatrix;
	int4 renderMode;
};

VSOut main(VSIn input)
{
	VSOut output;
	output.Position = mul(MvpMatrix, input.Position);
	output.Normal = input.Normal;
	output.UV = input.UV;
    output.UV.g = 1 - output.UV.g;
	return output;
}