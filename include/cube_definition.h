#ifndef HEADER_CUBE_DEFINITION_H
#define HEADER_CUBE_DEFINITION_H

#define CUBE_SIZE 0.500000f 

#define BACK_FACE 0.0f
#define FRONT_FACE 1.0f
#define LEFT_FACE 2.0f
#define RIGHT_FACE 3.0f
#define BOTTOM_FACE 4.0f
#define TOP_FACE 5.0f

enum cubeFaceIndex {
	BACK_FACE_IDX=0U,
	FRONT_FACE_IDX=1U,
	LEFT_FACE_IDX=2U,
	RIGHT_FACE_IDX=3U,
	BOTTOM_FACE_IDX=4U,
	TOP_FACE_IDX=5U
};

/* Define cube face vertex */
/* bl-> tr -> br -> tl */
#define CUBE_BACK_FACE_VERTEX \
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, BACK_FACE}},\
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, BACK_FACE}},\
    {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, BACK_FACE}},\
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, BACK_FACE}}\

/* bl -> br -> tr -> tl */
#define CUBE_FRONT_FACE_VERTEX \
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, FRONT_FACE}},\
    {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, FRONT_FACE}},\
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, FRONT_FACE}},\
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, FRONT_FACE}}\

/* tr -> tl -> bl -> br*/
#define CUBE_LEFT_FACE_VERTEX \
    {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, LEFT_FACE}},\
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, LEFT_FACE}},\
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, LEFT_FACE}},\
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, LEFT_FACE}}\

/* tl -> br -> tr -> bl*/
#define CUBE_RIGHT_FACE_VERTEX \
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, RIGHT_FACE}},\
    {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, RIGHT_FACE}},\
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, RIGHT_FACE}},\
    {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, RIGHT_FACE}}\

/* tr -> tl -> bl -> br */
#define CUBE_BOTTOM_FACE_VERTEX \
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, BOTTOM_FACE}},\
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, BOTTOM_FACE}},\
    {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, BOTTOM_FACE}},\
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, BOTTOM_FACE}}\

/* tl -> br -> tr -> bl */
#define CUBE_TOP_FACE_VERTEX \
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, TOP_FACE}},\
    {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, TOP_FACE}},\
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, TOP_FACE}},\
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, TOP_FACE}}\

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

#endif /* HEADER_CUBE_DEFINITION_H */
