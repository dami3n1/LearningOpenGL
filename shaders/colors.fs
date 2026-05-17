#version 330 core
struct Light {
    vec3 position; // position of the light source in world space
    vec3 ambient; // color of the ambient light, which is the light that is scattered in all directions and illuminates all objects equally, regardless of their position or orientation
    vec3 diffuse; // color of the diffuse light, which is the light that is scattered in all directions but is stronger on surfaces that are directly facing the light source
    vec3 specular; // color of the specular light, which is the light that is reflected in a specific direction and creates highlights on shiny surfaces
};

struct Material {
    vec3 ambient; // what color is reflected under ambient lighting (ususally the same as the surface color)
    vec3 diffuse; // color of surface under diffuse lighting (just like ambiet lighting) but it is sset to desired surface color
    vec3 specular; //sets the color of the specular highlight on the surface
    float shininess; // scattering/radius of the specular highlight, the higher the number the smaller and sharper the highlight, the lower the number the larger and duller the highlight
};

uniform Light light;

uniform Material material;

out vec4 FragColor;
in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos; 
uniform vec3 viewPos;

void main()
{
    vec3 ambient  = light.ambient * material.ambient; 

    //diffuse lighting
    //by taking the dot product of the normal and the light direction vectors
    //to get a value between 0 and 1 that we can use to scale the light color
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    //then we get the max of the dot product and 0.0 to make sure we don't get negative values which would be the case if the light is coming from behind the surface  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = light.diffuse * (diff * material.diffuse);

    // calculate the view direction and the reflection direction for specular lighting
    //you can do this in world space or in view space, but you need to make sure that the light direction and the normal are in the same space as well
    //to do in view space, you would need to transform the light position and the view position by the view matrix in the vertex shader and pass them to the fragment shader as varying variables
    vec3 viewDir = normalize(viewPos - FragPos);
    //negative light direction because we want the direction from the fragment to the light source
    vec3 reflectDir = reflect(-lightDir, norm); 

    // then we take the dot product of the view direction and the reflection direction to get the specular intensity
    //the number raises to the power of the number which increases it shininess of the surface, the higher the number the shinier the surface
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
   vec3 specular = light.specular * (spec * material.specular); 

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}