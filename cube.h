#ifndef HEADER_CUBE_H
#define HEADER_CUBE_H

/*
    Cube model logic from https://learnopengl.com/code_viewer.php?code=advanced/faceculling_vertexdata
    bl: Bot Left
    br: Bot Right
    tr: Top Right
    tl: Top Left
*/

/* Define cube face vertex */

/* bl-> tr -> br -> tl */
#define CUBE_BACK_FACE_VERTEX \
    {-0.5f, -0.5f, -0.5f},\
    {0.5f, 0.5f, -0.5f},\
    {0.5f, -0.5f, -0.5f},\
    {-0.5f, 0.5f, -0.5f}\

/* bl -> br -> tr -> tl */
#define CUBE_FRONT_FACE_VERTEX \
    {-0.5f, -0.5f, 0.5f},\
    {0.5f, -0.5f, 0.5f},\
    {0.5f, 0.5f, 0.5f},\
    {-0.5f, 0.5f, 0.5f}\

/* tr -> tl -> bl -> br*/
#define CUBE_LEFT_FACE_VERTEX \
    {-0.5f, 0.5f, 0.5f},\
    {-0.5f, 0.5f, -0.5f},\
    {-0.5f, -0.5f, -0.5f},\
    {-0.5f, -0.5f, 0.5f}\

/* tl -> br -> tr -> bl*/
#define CUBE_RIGHT_FACE_VERTEX \
    {0.5f, 0.5f, 0.5f},\
    {0.5f, -0.5f, -0.5f},\
    {0.5f, 0.5f, -0.5f},\
    {0.5f, -0.5f, 0.5f}\

/* tr -> tl -> bl -> br */
#define CUBE_BOTTOM_FACE_VERTEX \
    {-0.5f, -0.5f, -0.5f},\
    {0.5f, -0.5f, -0.5f},\
    {0.5f, -0.5f, 0.5f},\
    {-0.5f, -0.5f, 0.5f}\

/* tl -> br -> tr -> bl */
#define CUBE_TOP_FACE_VERTEX \
    {-0.5f, 0.5f, -0.5f},\
    {0.5f, 0.5f, 0.5f},\
    {0.5f, 0.5f, -0.5f},\
    {-0.5f, 0.5f, 0.5f}\


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

/* Define texture by cube face */

#define CUBE_FRONT_FACE_TEXTURE \
    {0.0f, 0.0f},\
    {1.0f, 0.0f},\
    {1.0f, 1.0f},\
    {0.0f, 1.0f}\

#define CUBE_BACK_FACE_TEXTURE \
    {0.0f, 0.0f},\
    {1.0f, 0.0f},\
    {1.0f, 1.0f},\
    {0.0f, 1.0f}\

#define CUBE_TOP_FACE_TEXTURE \
    {0.0f, 1.0f},\
    {1.0f, 1.0f},\
    {1.0f, 0.0f},\
    {0.0f, 0.0f}\

#define CUBE_BOTTOM_FACE_TEXTURE \
    {0.0f, 0.0f},\
    {1.0f, 0.0f},\
    {1.0f, 1.0f},\
    {0.0f, 1.0f}\

#define CUBE_LEFT_FACE_TEXTURE \
    {0.0f, 0.0f},\
    {1.0f, 0.0f},\
    {1.0f, 1.0f},\
    {0.0f, 1.0f}\

#define CUBE_RIGHT_FACE_TEXTURE \
    {0.0f, 0.0f},\
    {1.0f, 0.0f},\
    {1.0f, 1.0f},\
    {0.0f, 1.0f}\

#endif /* HEADER_CUBE_H */
