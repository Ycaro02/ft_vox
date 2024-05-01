#ifndef HEADER_CAMERA_H
#define HEADER_CAMERA_H

#include "../libft/libft.h"

#define CAM_ZOOM 0.3f				/* Zoom/Unzoom value */
#define CAM_MOVE_HORIZONTAL 1.0f	/* Move camera horizontal value */
#define CAM_UP_DOWN 0.05f			/* Move camera up/down value */
#define ROTATE_ANGLE 4.0f			/* Rotate obj angle when arrow pressed */

#define VEC3_ROTATEX (vec3){1.0f, 0.0f, 0.0f}
#define VEC3_ROTATEY (vec3){0.0f, 1.0f, 0.0f}
#define VEC3_ROTATEZ (vec3){0.0f, 0.0f, 1.0f}

/* Camera structure */
typedef struct t_camera {
    vec3		position;			/* position vector */
    vec3		target;				/* target vector */
    vec3		up;					/* up vector */
    mat4		view;				/* view matrix */
    mat4		projection;			/* projection matrix */
} t_camera;

t_camera create_camera(float fov, float aspect_ratio, float near, float far);
void move_camera_forward(t_camera* camera, float distance);
void move_camera_backward(t_camera* camera, float distance);
void move_camera_up(t_camera* camera, float distance);
void rotate_camera(t_camera* camera, float angle, vec3 axis);
void display_camera_value(void *context);
void reset_camera(void *context);
void update_camera(void *context, GLuint shader_id);

#endif /* HEADER_CAMERA_H */
