#include "../../include/vox.h"
#include "../../include/cube.h"

void chunkPosGet(Camera *camera)
{
	f32 chunkSize = 8.0; // cubeSize is 0.5
	camera->chunkPos[0]= floor(camera->position[0] / chunkSize);
	camera->chunkPos[1] = floor(camera->position[1] / chunkSize);
	camera->chunkPos[2] = floor(camera->position[2] / chunkSize);
}

/**
 * @brief Display camera value
 * @param cam camera structure
*/
void display_camera_value(void *context)
{
	Context *c = context;
	ft_printf_fd(1, CYAN"Camera position: %f %f %f\n", c->cam.position[0], c->cam.position[1], c->cam.position[2]);
	ft_printf_fd(1, "Camera target: %f %f %f\n", c->cam.target[0], c->cam.target[1], c->cam.target[2]);
	ft_printf_fd(1, "Camera up: %f %f %f\n", c->cam.up[0], c->cam.up[1], c->cam.up[2]);
	ft_printf_fd(1, "Camera view: \n");
	for (u32 i = 0; i < 4; i++) {
		ft_printf_fd(1, "%f %f %f %f\n", c->cam.view[i][0], c->cam.view[i][1], c->cam.view[i][2], c->cam.view[i][3]);
	}
	ft_printf_fd(1, "Camera projection: \n");
	for (u32 i = 0; i < 4; i++) {
		ft_printf_fd(1, "%f %f %f %f\n", c->cam.projection[i][0], c->cam.projection[i][1], c->cam.projection[i][2], c->cam.projection[i][3]);
	}
	ft_printf_fd(1, RESET);

	ft_printf_fd(1, "Model matrix: \n");
	for (u32 i = 0; i < 4; i++) {
		ft_printf_fd(1, "%f %f %f %f\n", c->cube.rotation[i][0], c->cube.rotation[i][1], c->cube.rotation[i][2], c->cube.rotation[i][3]);
	}
}

/**
 * @brief Create a new camera
 * @param fov field of view
 * @param aspect_ratio aspect ratio
 * @param near near plane
 * @param far far plane
 * @return new camera
*/
Camera create_camera(float fov, float aspect_ratio, float near, float far)
{
    Camera camera;

    /* init camera position */
	glm_vec3_copy((vec3){0.0f, 10.0f, 0.0f}, camera.position);
    /* init camera target */
	glm_vec3_copy((vec3){1.0f, 10.0f, 1.0f}, camera.target);
    /* init up vector */
	glm_vec3_copy((vec3){0.00000f, 1.00000f, 0.00000f}, camera.up);

    /* Compute view martice */
    /* Look at view */
	glm_lookat(camera.position, camera.target, camera.up, camera.view);

    /* Compute projection matrice */
	glm_perspective(glm_rad(fov), aspect_ratio, near, far, camera.projection);

    return (camera);
}

/**
 * @brief Update camera
 * @param camera camera to update
 * @param shader_id shader id
*/
void update_camera(void *context, GLuint shader_id) 
{
	Context *c = context;


    /* Look at view */
	glm_lookat(c->cam.position, c->cam.target, c->cam.up, c->cam.view);

    set_shader_var_mat4(shader_id, "view", c->cam.view);
    set_shader_var_mat4(shader_id, "projection", c->cam.projection);
	set_shader_var_mat4(shader_id, "model", c->cube.rotation);
	
	chunkPosGet(&c->cam);
}

/**
 * @brief Move camera forward
 * @param camera camera to move
 * @param distance distance to move
*/
void move_camera_forward(Camera* camera, float distance) 
{
    vec3 direction;

    glm_vec3_sub(camera->target, camera->position, direction); /* tocheck */

	/* Plane direction vector */
	direction[1] = 0.0f;

    glm_vec3_normalize(direction);
    glm_vec3_scale(direction, distance, direction);
	glm_vec3_add(camera->position, direction, camera->position);
	glm_vec3_add(camera->target, direction, camera->target);
}

