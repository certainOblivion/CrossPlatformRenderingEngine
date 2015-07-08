cbuffer CBufferPerFrame
{
	float4 AmbientColor;
	float4 LightColor;
}

cbuffer CBufferPerObject
{
	float3 SpecularColor;
	float SpecularPower;
}

Texture2D ColorTexture;
SamplerState ColorSampler;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 TextureCoordinate : TEXCOORD;
	float3 Normal : NORMAL;
	float3 LightDirection : LIGHTDIR;
	float3 ViewDirection : VIEWDIR;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float4 OUT = (float4)0;

	float3 normal = normalize(IN.Normal);
	float3 lightDirection = normalize(IN.LightDirection);
	float3 viewDirection = normalize(IN.ViewDirection);
	float n_dot_l = dot(normal, lightDirection);
	float3 halfVector = normalize(lightDirection + viewDirection);
	float n_dot_h = dot(normal, halfVector);

	float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
	float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);

	float3 ambient = AmbientColor.rgb * color.rgb;
	float3 diffuse = LightColor.rgb * lightCoefficients.y * color.rgb;
	float3 specular = SpecularColor * min(lightCoefficients.z, color.w);

	OUT.rgb = ambient + diffuse + specular;
	OUT.a = 1.0f;

	return OUT;
}