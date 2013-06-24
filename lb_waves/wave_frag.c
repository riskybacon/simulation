#version 150
//--------------------------------------------------------------------------------
// wave_frag.c
//
// Fragment shader for the waves
//
// CS 523 Spring 2013
// Project 3
//
// Jeff Bowles <jbowles@riskybacon.com>
//--------------------------------------------------------------------------------

in vec3 normalVector;
in vec3 lightVector;
in vec3 halfwayVector;
out vec4 fragColor;

void main (void) {
   float fogFactor = 0;
   
   vec3 normalVector1  = normalize(normalVector);
   vec3 lightVector1   = normalize(lightVector);
	vec3 halfwayVector1 = normalize(halfwayVector);
   
	vec4 c = vec4(1,1,1,1);//texture(water, tex_coord);
   
	vec4 emissiveColor = vec4(1.0, 1.0, 1.0,  1.0);
	vec4 ambientColor  = vec4(0.0, 0.65, 0.75, 1.0);
	vec4 diffuseColor  = vec4(0.5, 0.65, 0.75, 1.0);
	vec4 specularColor = vec4(1.0, 0.25, 0.0,  1.0);
   
	float emissiveWeight = 0.10;
	float ambientWeight  = 0.30;
	float diffuseWeight  = 0.30;
	float specularWeight = 0.80;
   
   // Check to see if the fragment is facing the camera.
	float d = dot(normalVector1, lightVector1);
	bool facing = d > 0.0;

   
	fragColor = emissiveColor * emissiveWeight +
               ambientColor  * ambientWeight  * c +

               // Add the diffuse component
               diffuseColor  * diffuseWeight  * c * max(d, 0) +

               // Add the specular highlight, if the fragment is facing the camera
               (facing ?
                specularColor * specularWeight * c * max(pow(dot(normalVector1, halfwayVector1), 120.0), 0.0) : vec4(0.0, 0.0, 0.0, 0.0));
   
	fragColor = fragColor * (1.0-fogFactor) + vec4(0.25, 0.75, 0.65, 1.0) * (fogFactor);
   
	fragColor.a = 1.0;
}
