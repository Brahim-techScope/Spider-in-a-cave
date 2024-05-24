#include "spider_controller.hpp"




void SpiderController::initialize(spider* _ControlledSpider,timer_basic* _timer,input_devices& _inputs, window_structure& window){
    ControlledSpider = _ControlledSpider;
    LegPartitions = ControlledSpider->getLegPartitions();
    camera_control.initialize(_inputs, window);
    inputs = &_inputs;
    timer = _timer;

    for(int i=0;i<NUM_LEGS;i++){
        rest_displacement[i] = 0;
    }

    debug.sphere.initialize_data_on_gpu(mesh_primitive_sphere(0.1f));
    old_t = timer->t;
    position = ControlledSpider->translation;
}
bool SpiderController::stick_to_ground(collision_object* col, bool reset){
    ControlledSpider->updateGlobal();
    debug.reset_stick();
    bool allGood = true;
    for(int i=0;i<NUM_LEGS;i++){
        vec3 pos = ControlledSpider->getLegJoint(params.legs[i]);
        pos = pos + params.RestPositionDistance*(pos - ControlledSpider->translation);
        collision_ray ray(pos,-5*ControlledSpider->getUpVector());
        if(!col->does_collide(&ray,pos)){
            allGood = false;
            legPositions[params.legs[i]] = ControlledSpider->getLegPosition(params.legs[i]);
        }
        else{
            legPositions[params.legs[i]] = pos;
            ControlledSpider->setLegPosition(params.legs[i],pos);
            debug.rays_collision_pos.push_back(pos);
        }
        debug.rays_to_draw.push_back(ray);
    }
    smoothHeight(reset);
    position = ControlledSpider->translation;
    ControlledSpider->updateTranslation();
    ControlledSpider->updateRotation();
    ControlledSpider->updateGlobal();
    for(int i=0;i<NUM_LEGS;i++){
        rest_displacement[params.legs[i]] = dot(legPositions[params.legs[i]]-ControlledSpider->getRestPosition(params.legs[i]),ControlledSpider->getUpVector());
    }
    return allGood;
}

bool SpiderController::isEventTriggered(int &event_index)
{
    if(debug.debug_rest_positions){
        debug.reset_rest();
    }
    float max_variance = -1;
    int max_index = -1;
    std::vector<spider::leg> max_legs_to_move;
    bool isTriggered = false;
    for(int i=0;i<LegPartitions.size();i++){
        float variance = 0;
        std::vector<spider::leg> temp_to_move;
        for(int j=0;j<LegPartitions[i].size();j++){
            float restRadius = getRestRadius(LegPartitions[i][j]);
            vec3 restPosition = ControlledSpider->getRestPosition(LegPartitions[i][j]) + rest_displacement[LegPartitions[i][j]] * ControlledSpider->getUpVector();
            float distance = norm(restPosition - legPositions[LegPartitions[i][j]]);
            if(debug.debug_rest_positions){
                debug.rest_spheres_and_radiuses.push_back({restPosition.x,restPosition.y,restPosition.z,restRadius});
            }
            if(distance>restRadius){
                variance += pow(distance-restRadius,2);
                isTriggered = true;
            }
            if(distance>restRadius/2){
                temp_to_move.push_back(LegPartitions[i][j]);
            }
        }
        if(LegPartitions[i].size()>0){
            variance /= LegPartitions[i].size();
        }
        if(variance>max_variance){
            max_index = i;
            max_variance = variance;
            max_legs_to_move = temp_to_move;
            if(params.moveAllLegs){
                max_legs_to_move = LegPartitions[i];
            }
        }
    }
    if(isTriggered){
        event_index = max_index;
        eventQueue.legs_to_move = max_legs_to_move;
    }
    return isTriggered;
}

float SpiderController::getRestRadius(spider::leg whichLeg)
{
    if(whichLeg==spider::BackLeft){whichLeg = spider::FrontLeft;}
    if(norm(velocity)<0.01){
        return 0.07f;
    }
    return 0.2f;
}

