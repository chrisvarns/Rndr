struct VSIn
{
	float4 Position : POSITION;
};

struct VSOut
{
	float4 Position : SV_POSITION;
};

VSOut main(VSIn input)
{
	VSOut output;
	output.Position = input.Position;
	return output;
}