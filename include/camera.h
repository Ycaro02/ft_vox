#ifndef HEADER_CAMERA_H
#define HEADER_CAMERA_H

// #include "world.h"
#include "../rsc/deps/cglm/include/cglm/cglm.h"
#include "../libft/libft.h"

typedef struct s_chunks Chunks;


#define ONE_CHUNK_PER_SEC 0.133333f /* 1 chunk per sec, 0.1333 * 60 = 8.0f */

#define ONE_BLOCK_PER_SEC 0.008333f 	/* 1 block per sec, 008333f * 60 = 0.5f */
#define TWO_BLOCK_PER_SEC 0.016666f 	/* 2 block per sec, 016666f * 60 = 1.0f */
#define THREE_BLOCK_PER_SEC 0.025000f 	/* 3 block per sec, 025000f * 60 = 1.5f */
#define FOUR_BLOCK_PER_SEC 0.033333f 	/* 4 block per sec, 033333f * 60 = 2.0f */
#define EIGHT_BLOCK_PER_SEC 0.066666f 	/* 8 block per sec, 066666f * 60 = 4.0f */

/* base */
// #define CAM_BASE_SPEED ONE_BLOCK_PER_SEC	/* Camera move speed */
// #define CAM_BASE_SPEED TWO_BLOCK_PER_SEC	/* Camera move speed */
#define CAM_BASE_SPEED THREE_BLOCK_PER_SEC	/* Camera move speed */
// #define CAM_BASE_SPEED ONE_CHUNK_PER_SEC	/* Camera move speed */

/* Camera Hight move speed */
#define CAM_HIGHT_SPEED (CAM_BASE_SPEED * 20.0f) 

#define CAM_UP_DOWN 0.2f			/* Move camera up/down value */
#define CAM_NEAR 	0.1f
#define CAM_FAR 	1000.0f

#define CAM_ASPECT_RATIO(width, height) (f32)((f32)width / (f32)height)

#define VEC3_ROTATEX (vec3){1.0f, 0.0f, 0.0f}
#define VEC3_ROTATEY (vec3){0.0f, 1.0f, 0.0f}
#define VEC3_ROTATEZ (vec3){0.0f, 0.0f, 1.0f}

typedef struct s_frustrum {
    vec4 planes[6]; // Les plans du frustum : 0=droite, 1=gauche, 2=bas, 3=haut, 4=proche, 5=loin
} Frustum;

typedef struct s_bounding_box {
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
	f32			camSpeed;			/* Camera speed */
	// mat4		view_no_translation;
} Camera; 

/* Frustrum */
s8			isChunkInFrustum(Mutex *gameMtx, Frustum *frustum, BoundingBox *box);
void		extractFrustumPlanes(Frustum *frustum, mat4 projection, mat4 view);
BoundingBox chunkBoundingBoxGet(s32 chunkX, s32 chunkZ, f32 chunkSize);

/* Camera */
Camera		create_camera(f32 fov, f32 aspect_ratio, f32 near, f32 far);
void		move_camera_forward(Camera* camera, float distance);
void		move_camera_backward(Camera* camera, float distance);
void		move_camera_up(Camera* camera, float distance);
void		rotate_camera(Camera* camera, float angle, vec3 axis);
void		reseCamera(void *context);
void		update_camera(void *context, GLuint shader_id);
void		straf_camera(Camera* camera, float distance, s8 dir);

void		rotateTopBot(Camera* camera, float angle);

#endif /* HEADER_CAMERA_H */
