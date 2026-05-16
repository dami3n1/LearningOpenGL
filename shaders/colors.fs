#version 330 core
out vec4 FragColor;
in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos; 
uniform vec3 viewPos;

void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    //calculate diffuse lighting
    //by taking the dot product of the normal and the light direction vectors
    //to get a value between 0 and 1 that we can use to scale the light color
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    //then we get the max of the dot product and 0.0 to make sure we don't get negative values which would be the case if the light is coming from behind the surface  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;

    // calculate the view direction and the reflection direction for specular lighting
    //you can do this in world space or in view space, but you need to make sure that the light direction and the normal are in the same space as well
    //to do in view space, you would need to transform the light position and the view position by the view matrix in the vertex shader and pass them to the fragment shader as varying variables
    vec3 viewDir = normalize(viewPos - FragPos);
    //negative light direction because we want the direction from the fragment to the light source
    vec3 reflectDir = reflect(-lightDir, norm); 

    // then we take the dot product of the view direction and the reflection direction to get the specular intensity
    //the number raises to the power of the number which increases it shininess of the surface, the higher the number the shinier the surface
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 512);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
FragColor = vec4(result, 1.0);
}