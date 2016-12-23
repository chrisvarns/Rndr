struct VSIn
{
	float4 Position : POSITION;
	float4 Normal	: NORMAL;
};

struct VSOut
{
	float4 Position	: SV_POSITION;
	float4 Normal	: NORMAL;
	float Depth		: TEXTURE0;
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
	output.Depth = output.Position.z / output.Position.w;
	return output;
}