void SpiderController::smoothHeight(bool average)
{
    vec3 total_average={0,0,0};
    vec3 left_average={0,0,0};
    vec3 right_average={0,0,0};
    vec3 front_average={0,0,0};
    vec3 back_average={0,0,0};
    int leftC=0,rightC=0,frontC=0,backC=0;
    int count = 0;
    for(int i=0;i<NUM_LEGS;i++){
        count++;
        total_average += legPositions[params.legs[i]];
        if(i%2==0){
            left_average += legPositions[params.legs[i]];
            leftC++;
        }else{
            right_average += legPositions[params.legs[i]];
            rightC++;
        }

        if(i<=1){
            front_average += legPositions[params.legs[i]];
            frontC++;
        }
        else if(i>=NUM_LEGS-2){
            back_average += legPositions[params.legs[i]];
            backC++;
        }
    }
    back_average /= backC;
    front_average /= frontC;
    left_average /= leftC;
    right_average /= rightC;


    vec3 initialUp = {0,0,1};
    vec3 upVector = cross(front_average-back_average,right_average-left_average);
    if(norm(upVector)<=0.00001){
        std::cout << "ERROR" << std::endl;
        return;
    }
    upVector = normalize(upVector);
    quaternion q; // Quaternion of rotation
    vec3 temp = cross(initialUp, upVector);
    //q.xyz() = temp;
    q.x = temp.x;
    q.y = temp.y;
    q.z = temp.z;
    q.w = sqrt(pow(norm(initialUp),2) * pow(norm(upVector),2)) + dot(initialUp, upVector);
    q = normalize(q);

    ControlledSpider->set_rotation(q);

    if(count>0){
        total_average /= count;
        if(average){
            ControlledSpider->translation = total_average + upVector*params.BodyHeight;
        }
        else{
            vec3 target_pos = total_average + upVector*params.BodyHeight;
            vec3 diff = target_pos - ControlledSpider->translation;
            float value = dot(diff,ControlledSpider->getUpVector());
            ControlledSpider->translation += value * ControlledSpider->getUpVector();
        }
        
    }
    collision_ray ray(ControlledSpider->translation,ControlledSpider->getUpVector());
    ray.color = {0.7,0,1};
    debug.rays_to_draw.push_back(ray);
}

void SpiderController::debug_draw(environment_structure environment){
    if(debug.debug_stick_to_ground){
        for(auto ray : debug.rays_to_draw){
        ray.draw(environment);
        }
        for(auto pos : debug.rays_collision_pos){
            debug.sphere.model.translation = pos;
            cgp::draw(debug.sphere,environment);
        }
    }
    if(debug.debug_rest_positions){
        for(auto sphere_params : debug.rest_spheres_and_radiuses){
            debug.sphere.model.translation = sphere_params.xyz();
            debug.sphere.model.scaling = sphere_params.w * 10;
            cgp::draw(debug.sphere,environment);
        }
    }
}

void SpiderController::update(collision_object* col){
    timer->update();
    float dt = timer->t-old_t;

    int min_iter = dt/params.maxDt;
    float leftDt = dt - min_iter*params.maxDt;
    for(int i=0;i<min_iter;i++){
        handlePosition(params.maxDt);
        handleVelocity(params.maxDt);
        animate(params.maxDt,col);
    }
    handlePosition(leftDt);
    handleVelocity(leftDt);
    animate(dt,col);
    smoothHeight();


    old_t = timer->t;
    ControlledSpider->translation = position;
    ControlledSpider->updateTranslation();
    ControlledSpider->updateRotation();
    ControlledSpider->updateGlobal();
    for(int i=0;i<NUM_LEGS;i++){
        ControlledSpider->setLegPosition(params.legs[i],legPositions[params.legs[i]]);
    }
    
}


void SpiderController::animate(float dt,collision_object* col){
    if(!eventQueue.isEvent){
        if(isEventTriggered(eventQueue.event)){
            eventQueue.isEvent = true;
            eventQueue.event_time = 0;

            smoothHeight();

            float vx = dot(ControlledSpider->getFrontVector(),velocity)/params.maxSpeed;
            float vy = dot(ControlledSpider->getRightVector(),velocity)/params.maxSpeed;
            for(auto leg : eventQueue.legs_to_move){
                vec3 restPos = ControlledSpider->getRestPosition(leg,vx,vy);
                initialLegPositions[leg] = legPositions[leg];
                collision_ray ray(restPos+params.maxLegElevation*ControlledSpider->getUpVector(),-(params.maxLegElevation-params.minLegElevation)*ControlledSpider->getUpVector());
                vec3 temp;
                if(col->does_collide(&ray,temp)){
                    targetLegPositions[leg] = temp;
                    rest_displacement[leg] = dot(temp-ControlledSpider->getRestPosition(leg),ControlledSpider->getUpVector());
                }
            }
        }
    }
    if(eventQueue.isEvent){
        float animationDuration = params.getAnimationDuration();
        eventQueue.event_time += dt;

        float fraction = eventQueue.event_time/animationDuration;
        float sin_fraction = sin(fraction*Pi);
        float cos_fraction = 1-(cos(fraction*Pi)+1)/2;
        for(auto leg : eventQueue.legs_to_move){
            legPositions[leg] = cos_fraction * targetLegPositions[leg] + (1-cos_fraction) * initialLegPositions[leg];
            legPositions[leg] += sin_fraction * params.animationHeight * ControlledSpider->getUpVector();
        }

        float timeDiff = eventQueue.event_time-animationDuration;
        if(timeDiff>0){
            eventQueue.isEvent = false;
            eventQueue.event_time = 0;
            animate(timeDiff,col);
        }
    }
}



