#ifndef HEADER_CUBE_H
#define HEADER_CUBE_H

/* Define cube face vertex */

#define CUBE_FRONT_FACE_VERTEX \
    {-0.5f, -0.5f, 0.5f},\
    {0.5f, -0.5f, 0.5f},\
    {0.5f, 0.5f, 0.5f},\
    {-0.5f, 0.5f, 0.5f}\

#define CUBE_BACK_FACE_VERTEX \
    {-0.5f, -0.5f, -0.5f},\
    {0.5f, -0.5f, -0.5f},\
    {0.5f, 0.5f, -0.5f},\
    {-0.5f, 0.5f, -0.5f}\

#define CUBE_TOP_FACE_VERTEX \
    {-0.5f, 0.5f, 0.5f},\
    {0.5f, 0.5f, 0.5f},\
    {0.5f, 0.5f, -0.5f},\
    {-0.5f, 0.5f, -0.5f}\

#define CUBE_BOTTOM_FACE_VERTEX \
    {-0.5f, -0.5f, 0.5f},\
    {0.5f, -0.5f, 0.5f},\
    {0.5f, -0.5f, -0.5f},\
    {-0.5f, -0.5f, -0.5f}\

#define CUBE_LEFT_FACE_VERTEX \
    {-0.5f, -0.5f, 0.5f},\
    {-0.5f, 0.5f, 0.5f},\
    {-0.5f, 0.5f, -0.5f},\
    {-0.5f, -0.5f, -0.5f}\

#define CUBE_RIGHT_FACE_VERTEX \
    {0.5f, -0.5f, 0.5f},\
    {0.5f, 0.5f, 0.5f},\
    {0.5f, 0.5f, -0.5f},\
    {0.5f, -0.5f, -0.5f}\

/* To test */
#define CUBE_FACE_IDX {0, 1, 2, 3, 0}

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
