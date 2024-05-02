
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

