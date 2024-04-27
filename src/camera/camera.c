#include "../../include/vox.h"

/**
 * @brief Display camera value
 * @param cam camera structure
*/
void display_camera_value(t_camera *cam)
{
	ft_printf_fd(1, CYAN"Camera position: %f %f %f\n", cam->position[0], cam->position[1], cam->position[2]);
	ft_printf_fd(1, "Camera target: %f %f %f\n", cam->target[0], cam->target[1], cam->target[2]);
	ft_printf_fd(1, "Camera up: %f %f %f\n", cam->up[0], cam->up[1], cam->up[2]);
	ft_printf_fd(1, "Camera view: \n");
	for (u32 i = 0; i < 4; i++) {
		ft_printf_fd(1, "%f %f %f %f\n", cam->view[i][0], cam->view[i][1], cam->view[i][2], cam->view[i][3]);
	}
	ft_printf_fd(1, "Camera projection: \n");
	for (u32 i = 0; i < 4; i++) {
		ft_printf_fd(1, "%f %f %f %f\n", cam->projection[i][0], cam->projection[i][1], cam->projection[i][2], cam->projection[i][3]);
	}
	ft_printf_fd(1, RESET);
}

/**
 * @brief Create a new camera
 * @param fov field of view
 * @param aspect_ratio aspect ratio
 * @param near near plane
 * @param far far plane
 * @return new camera
*/
t_camera create_camera(float fov, float aspect_ratio, float near, float far) 
{
    t_camera camera;

    /* init camera position */
    CREATE_VEC3(23.756206f, 0.00000f, 2.155274f, camera.position);
    /* init camera target */
    CREATE_VEC3(14.963508f, 0.00000f, 1.405361f, camera.target);
    /* init up vector */
    CREATE_VEC3(0.00000f, 1.00000f, 0.00000f, camera.up);

    /* Compute view martice */
    update_view_mat4(camera.position, camera.target, camera.up, camera.view);

    /* Compute projection matrice */
    mat4_perspective(deg_to_rad(fov), aspect_ratio, near, far, camera.projection);
    return (camera);
}

/**
 * @brief Update camera
 * @param camera camera to update
 * @param shader_id shader id
*/
void update_camera(t_camera* camera, GLuint shader_id) 
{
    update_view_mat4(camera->position, camera->target, camera->up, camera->view);
    set_shader_var_mat4(shader_id, "view", camera->view);
    // set_shader_var_mat4(shader_id, "model", camera->model); /* need to be updated */
    set_shader_var_mat4(shader_id, "projection", camera->projection);
}

/**
 * @brief Move camera forward
 * @param camera camera to move
 * @param distance distance to move
*/
void move_camera_forward(t_camera* camera, float distance) 
{
    vec3_f32 direction;

    vec3_sub(camera->target, camera->position, direction); /* tocheck */
    vec3_normalize(direction);
    vec3_scale(direction, distance, direction);
    VECTOR_ADD(float, 3, camera->position, direction);
    VECTOR_ADD(float, 3, camera->target, direction);
}

/**
 * @brief Move camera backward
 * @param camera camera to move
 * @param distance distance to move
*/
void move_camera_backward(t_camera* camera, float distance) {
    move_camera_forward(camera, -distance);
}

/**
 * @brief Move camera left
 * @param camera camera to move
 * @param distance distance to move
 * @param axis axis to move
*/
void rotate_camera(t_camera* camera, float angle, vec3_f32 axis) {
    mat4_f32 rotation;

    /* Create rotation matrix */
	mat4_rotate(rotation, deg_to_rad(angle), axis);

    /* Rotate the direction vector from the position to the target */
    vec3_f32 direction;
	vec3_sub(camera->target, camera->position, direction);
	mat4_mult_vec3(rotation, direction, 1.0f, direction);

    /* Update the target based on the rotated direction */
	vec3_add(camera->position, direction, camera->target);
}

/**
 * @brief Move camera up
 * @param camera camera to move
 * @param distance distance to move
*/
void move_camera_up(t_camera* camera, float distance) 
{
    vec3_f32 direction, right, up_movement, up = {0.0f, 1.0f, 0.0f};
    
	/* Compute direction vector */
	vec3_sub(camera->target, camera->position, direction);
	
	/* Compute right vector and normalise it */
    vec3_cross(direction, up, right);
	vec3_normalize(right);

	/* Compute up movement vector, normalise and scale it */
	vec3_cross(right, direction, up_movement);
	vec3_normalize(up_movement);
	vec3_scale(up_movement, distance, up_movement);

    vec3_add(camera->position, up_movement, camera->position); /* move up camera */
    vec3_add(camera->target, up_movement, camera->target); /* move up target */
}

/* Hard code camera postition */
void reset_camera(t_camera *camera) 
{
    /* init camera position */
    CREATE_VEC3(23.756206f, 0.00000f, 2.155274f, camera->position);
    /* init camera target */
    CREATE_VEC3(14.963508f, 0.00000f, 1.405361f, camera->target);
    /* init up vector */
    CREATE_VEC3(0.00000f, 1.00000f, 0.00000f, camera->up);

	update_view_mat4(camera->position, camera->target, camera->up, camera->view);
}