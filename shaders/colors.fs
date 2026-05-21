#version 330 core
struct Light {
    vec3 position;

    vec3 ambient; // color of the ambient light, which is the light that is scattered in all directions and illuminates all objects equally, regardless of their position or orientation
    vec3 diffuse; // color of the diffuse light, which is the light that is scattered in all directions but is stronger on surfaces that are directly facing the light source
    vec3 specular; // color of the specular light, which is the light that is reflected in a specific direction and creates highlights on shiny surfaces

    float constant;
    float linear;
    float quadratic;
};

struct Material {
    sampler2D diffuse; // the diffuse texture of the material, which is the base color of the surface
    sampler2D specular; // the specular texture of the material, which is used to create highlights on shiny surfaces
    sampler2D emission; // the emission texture of the material, which is used to create a glowing effect on the surface, it adds light to the scene without being affected by the lighting calculations
    vec3 emissionColor; // the color of the emission, which is a multiplier for the emission texture, it allows you to control the color of the light emitted by the surface
    float emissionStrength; // the strength of the emission, which is a multiplier for the emission texture, it allows you to control how much light the surface emits
    float shininess; // scattering/radius of the specular highlight, the higher the number the smaller and sharper the highlight, the lower the number the larger and duller the highlight
};

uniform Light light;

uniform Material material;

out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;

uniform vec3 viewPos;

void main()
{
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    //diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos); 
    //then we get the max of the dot product and 0.0 to make sure we don't get negative values which would be the case if the light is coming from behind the surface  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;  

    // calculate the view direction and the reflection direction for specular lighting
    //you can do this in world space or in view space, but you need to make sure that the light direction and the normal are in the same space as well
    //to do in view space, you would need to transform the light position and the view position by the view matrix in the vertex shader and pass them to the fragment shader as varying variables
    vec3 viewDir = normalize(viewPos - FragPos);
    //negative light direction because we want the direction from the fragment to the light source
    vec3 reflectDir = reflect(-lightDir, norm); 

    // then we take the dot product of the view direction and the reflection direction to get the specular intensity
    //the number raises to the power of the number which increases it shininess of the surface, the higher the number the shinier the surface
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

     // 1. Fetch the raw texture color
    vec3 emTex = texture(material.emission, TexCoords).rgb;

    // 2. Find the brightness of the pixel (0.0 to 1.0)
    // This turns your green shapes into a white mask, and background stays black
    float mask = max(emTex.r, max(emTex.g, emTex.b)); 

    // 3. Multiply the mask by your new color and strength
    // Now black stays black, and the green shapes become full blue
    vec3 emission = mask * material.emissionColor * material.emissionStrength;

    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular + emission;
    FragColor = vec4(result, 1.0);
}