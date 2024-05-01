#include "../../include/vox.h"

/**
 * @brief Get object center
 * @param m object model
 * @param center center vector
*/
void get_obj_center(t_modelCube *cube, vec3 center) 
{
    vec3 total = {0.0f};

    for (u32 i = 0; i < cube->v_size; i++) {
        total[0] += cube->vertex[i][0];
        total[1] += cube->vertex[i][1];
        total[2] += cube->vertex[i][2];
    }
    center[0] = total[0] / cube->v_size;
    center[1] = total[1] / cube->v_size;
    center[2] = total[2] / cube->v_size;
}

FT_INLINE void mat_mult_translation(mat4 mat, vec3 translation) {
	mat4 translation_mat = {0};

	glm_translate_make(translation_mat, translation);
	glm_mat4_mul(mat, translation_mat, mat);
}

/**
 * @brief Rotate object around center
 * @param m object model
 * @param rotate_vec vector used to rotate
 * @param angle angle used to rotate
 * @param shader_id shader id for update model matrix
*/
void rotate_object_around_center(t_modelCube* cube, vec3 rotate_vec, float angle, GLuint shader_id) 
{
    vec3 obj_center = {0}, obj_center_neg = {0};
    mat4 translation_to_origin = {0}, rotation = {0};
    
	/* Find model center */
	get_obj_center(cube, obj_center);
	glm_vec3_negate_to(obj_center_neg, obj_center);

	/* Translate position, place computed center at the origin */
	glm_translate_make(translation_to_origin, obj_center_neg);

    /* Apply rotate */
	glm_rotate_make(rotation, glm_rad(angle), rotate_vec);

    /* Update obj model */
	glm_mat4_mul(translation_to_origin, cube->rotation, cube->rotation);
	glm_mat4_mul(rotation, cube->rotation, cube->rotation);
	glm_mat4_mul(translation_to_origin, cube->rotation, cube->rotation);
	mat_mult_translation(cube->rotation, obj_center);					/* Translate back */

    /* Update model matrix in shader */
    set_shader_var_mat4(shader_id, "model", cube->rotation);
}