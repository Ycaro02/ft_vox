#include "../../include/world.h"
#include "../../include/cube.h"
#include "../../include/camera.h"
#include "../../include/chunks.h"

BoundingBox chunkBoundingBoxGet(s32 chunkX, s32 chunkZ, f32 chunkSize) {
    BoundingBox box;
    /* Compute bot left corner */
    glm_vec3_copy((vec3){chunkX * chunkSize, 0, chunkZ * chunkSize}, box.min);

    /* Compute top right corner */
	/* Hardcode 80.0f need to refact this */
    glm_vec3_copy((vec3){box.min[0] + chunkSize, 80.0f, box.min[2] + chunkSize}, box.max);
    return (box);
}

/* Locked before in update camera and context init */
void extractFrustumPlanes(Frustum *frustum, mat4 projection, mat4 view) {
    mat4 viewProjection;
	glm_mat4_mul(projection, view, viewProjection);

	/* Left plane */
	glm_vec4_copy((vec4) {viewProjection[0][3] + viewProjection[0][0], viewProjection[1][3] + viewProjection[1][0], viewProjection[2][3] + viewProjection[2][0], viewProjection[3][3] + viewProjection[3][0]}, frustum->planes[0]);
	/* Right plane */
	glm_vec4_copy((vec4) {viewProjection[0][3] - viewProjection[0][0], viewProjection[1][3] - viewProjection[1][0], viewProjection[2][3] - viewProjection[2][0], viewProjection[3][3] - viewProjection[3][0]}, frustum->planes[1]);
	/* Bottom plane */
	glm_vec4_copy((vec4) {viewProjection[0][3] + viewProjection[0][1], viewProjection[1][3] + viewProjection[1][1], viewProjection[2][3] + viewProjection[2][1], viewProjection[3][3] + viewProjection[3][1]}, frustum->planes[2]);
	/* Top plane */
	glm_vec4_copy((vec4) {viewProjection[0][3] - viewProjection[0][1], viewProjection[1][3] - viewProjection[1][1], viewProjection[2][3] - viewProjection[2][1], viewProjection[3][3] - viewProjection[3][1]}, frustum->planes[3]);
	/* Near plane */
	glm_vec4_copy((vec4) {viewProjection[0][3] + viewProjection[0][2], viewProjection[1][3] + viewProjection[1][2], viewProjection[2][3] + viewProjection[2][2], viewProjection[3][3] + viewProjection[3][2]}, frustum->planes[4]);
	/* Far plane */
	glm_vec4_copy((vec4) {viewProjection[0][3] - viewProjection[0][2], viewProjection[1][3] - viewProjection[1][2], viewProjection[2][3] - viewProjection[2][2], viewProjection[3][3] - viewProjection[3][2]}, frustum->planes[5]);

    /* Normalize planes */
    for (int i = 0; i < 6; i++) {
        f32 length = sqrtf(frustum->planes[i][0] * frustum->planes[i][0] +
                             frustum->planes[i][1] * frustum->planes[i][1] +
                             frustum->planes[i][2] * frustum->planes[i][2]);
        frustum->planes[i][0] /= length;
        frustum->planes[i][1] /= length;
        frustum->planes[i][2] /= length;
        frustum->planes[i][3] /= length;
		if (float_equal(length, 0.0f) == TRUE) {
			ft_printf_fd(1, RED"\nError: frustum plane divider %d is null\n"RESET, i);
		}
    }
}

s8 isChunkInFrustum(Mutex *gameMtx, Frustum *frustum, BoundingBox *box) {
    // Calculate the 8 corners of the bounding box
    vec3 corners[8];
    glm_vec3_copy((vec3){box->min[0], box->min[1], box->min[2]}, corners[0]);
    glm_vec3_copy((vec3){box->max[0], box->min[1], box->min[2]}, corners[1]);
    glm_vec3_copy((vec3){box->min[0], box->max[1], box->min[2]}, corners[2]);
    glm_vec3_copy((vec3){box->max[0], box->max[1], box->min[2]}, corners[3]);
    glm_vec3_copy((vec3){box->min[0], box->min[1], box->max[2]}, corners[4]);
    glm_vec3_copy((vec3){box->max[0], box->min[1], box->max[2]}, corners[5]);
    glm_vec3_copy((vec3){box->min[0], box->max[1], box->max[2]}, corners[6]);
    glm_vec3_copy((vec3){box->max[0], box->max[1], box->max[2]}, corners[7]);

	mtx_lock(gameMtx);

    // Check each plane
    for (s32 i = 0; i < 6; i++) {
        s32 inCount = 0;
        for (s32 j = 0; j < 8; j++) {
            if (glm_vec4_dot((vec4){corners[j][0], corners[j][1], corners[j][2], 1.0f}, frustum->planes[i]) > 0.0f)
                inCount++;
        }
        if (inCount == 0) {
			mtx_unlock(gameMtx);
            return (FALSE);
		}
    }

	mtx_unlock(gameMtx);
    return (TRUE);
}

/* ENd frustrum */