#include "cristal.hpp"


bool cristal::texturesInitialized = false;
opengl_texture_image_structure cristal::texture_purple;
opengl_texture_image_structure cristal::texture_orange;
opengl_shader_structure cristal::cristal_shader;

cristal::cristal(){
    toDraw.material.phong.ambient=1;
    toDraw.material.phong.specular = 0.2;
    toDraw.material.phong.diffuse = 0;
    toDraw.material.phong.specular_exponent = 40;
}

void cristal::checkTextures()
{
    if(!texturesInitialized){
        texture_purple.load_and_initialize_texture_2d_on_gpu(project::path+"assets/cristal/textures/Cristals.png",GL_REPEAT,GL_REPEAT);
        texture_orange.load_and_initialize_texture_2d_on_gpu(project::path+"assets/cristal/textures/Cristals2.png",GL_REPEAT,GL_REPEAT);
        cristal_shader.load(project::path + "shaders/cristal_shader/cristal.vert.glsl", project::path + "shaders/cristal_shader/cristal.frag.glsl");
        texturesInitialized = true;
    }
}

void cristal::update(){
    toDraw.model.scaling = scaling;
    toDraw.model.scaling_xyz = scaling_xyz;
    toDraw.model.translation = translation;
    toDraw.model.rotation = rotation;

    toDraw.material.phong.ambient=1;
    toDraw.material.phong.specular = 0.2;
    toDraw.material.phong.diffuse = 0;
    toDraw.material.phong.specular_exponent = 40;
    toDraw.shader = cristal_shader;
}

void cristal::draw(environment_structure environment){    
    cgp::draw(toDraw,environment);
}

light_params cristal::getLightParams(){
    return light_params(getLightPosition(),color,distance,intensity);
}

bool cristal_ram::initialized = false;
mesh cristal_ram::cristal;
mesh_drawable cristal_ram::cristald;
void cristal_ram::chooseTexture(){
    toDraw.texture = cristal::texture_purple;
}
cristal_ram::cristal_ram(){
    intensity = 1.5;
    distance = 7;
    toDraw.shader = cristal_shader;
    color = {1,0.5,1};
}
void cristal_ram::initialize()
{
    if(!initialized){
        cristal = mesh_load_file_obj(project::path+"assets/cristal/cristals2.obj");
        cristald.initialize_data_on_gpu(cristal);
        initialized = true;
    }
    toDraw = cristald;
    checkTextures();
    chooseTexture();
}

void cristal_ram::addCollisions(collision_partition *partition){
    for(int i=0;i<cristal.connectivity.size();i++){
        uint3 indexes = cristal.connectivity[i];
        vec3 pos1 = translation + scaling * scaling_xyz * (rotation * cristal.position[indexes[0]]);
        vec3 pos2 = translation + scaling * scaling_xyz * (rotation * cristal.position[indexes[1]]);
        vec3 pos3 = translation + scaling * scaling_xyz * (rotation * cristal.position[indexes[2]]);

        partition->add_collision(new collision_triangle(pos1,pos2-pos1,pos3-pos1));
    }
}

vec3 cristal_ram::getLightPosition()
{
    vec3 up = {0,0,1};
    up = scaling_xyz * up;
    up = rotation * up;
    up = scaling * up;
    up *= 0.9;
    return translation + up;
}


bool cristal_rock::initialized = false;
mesh cristal_rock::cristal;
mesh_drawable cristal_rock::cristald;
void cristal_rock::chooseTexture(){
    toDraw.texture = cristal::texture_purple;
}
cristal_rock::cristal_rock(){
    intensity = 1.5;
    distance = 7;

    color = {1,0.5,1};
}
void cristal_rock::initialize()
{
    if(!initialized){
        cristal = mesh_load_file_obj(project::path+"assets/cristal/cristals3.obj");
        cristald.initialize_data_on_gpu(cristal);
        initialized = true;
    }
    toDraw = cristald;
    checkTextures();
    chooseTexture();
}

void cristal_rock::addCollisions(collision_partition *partition){
    for(int i=0;i<cristal.connectivity.size();i++){
        uint3 indexes = cristal.connectivity[i];
        vec3 pos1 = translation + scaling * scaling_xyz * (rotation * cristal.position[indexes[0]]);
        vec3 pos2 = translation + scaling * scaling_xyz * (rotation * cristal.position[indexes[1]]);
        vec3 pos3 = translation + scaling * scaling_xyz * (rotation * cristal.position[indexes[2]]);

        partition->add_collision(new collision_triangle(pos1,pos2-pos1,pos3-pos1));
    }
}

vec3 cristal_rock::getLightPosition()
{
    vec3 up = {0,0,1};
    up = scaling_xyz * up;
    up = rotation * up;
    up = scaling * up;
    up *= 0.9;
    return translation + up;
}




bool cristal_large::initialized = false;
mesh cristal_large::cristal;
mesh_drawable cristal_large::cristald;
void cristal_large::chooseTexture(){
    toDraw.texture = cristal::texture_purple;
}
cristal_large::cristal_large(){
    intensity = 1.5;
    distance = 7;

    color = {1,0.5,1};
}
void cristal_large::initialize()
{
    if(!initialized){
        cristal = mesh_load_file_obj(project::path+"assets/cristal/cristals4.obj");
        cristald.initialize_data_on_gpu(cristal);
        initialized = true;
    }
    toDraw = cristald;
    checkTextures();
    chooseTexture();
}

void cristal_large::addCollisions(collision_partition *partition){
    for(int i=0;i<cristal.connectivity.size();i++){
        uint3 indexes = cristal.connectivity[i];
        vec3 pos1 = translation + scaling * scaling_xyz * (rotation * cristal.position[indexes[0]]);
        vec3 pos2 = translation + scaling * scaling_xyz * (rotation * cristal.position[indexes[1]]);
        vec3 pos3 = translation + scaling * scaling_xyz * (rotation * cristal.position[indexes[2]]);

        partition->add_collision(new collision_triangle(pos1,pos2-pos1,pos3-pos1));
    }
}

vec3 cristal_large::getLightPosition()
{
    vec3 up = {0,0,1};
    up = scaling_xyz * up;
    up = rotation * up;
    up = scaling * up;
    up *= 1.05;
    return translation + up;
}

void cristal_rock_gold::chooseTexture(){
    toDraw.texture = cristal::texture_orange;
}

cristal_rock_gold::cristal_rock_gold()
{
    color = {1,0.5,0};
}

void cristal_large_gold::chooseTexture(){
    toDraw.texture = cristal::texture_orange;
}

cristal_large_gold::cristal_large_gold(){
    color = {1,0.5,0};
}

void cristal_ram_gold::chooseTexture(){
    toDraw.texture = cristal::texture_orange;
}

cristal_ram_gold::cristal_ram_gold(){
    color = {1,0.5,0};
}
