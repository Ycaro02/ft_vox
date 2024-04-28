#include "../../include/ft_vox.h"

/**
 * @brief Create a new cube
 * @param position cube position
 * @param size cube size
 * @param color cube color
 * @return new cube
*/
t_cube create_cube(vec3 position, vec3 size, vec3 color)
{
	t_cube cube;

	cube.position = position;
	cube.size = size;
	cube.color = color;
	return (cube);
}