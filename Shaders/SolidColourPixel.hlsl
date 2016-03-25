struct PSIn
{
	float4 Position : SV_POSITION;
};

struct PSOut
{
	float4 Colour : SV_TARGET;
};

PSOut main(PSIn input)
{
	PSOut output;
	output.Colour = float4(0.62852854784553653, 0.95568463956856, 0.495393874586, 1);
	return output;
}