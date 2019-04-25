// Global variab to store a combined view and projection transformation matrix.


matrix ViewProjMatrix;

vector Blue = { .0f, .0f, 1.0f, 1.0f };

struct VS_INPUT
{
    vector position : POSITION;
};

struct VS_OUTPUT
{
    vector position : POSITION;
    vector diffuse : COLOR;
};

VS_OUTPUT Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    output.position = mul(input.position, ViewProjMatrix);

    output.diffuse = Blue;

    return output;
};