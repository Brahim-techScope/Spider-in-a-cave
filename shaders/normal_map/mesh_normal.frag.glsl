#version 330 core 

// Fragment shader - this code is executed for every pixel/fragment that belongs to a displayed shape
//
// Compute the color using Phong illumination (ambient, diffuse, specular) 
//  There is 3 possible input colors:
//    - fragment_data.color: the per-vertex color defined in the mesh
//    - material.color: the uniform color (constant for the whole shape)
//    - image_texture: color coming from the texture image
//  The color considered is the product of: fragment_data.color x material.color x image_texture
//  The alpha (/transparent) channel is obtained as the product of: material.alpha x image_texture.a
// 

// Inputs coming from the vertex shader
in struct fragment_data
{
    vec3 position; // vertex position in world space
    vec3 normal;   // normal position in world space
    mat3 TBN;	   // TBN matrix
    vec3 color;    // vertex color
    vec2 uv;       // vertex uv
} fragment;

// Output of the fragment shader - output color
layout(location=0) out vec4 FragColor;


// Uniform values that must be send from the C++ code
// ***************************************************** //

uniform sampler2D image_texture;   // Texture image identifiant
uniform sampler2D image_texture_2;

uniform mat4 view;       // View matrix (rigid transform) of the camera - to compute the camera position

uniform vec3 light; // position of the light


// Coefficients of phong illumination model
struct phong_structure {
	float ambient;      
	float diffuse;
	float specular;
	float specular_exponent;
};

// Settings for texture display
struct texture_settings_structure {
	bool use_texture;       // Switch the use of texture on/off
	bool texture_inverse_v; // Reverse the texture in the v component (1-v)
	bool two_sided;         // Display a two-sided illuminated surface (doesn't work on Mac)
};



// Material of the mesh (using a Phong model)
struct material_structure
{
	vec3 color;  // Uniform color of the object
	float alpha; // alpha coefficient

	phong_structure phong;                       // Phong coefficients
	texture_settings_structure texture_settings; // Additional settings for the texture
}; 

uniform material_structure material;


uniform int is_cartoon = 1;
uniform int cartoon_levels = 8;



uniform bool multilight = false;

uniform vec3 light_0_pos;
uniform vec3 light_1_pos;
uniform vec3 light_2_pos;
uniform vec3 light_3_pos;
uniform vec3 light_4_pos;
uniform vec3 light_5_pos;
uniform vec3 light_6_pos;
uniform vec3 light_7_pos;

uniform vec3 light_0_color;
uniform vec3 light_1_color;
uniform vec3 light_2_color;
uniform vec3 light_3_color;
uniform vec3 light_4_color;
uniform vec3 light_5_color;
uniform vec3 light_6_color;
uniform vec3 light_7_color;

uniform float light_0_dist;
uniform float light_1_dist;
uniform float light_2_dist;
uniform float light_3_dist;
uniform float light_4_dist;
uniform float light_5_dist;
uniform float light_6_dist;
uniform float light_7_dist;

uniform float light_0_intensity;
uniform float light_1_intensity;
uniform float light_2_intensity;
uniform float light_3_intensity;
uniform float light_4_intensity;
uniform float light_5_intensity;
uniform float light_6_intensity;
uniform float light_7_intensity;

struct light_params{
	vec3 position;
	vec3 color;
	float distance;
	float intensity;
};
int num_light = 8;
light_params lights[8] = light_params[](
	light_params(light_0_pos,light_0_color,light_0_dist,light_0_intensity),
	light_params(light_1_pos,light_1_color,light_1_dist,light_1_intensity),
	light_params(light_2_pos,light_2_color,light_2_dist,light_2_intensity),
	light_params(light_3_pos,light_3_color,light_3_dist,light_3_intensity),
	light_params(light_4_pos,light_4_color,light_4_dist,light_4_intensity),
	light_params(light_5_pos,light_5_color,light_5_dist,light_5_intensity),
	light_params(light_6_pos,light_6_color,light_6_dist,light_6_intensity),
	light_params(light_7_pos,light_7_color,light_7_dist,light_7_intensity)
	);


uniform float time;

vec3 computeColorWithLights(vec3 color_object,vec3 N,vec3 camera_position,vec3 fragment_position,float Ka,float Kd,float Ks,float specular_exponent){
	vec3 color_shading = Ka * color_object;
	for(int i=0;i<num_light;i++){
		// Unit direction toward the light
		vec3 L = normalize(lights[i].position-fragment.position);
		float distance = length(lights[i].position-fragment.position);

		// Diffuse coefficient
		float diffuse_component = max(dot(N,L),0.0);

		float lightIntensity = 0;
		if(lights[i].distance>=distance) {
			lightIntensity = lights[i].intensity * (lights[i].distance-distance)/lights[i].distance;
			float period = 1.0;
			float delay = 0.5*lights[i].position.y/4;
			float period_multiplier = 0.88;
			period_multiplier += 0.12*cos(period*time+delay);
			lightIntensity = lightIntensity * period_multiplier;
		}

		// Specular coefficient
		float specular_component = 0.0;
		if(diffuse_component>0.0){
			vec3 R = reflect(-L,N); // reflection of light vector relative to the normal.
			vec3 V = normalize(camera_position-fragment.position);
			specular_component = pow( max(dot(R,V),0.0), material.phong.specular_exponent );
		}
		vec3 toAdd = ( Kd * diffuse_component) * lights[i].color * color_object + Ks * specular_component * lights[i].color;
		toAdd = lightIntensity * toAdd;
		color_shading += toAdd;
	}

	return color_shading;
}



