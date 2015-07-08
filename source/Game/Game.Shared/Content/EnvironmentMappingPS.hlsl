cbuffer CBufferPerFrame
{
	float4 AmbientColor;
	float4 EnvColor;
}

cbuffer CBufferPerObject
{
	float ReflectionAmount;
}

TextureCube EnvironmentMap : register(t1);
Texture2D ColorTexture : register(t0);
SamplerState TrilinearSampler;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 TextureCoordinate : TEXCOORD;
	float3 ReflectionVector : REFLECTION;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float4 OUT = (float4)0;

	float4 color = ColorTexture.Sample(TrilinearSampler, IN.TextureCoordinate);
	float3 ambient = AmbientColor.rgb * color.rgb;
	float3 environment = EnvColor.rgb * EnvironmentMap.Sample(TrilinearSampler, IN.ReflectionVector).rgb;

	OUT.rgb = lerp(ambient, environment, ReflectionAmount);
	OUT.a = color.a;

	return OUT;
}