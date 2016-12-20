struct VSIn
{
	float4 Position : POSITION;
};

struct VSOut
{
	float4 Position : SV_POSITION;
};

cbuffer VSConstantBuffer : register(b0)
{
	matrix ViewMatrix;
};

VSOut main(VSIn input)
{
	VSOut output;
	output.Position = mul(input.Position, ViewMatrix);
	return output;
}