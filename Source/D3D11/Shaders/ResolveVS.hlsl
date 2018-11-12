struct VSIn
{
	float2 Position : POSITION;
	float2 UV		: TEXCOORD0;
};

struct VSOut
{
	float4 Position	: SV_POSITION;
	float2 UV		: TEXCOORD0;
};

VSOut main(VSIn input)
{
	VSOut output;
	output.Position = float4(input.Position, 0, 1);
	output.UV = input.UV;
	return output;
}