// Camera position
mat3 O = transpose(mat3(view)); // get the orientation matrix
vec3 last_col = vec3(view * vec4(0.0, 0.0, 0.0, 1.0)); // get the last column
vec3 camera_position = -O * last_col;
uniform bool has_fog;
uniform float fog_distance;
uniform vec3 fog_color;


void main()
{
	// Compute the position of the center of the camera
	mat3 O = transpose(mat3(view));                   // get the orientation matrix
	vec3 last_col = vec3(view*vec4(0.0, 0.0, 0.0, 1.0)); // get the last column
	vec3 camera_position = -O*last_col;

	

	


	// Current uv coordinates
	vec2 uv_image = vec2(fragment.uv.x, fragment.uv.y);
	if(material.texture_settings.texture_inverse_v) {
		uv_image.y = 1.0-uv_image.y;
	}
	// Get the second texture color
	vec3 color_image_texture_2 = texture(image_texture_2, uv_image).rgb;
	color_image_texture_2 = normalize(color_image_texture_2 * 2.0 - 1.0);



	// Renormalize normal

	vec3 N = normalize(fragment.normal);
	N = normalize(fragment.TBN * color_image_texture_2);
	//normalize(N+0.1*qtransform(q,vec3(2*color_image_texture_2.x-0.5,2*color_image_texture_2.y-0.5,color_image_texture_2.z*0+1)));

	// Inverse the normal if it is viewed from its back (two-sided surface)
	//  (note: gl_FrontFacing doesn't work on Mac)
	if (material.texture_settings.two_sided && gl_FrontFacing == false) {
		N = -N;
	}
	

	// Phong coefficient (diffuse, specular)
	// *************************************** //



	

	// Texture
	// *************************************** //


	// Get the current texture color
	vec4 color_image_texture = texture(image_texture, uv_image);
	if(material.texture_settings.use_texture == false) {
		color_image_texture=vec4(1.0,1.0,1.0,1.0);
	}

	// Blending of color
	// ******************************************  //


	// Blend the crack texture with a white image along the y direction

	// Finally multiply the color of the two textures
	//color_image_texture = color_image_texture * color_image_texture_2;


	
	// Compute Shading
	// *************************************** //

	// Compute the base color of the object based on: vertex color, uniform color, and texture
	vec3 color_object  = fragment.color * material.color * color_image_texture.rgb;

	// Compute the final shaded color using Phong model
	float Ka = material.phong.ambient;
	float Kd = material.phong.diffuse;
	float Ks = material.phong.specular;
	vec3 color_shading;
	
	if(!multilight){
		// Unit direction toward the light
		vec3 L = normalize(light-fragment.position);

		// Diffuse coefficient
		float diffuse_component = max(dot(N,L),0.0);

		// Specular coefficient
		float specular_component = 0.0;
		if(diffuse_component>0.0){
			vec3 R = reflect(-L,N); // reflection of light vector relative to the normal.
			vec3 V = normalize(camera_position-fragment.position);
			specular_component = pow( max(dot(R,V),0.0), material.phong.specular_exponent );
		}
		color_shading = (Ka + Kd * diffuse_component) * color_object + Ks * specular_component * vec3(1.0, 1.0, 1.0);
	
	}
	else{
		color_shading = computeColorWithLights(color_object,N,camera_position,fragment.position,Ka,Kd,Ks,material.phong.specular_exponent);
	}
	
	if(is_cartoon==1){
		float color_norm = length(color_shading);
		float final_norm = ceil(color_norm*cartoon_levels)/cartoon_levels;
		color_shading = final_norm/color_norm*color_shading;
	}
	
	// Output color, with the alpha component
    FragColor = vec4(color_shading, material.alpha * color_image_texture.a);

	if(has_fog){
		float distanceToCamera = length(camera_position-fragment.position);
		float fogEffect = (fog_distance-distanceToCamera)/fog_distance;
		float intensity = length(FragColor);
		intensity = intensity*intensity*intensity;
		fogEffect += intensity*1.5;
		fogEffect = clamp(fogEffect,0.0,1.0);
		vec4 fog_color_a = vec4(fog_color.x,fog_color.y,fog_color.z,1);
		FragColor = fogEffect*FragColor + (1-fogEffect)*fog_color_a;
	}
}
