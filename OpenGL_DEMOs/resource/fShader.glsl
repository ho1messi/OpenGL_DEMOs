#version 330

struct Material
{
	vec4 color;
	float shiness;
};

struct Light
{
	vec4 position;
	vec4 ambient;
	vec4 diffuse;
};

uniform Material material;
uniform Light    light;

in vec4 fragPos;
in vec4 glNormal;

out vec4 colorOut;

void main()
{
	vec4 ambient = light.ambient * material.color;

	vec4 normal = normalize(glNormal);
	vec4 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(lightDir, normal), 0.0f);
	vec4 diffuse = diff * light.diffuse * material.color;

	colorOut = ambient + diffuse;
}