/**
 * @brief Move camera backward
 * @param camera camera to move
 * @param distance distance to move
*/
void move_camera_backward(Camera* camera, float distance) {
    move_camera_forward(camera, -distance);
}


void straf_camera(Camera* camera, float distance, s8 dir) {
	vec3 direction, right;

	/* Compute direction vector */
	glm_vec3_sub(camera->target, camera->position, direction);
	
	/* Compute right vector and normalise it */
	glm_vec3_cross(direction, camera->up, right);
	glm_vec3_normalize(right);

	/* Compute up movement vector, normalise and scale it */
	// glm_vec3_normalize(right);
	glm_vec3_scale(right, distance, right);

	if (dir == DIR_LEFT) {
		glm_vec3_negate(right);
	}
	glm_vec3_add(camera->position, right, camera->position); /* move up camera */
	glm_vec3_add(camera->target, right, camera->target); /* move up target */
}


/**
 * @brief Move camera left
 * @param camera camera to move
 * @param distance distance to move
 * @param axis axis to move
*/
void rotateTopBot(Camera* camera, float angle) {
    mat4 rotation = GLM_MAT4_IDENTITY_INIT;
    vec3 axis;

    /* Rotate the direction vector from the position to the target */
    vec3 direction;
    glm_vec3_sub(camera->target, camera->position, direction);

    /* Calculate the rotation axis */
    glm_cross(camera->up, direction, axis);

    /* Create rotation matrix */
    glm_rotate(rotation, glm_rad(angle), axis);

    // mat4_mult_vec3(rotation, direction, 1.0f, direction);
    glm_mat4_mulv3(rotation, direction, 1.0f, direction);

	/* Check if the new direction is too high or too low */
    if (direction[1] > 1.0f || direction[1] < -1.0f) {
        return;
    }


    /* Update the target based on the rotated direction */
    glm_vec3_add(camera->position, direction, camera->target);
}

void rotate_camera(Camera *camera, float angle, vec3 axis) {
    mat4 rotation = GLM_MAT4_IDENTITY_INIT;

    /* Create rotation matrix */
	glm_rotate(rotation, glm_rad(angle), axis);

    /* Rotate the direction vector from the position to the target */
    vec3 direction;
	glm_vec3_sub(camera->target, camera->position, direction);

	glm_mat4_mulv3(rotation, direction, 1.0f, direction);

    /* Update the target based on the rotated direction */
	glm_vec3_add(camera->position, direction, camera->target);
}

/**
 * @brief Move camera up
 * @param camera camera to move
 * @param distance distance to move
*/
void move_camera_up(Camera *camera, float distance) 
{
    vec3 direction, right, up_movement, up = {0.0f, 1.0f, 0.0f};
    
	/* Compute direction vector */
	glm_vec3_sub(camera->target, camera->position, direction);
	
	/* Compute right vector and normalise it */
    glm_vec3_cross(direction, up, right);
	glm_vec3_normalize(right);

	/* Compute up movement vector, normalise and scale it */
	glm_vec3_cross(right, direction, up_movement);
	glm_vec3_normalize(up_movement);
	glm_vec3_scale(up_movement, distance, up_movement);

    glm_vec3_add(camera->position, up_movement, camera->position); /* move up camera */
    glm_vec3_add(camera->target, up_movement, camera->target); /* move up target */
}

/* Hard code camera postition */
void reseCamera(void *context)
{
	Context *c = context;

    /* init camera position */
	glm_vec3_copy((vec3){0.0f, 10.0f, 0.0f}, c->cam.position);
    /* init camera target */
	glm_vec3_copy((vec3){1.0f, 10.0f, 1.0f}, c->cam.target);
    /* init up vector */
	glm_vec3_copy((vec3){0.00000f, 1.00000f, 0.00000f}, c->cam.up);

	/* Look at view */
	glm_lookat(c->cam.position, c->cam.target, c->cam.up, c->cam.view);
	glm_mat4_identity(c->cube.rotation);
}