#ifndef HEADER_CAMERA_H
#define HEADER_CAMERA_H

// #include "world.h"
#include "../rsc/deps/cglm/include/cglm/cglm.h"
#include "../libft/libft.h"

typedef struct s_chunks Chunks;


#define CAM_ZOOM 0.3f				/* Zoom/Unzoom value */
#define CAM_MOVE_HORIZONTAL 1.0f	/* Move camera horizontal value */
#define CAM_UP_DOWN 0.2f			/* Move camera up/down value */
#define ROTATE_ANGLE 2.0f			/* Rotate obj angle when arrow pressed */

#define CAM_NEAR 	0.1f
#define CAM_FAR 	1000.0f

#define CAM_ASPECT_RATIO(width, height) (f32)((f32)width / (f32)height)

#define VEC3_ROTATEX (vec3){1.0f, 0.0f, 0.0f}
#define VEC3_ROTATEY (vec3){0.0f, 1.0f, 0.0f}
#define VEC3_ROTATEZ (vec3){0.0f, 0.0f, 1.0f}

typedef struct {
    vec4 planes[6]; // Les plans du frustum : 0=droite, 1=gauche, 2=bas, 3=haut, 4=proche, 5=loin
} Frustum;

typedef struct {
    vec3 min; // Le coin inférieur gauche de la boîte
    vec3 max; // Le coin supérieur droit de la boîte
} BoundingBox;

/* Camera structure */
typedef struct s_camera {
    vec3		position;			/* position vector */
    vec3		target;				/* target vector */
    vec3		up;					/* up vector */
    mat4		view;				/* view matrix */
    mat4		projection;			/* projection matrix */
	vec3		viewVector;			/* view vector */
    vec3_s32    chunkPos;          /* Chunk position */
	Frustum		frustum;			/* Frustum */
	// mat4		view_no_translation;
} Camera; 

/* Frustrum */
s8			isChunkInFrustum(Frustum *frustum, BoundingBox *box);
void		extractFrustumPlanes(Frustum *frustum, mat4 projection, mat4 view);
BoundingBox chunkBoundingBoxGet(Chunks *chunk, f32 chunkSize, f32 cameraHeight);

/* Camera */
Camera		create_camera(float fov, float aspect_ratio, float near, float far);
void		move_camera_forward(Camera* camera, float distance);
void		move_camera_backward(Camera* camera, float distance);
void		move_camera_up(Camera* camera, float distance);
void		rotate_camera(Camera* camera, float angle, vec3 axis);
void		reseCamera(void *context);
void		update_camera(void *context, GLuint shader_id);
void		straf_camera(Camera* camera, float distance, s8 dir);

void		rotateTopBot(Camera* camera, float angle);

#endif /* HEADER_CAMERA_H */
