
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



// struct compacBlock {
// 	u32 shape_texture;
// 	/*
// 		u16 shape;
// 		u16 texture;
// 	*/
// 	u32 y_light;
// 	/*
// 		u8 flag;
// 		u8 torch_sun_light;  4 bits for torch light and 4 bits for sun light
// 		u16 y;
// 	*/
// 	u32 x;		/* x position */
// 	u32 z;		/* z position */
// };


// enum AtlasID {
//     AT_BREAK_STONE=0,     /* Break stone */
//     AT_BREAK_STONE1,        /* Break stone 1 */
//     AT_BREAK_STONE2,        /* Break stone 2 */
//     AT_BREAK_STONE3,        /* Break stone 3 */
//     AT_BREAK_STONE4,        /* Break stone 4 */
//     AT_BREAK_STONE5,        /* Break stone 5 */
//     AT_BREAK_STONE6,        /* Break stone 6 */
//     AT_BREAK_STONE7,        /* Break stone 7 */
//     AT_BREAK_STONE8,        /* Break stone 8 */
//     AT_BREAK_STONE9,        /* Break stone 9 */
//     AT_WOOL_RED=10,            /* Wool red */
//     AT_WOOL_ORANGE,         /* Wool orange */
//     AT_WOOL_YELLOW,         /* Wool yellow */
//     AT_WOOL_PURPLE,     /* Wool lime */
//     AT_WOOL_GREEN,          /* Wool green */
//     AT_WOOL_CYAN,           /* Wool cyan */
//     AT_WOOL_LIGHT_BLUE,     /* Wool light blue */
//     AT_WOOL_BLUE,           /* Wool blue */
//     AT_WOOL_PURPLE,         /* Wool purple */
//     AT_WOOL_MAGENTA,        /* Wool magenta */
//     AT_WOOL_LIGHTMAGENTA,   /* Wool magenta */
//     AT_WOOL_PINK,           /* Wool pink */
//     AT_WOOL_DARKPINK,       /* Wool pink */
//     AT_WOOL_GRAY,           /* Wool gray */
//     AT_WOOL_LIGHT_GRAY,     /* Wool light gray */
//     AT_WOOL_WHITE,          /* Wool white */
//     AT_SPONGE=26,           /* Sponge */
//     AT_GLASS,               /* Glass */
// 	AT_EMPTYTOREMOVE=28,	/* Empty to remove */
//     AT_FULL_WHITE_START,    /* Full white start */
//     AT_FULL_GOLD_START,     /* Full gold start */
//     AT_GOLD_ORE=31,         /* Gold ore */
//     AT_IRON_ORE,            /* Iron ore */
//     AT_COAL_ORE,            /* Coal ore */
//     AT_BOOKSHELF,           /* Bookshelf */
//     AT_STONE_WITH_VINES,    /* Stone with vines */
//     AT_OBSIDIAN,            /* Obsidian */
//     AT_SEMI_FULL_WHITE,     /* Semi full white */
//     AT_SEMI_FULL_GOLD,      /* Semi full gold */
//     AT_COBBLESTONE=39,      /* Cobblestone */
//     AT_BEDROCK,             /* Bedrock */
//     AT_SAND,                /* Sand */
//     AT_GRAVEL,              /* Gravel */
//     AT_LOG_SIDE,            /* Log side */
//     AT_LOG_INTERIOR,        /* Log interior */
//     AT_LEAVES,				/* Leaves */
//     AT_FULL_WHITE=46,       /* Full white */
// 	AT_FULLGOLD=47,			/* Full gold */
// 	AT_SHROOM_REDW=48,		/* Red White mushroom */
// 	AT_SHROOM_BROWN=49,		/* Brown mushroom */
// 	AT_LAVE=50,				/* Lava */
// 	AT_GRASS_TOP=51,		/* Grass top */
// 	AT_GRASS_TOP2=52,		/* Grass top */
// 	AT_STONE=53,			/* Stone */
// 	AT_DIRT=54,				/* Dirt */
// 	AT_GRASS_SIDE=55,		/* Grass side */
// 	AT_WOOD_PLANK=56,		/* Wood plank */
// 	AT_STONE_CUT=57,		/* Stone cut */
// 	AT_FULLGREY=58,			/* Full grey */
// 	AT_BRICK=59,			/* Brick */
// 	AT_TNT_SIDE=60,			/* TNT side */
// 	AT_TNT_TOP=61,			/* TNT top */
// 	AT_TNT_BOTTOM=62,		/* TNT bottom */
// 	AT_SPIDERWEB=63,		/* Spider web */
// 	AT_ROSE=64,				/* Rose */
// 	AT_SUNFLOWER=65,		/* Sunflower */
// 	AT_WATER=66,			/* Water */
// 	AT_LITTLE_TREE=67,		/* Little tree */
// };

// enum BlockType {
//     AIR=0,								/* Air block (Empty) */
//     STONE=53,							/* Stone block (Underground) */
//     DIRT=54,							/* Dirt block (Exposed to light) */
// 	GRASS=55,							/* Grass block (Exposed to light) */
// 	GRASS_TOP=AT_GRASS_TOP,				/* Grass top */
// 	WATER=AT_WATER,						/* Water */
// 	WOOL_RED=AT_WOOL_RED,				/* Wool red */
// 	WOOL_ORANGE=AT_WOOL_ORANGE,			/* Wool orange */
// 	WOOL_YELLOW=AT_WOOL_YELLOW,			/* Wool yellow */
// 	WOOL_PURPLE=AT_WOOL_PURPLE,	/* Wool lime */
// 	WOOL_GREEN=AT_WOOL_GREEN,			/* Wool green */
// 	WOOL_CYAN=AT_WOOL_CYAN,				/* Wool cyan */
// 	WOOL_LIGHT_BLUE=AT_WOOL_LIGHT_BLUE,	/* Wool light blue */
// 	WOOL_BLUE=AT_WOOL_BLUE,				/* Wool blue */
// 	GLASS=AT_GLASS,						/* Glass */
// };
