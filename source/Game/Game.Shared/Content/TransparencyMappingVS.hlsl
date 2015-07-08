cbuffer CBufferPerFrame
{
	float3 LightDirection;
	float3 CameraPosition;
	float FogStart;
	float FogRange;
}

cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
	float4x4 World : WORLD;
}

struct VS_INPUT
{
	float4 ObjectPosition: POSITION;
	float2 TextureCoordinate : TEXCOORD;
	float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;
	float2 TextureCoordinate : TEXCOORD;
	float3 Normal : NORMAL;
	float3 LightDirection : LIGHTDIR;
	float FogAmount : FOG;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
	OUT.WorldPosition = mul(IN.ObjectPosition, World).xyz;
	OUT.TextureCoordinate = IN.TextureCoordinate;
	OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);
	OUT.LightDirection = normalize(-LightDirection.xyz);
	OUT.FogAmount = saturate((distance(CameraPosition, OUT.WorldPosition) - FogStart) / (FogRange));

	return OUT;
}
