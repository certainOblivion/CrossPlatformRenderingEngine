cbuffer CBufferPerFrame
{
	float4 AmbientColor;
	float4 LightColor;
	float4 FogColor;
	float3 CameraPosition;
}

cbuffer CBufferPerObject
{
	float3 SpecularColor;
	float SpecularPower;
}

Texture2D ColorTexture;
Texture2D TransparencyMap;
SamplerState TrilinearSampler;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;
	float2 TextureCoordinate : TEXCOORD;
	float3 Normal : NORMAL;
	float3 LightDirection : LIGHTDIR;
	float FogAmount : FOG;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	if (IN.FogAmount == 1.0f)
	{
		return float4(FogColor.rgb, 1.0f);
	}

	float sampledAlpha = TransparencyMap.Sample(TrilinearSampler, IN.TextureCoordinate).a;
	if (sampledAlpha == 0.0f)
	{
		return float4(1.0f, 1.0f, 1.0f, 0.0f);
	}

	float4 OUT = (float4)0;

	float3 normal = normalize(IN.Normal);
	float3 lightDirection = normalize(IN.LightDirection);
	float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);
	float n_dot_l = dot(normal, lightDirection);
	float3 halfVector = normalize(lightDirection + viewDirection);
	float n_dot_h = dot(normal, halfVector);

	float4 color = ColorTexture.Sample(TrilinearSampler, IN.TextureCoordinate);
	float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);

	float3 ambient = AmbientColor.rgb * color.rgb;
	float3 diffuse = LightColor.rgb * lightCoefficients.y * color.rgb;
	float3 specular = SpecularColor * min(lightCoefficients.z, color.w);

	OUT.rgb = lerp(ambient + diffuse + specular, FogColor.rgb, IN.FogAmount);
	OUT.a = sampledAlpha;

	return OUT;
}