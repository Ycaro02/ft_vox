#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"		/* Main project header */


#define OFFSET_GET(chunksID) (((chunksID / 4) * 16)) 


// #define SOUTH_OFFSET_GET(chunksID) (((chunksID % 4) * 16))

// #define EAST_OFFSET_GET(chunksID) (((chunksID % 2) * 16))

// #define WEST_OFFSET_GET(chunksID) (((chunksID % 2) * 16))

void cardinalOffsetScale(t_cardinal_offset *offset, u32 add) {
    offset->east += add;
    offset->west += add;
    offset->south += add;
    offset->north += add;
}

t_cardinal_offset chunksOffsetGet(u32 chunksID) {
    // t_cardinal_offset chunk_offset[] = {
	// 	{0, 0, 0, 0},\
	// 	{16, 0, 0, 0},\
	// 	{0, 16, 0, 0},\
	// 	{0, 0, 16, 0},\
	// 	{0, 0, 0, 16},\
	// 	{16, 0, 16, 0},\
	// 	{16, 0, 0, 16},\
	// 	{0, 16, 16, 0},\
	// 	{0, 16, 0, 16},\
	// };
	t_cardinal_offset chunk_offset[] = {
		{0, 0, 0, 0},\
		{1, 0, 0, 0},\
		{0, 1, 0, 0},\
		{0, 0, 1, 0},\
		{0, 0, 0, 1},\
		{1, 0, 1, 0},\
		{1, 0, 0, 1},\
		{0, 1, 1, 0},\
		{0, 1, 0, 1},\
	};
    static u32 offsetMax = 9;
    u32 up = 0;


    u32 id = 0;
    if (chunksID != 0) {
        id = chunksID % offsetMax;
        up = chunksID / offsetMax;
    }
    // ft_printf_fd(1, "For CID %d id: %d\n", chunksID, id);
    cardinalOffsetScale(&chunk_offset[id], up);
    return (chunk_offset[id]);
}

int main (void) {

    for (u32 i = 0; i < 20; ++i) {
        t_cardinal_offset off = chunksOffsetGet(i);
        ft_printf_fd(1, CYAN"ChunkID: |%d | "RESET"--> "YELLOW"Offset: [%d] [%d] [%d] [%d]\n"RESET, i, off.north, off.south, off.east, off.west);
    }

    return (0);
}
