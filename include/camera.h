#ifndef HEADER_CAMERA_H
#define HEADER_CAMERA_H

#include "../libft/libft.h"

#define CAM_ZOOM 0.3f				/* Zoom/Unzoom value */
#define CAM_MOVE_HORIZONTAL 1.0f	/* Move camera horizontal value */
#define CAM_UP_DOWN 0.05f			/* Move camera up/down value */
#define ROTATE_ANGLE 4.0f			/* Rotate obj angle when arrow pressed */

#define VEC3_ROTATEX (vec3_f32){1.0f, 0.0f, 0.0f}
#define VEC3_ROTATEY (vec3_f32){0.0f, 1.0f, 0.0f}
#define VEC3_ROTATEZ (vec3_f32){0.0f, 0.0f, 1.0f}

/* Camera structure */
typedef struct t_camera {
    vec3_f32		position;			/* position vector */
    vec3_f32		target;				/* target vector */
    vec3_f32		up;					/* up vector */
    mat4_f32		view;				/* view matrix */
    mat4_f32		projection;			/* projection matrix */
} t_camera;

#endif /* HEADER_CAMERA_H */