// Controls handlers
void SpiderController::handleVelocity(float dt){
    vec3 diff = target_velocity - velocity;
    float value = norm(diff);
    if(value>0.00001){diff = normalize(diff);}
    if(value<params.acceleration*dt){
        velocity = target_velocity;
        return;
    }
    velocity = velocity + params.acceleration*diff*dt;
}
void SpiderController::handlePosition(float dt){
    position = position + dt*velocity;
}

// Controls input
void SpiderController::mouse_move_event(environment_structure &environment,input_devices& _inputs){
    if (!_inputs.keyboard.shift)
		camera_control.action_mouse_move(environment.camera_view);

    
}
void SpiderController::mouse_click_event(environment_structure &environment){
    camera_control.action_mouse_move(environment.camera_view);
}
void SpiderController::action_keyboard(environment_structure &environment){
    camera_control.action_keyboard(environment.camera_view);
}
void SpiderController::idle_frame(environment_structure &environment, collision_object* col){
    bool forward=false;
    bool backward=false;
    bool left=false;
    bool right=false;
    vec3 direction = {0,0,0};
    if(params.selected_keyboard==0){
        forward = inputs->keyboard.is_pressed('w');
        backward = inputs->keyboard.is_pressed('s');
        left = inputs->keyboard.is_pressed('a');
        right = inputs->keyboard.is_pressed('d');
    }
    else if(params.selected_keyboard==1){
        forward = inputs->keyboard.is_pressed('z');
        backward = inputs->keyboard.is_pressed('s');
        left = inputs->keyboard.is_pressed('q');
        right = inputs->keyboard.is_pressed('d');
    }
    else if(params.selected_keyboard==2){
        forward = inputs->keyboard.up;
        backward = inputs->keyboard.down;
        left = inputs->keyboard.left;
        right = inputs->keyboard.right;
    }

    if(forward){direction+=ControlledSpider->getFrontVector();}
    if(backward){direction-=ControlledSpider->getFrontVector();}
    if(right){direction+=ControlledSpider->getRightVector();}
    if(left){direction-=ControlledSpider->getRightVector();}

    if(norm(direction)!=0){direction = normalize(direction);}
    target_velocity = params.maxSpeed * direction;
    

    vec3 center = ControlledSpider->translation;
    camera_control.camera_model.distance_to_center = params.camera_max_distance;
    vec3 director = camera_control.camera_model.position()-center;
    if(norm(director)>0.00001){
        director = normalize(director); 
    }
    else{
        director = {0,1,0};
    }
    collision_ray ray(center,params.camera_max_distance*director);
    vec3 temp;
    if(col->does_collide(&ray,temp)){
        float dist = norm(temp-center);
        camera_control.camera_model.distance_to_center = dist*0.95;
    }
    camera_control.camera_model.look_at(camera_control.camera_model.position(), center, ControlledSpider->getUpVector());
    camera_control.idle_frame(environment.camera_view);
}










// ___________________________________________________________  //
// ___________________________________________________________  //
// ___________________________________________________________  //
// ___________________________________________________________  //



float SpiderController::params::getAnimationDuration()
{
    return 0.3f/animationSpeed/maxSpeed;
}

void SpiderController::debug::reset_stick(){
    rays_to_draw.clear();
    rays_collision_pos.clear();
}
void SpiderController::debug::reset_rest(){
    rest_spheres_and_radiuses.clear();
}

void SpiderController::debug::display_gui()
{
    ImGui::Checkbox("Display rest positions",&debug_rest_positions);
}


