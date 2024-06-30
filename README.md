# FT_VOX: A Minecraft World Generator

This project is a small program designed to generate and display a large procedural 3D world made of textured cubes, inspired by Minecraft. The goal is to create a vast and natural terrain that users can explore smoothly.

## Technical Constraints

- **Procedural World Generation**: Generate a world of 16384 * 256 * 16384 cubes (256 is the height).
- **Cube Types**: Different types of cubes such as grass, ground, sand, etc.
- **Natural Terrain**: Generate hills, mountains, caves, and more with a natural topography.
- **Deterministic Generation**: Ensure the same seed produces the exact same map.
- **Dynamic Memory Management**: Save visited terrain in memory up to a limit and start deleting cubes to manage memory efficiently.
- **Graphic Rendering**: Smooth rendering with a minimal render distance of 160 cubes.
- **Texturing**: Each cube is textured, with at least 2 different textures and 2 different types of cubes.
- **Field of View (FoV)**: Set to 80 degrees to ensure an immersive experience.
- **Skybox**: Implement a skybox with seamless transitions.
- **Camera Movement**: Control the camera with the mouse on at least 2 axes and use keys to move in all directions relative to the camera's rotation.
- **Speed Control**: Adjust camera speed, with an option to increase speed by 20x for fast navigation.

## Technical Choices

- **Language**: Implemented using the C programming language.
- **Graphics Rendering**: Use OpenGL for graphics rendering.
- **Window/Event Management**: GLFW3 for window management and event handling.
- **Multithreading**: TinyCThread for multithreading support.
- **Font Rendering**: Freetype for font rendering.
- **Custom Hashmap**: Created a custom hashmap in C to handle blocks in the world.
- **Perlin Noise**: Implemented a custom 2D Perlin noise algorithm for world generation.

## Usage

To run the program, follow these steps:

1. **Clone the Repository**:
   ```bash
	git clone https://github.com/yourusername/ft_vox.git
	make
	./ft_vox
	```

## Demonstration

![First generation](rsc/gif/first_gen.gif)
