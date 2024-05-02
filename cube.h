#ifndef HEADER_CUBE_H
#define HEADER_CUBE_H

#include "libft/libft.h"

/*
    Cube model logic from https://learnopengl.com/code_viewer.php?code=advanced/faceculling_vertexdata
    bl: Bot Left
    br: Bot Right
    tr: Top Right
    tl: Top Left
*/

typedef struct PACKED_STRUCT s_vertex_texture {
    vec3 position;
    vec2 texCoord;
} VertexTexture;

/* Define cube face vertex */
/* bl-> tr -> br -> tl */
#define CUBE_BACK_FACE_VERTEX \
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},\
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},\
    {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},\
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}}\

/* bl -> br -> tr -> tl */
#define CUBE_FRONT_FACE_VERTEX \
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},\
    {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},\
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},\
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}}\

/* tr -> tl -> bl -> br*/
#define CUBE_LEFT_FACE_VERTEX \
    {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},\
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},\
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},\
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}}\

/* tl -> br -> tr -> bl*/
#define CUBE_RIGHT_FACE_VERTEX \
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},\
    {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},\
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},\
    {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}}\

/* tr -> tl -> bl -> br */
#define CUBE_BOTTOM_FACE_VERTEX \
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},\
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},\
    {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},\
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}}\

/* tl -> br -> tr -> bl */
#define CUBE_TOP_FACE_VERTEX \
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},\
    {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},\
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},\
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}}\

/* Cube face index */
#define CUBE_BACK_FACE(bl, tr, br, tl) \
    {bl, tr, br},\
    {tr, bl, tl}\

#define CUBE_FRONT_FACE(bl, br, tr, tl) \
    {bl, br, tr},\
    {tr, tl, bl}\

#define CUBE_LEFT_FACE(tr, tl, bl, br) \
    {tr, tl, bl},\
    {bl, br, tr}\

#define CUBE_RIGHT_FACE(tl, br, tr, bl) \
    {tl, br, tr},\
    {br, tl, bl}\

#define CUBE_BOTTOM_FACE(tr, tl, bl, br) \
    {tr, tl, bl},\
    {bl, br, tr}\

#define CUBE_TOP_FACE(tl, br, tr, bl) \
    {tl, br, tr},\
    {br, tl, bl}\


#endif /* HEADER_CUBE_H */
