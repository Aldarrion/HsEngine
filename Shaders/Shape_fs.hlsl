struct ps_in
{
    float4 Color : COLOR;
};

float4 main(ps_in input) : SV_Target
{
    return input.Color;
}
