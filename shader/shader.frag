#version 430 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightDiff, lightSpec, lightAmbi;
uniform vec3 objDiff, objSpec, objAmbi, objEmit;

void main()
{
    vec3 ambient = lightAmbi * objAmbi;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightDiff * objDiff;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightSpec * objSpec;

    vec3 result = (ambient + diffuse + specular + objEmit);
    FragColor = vec4(result, 1.0);
}