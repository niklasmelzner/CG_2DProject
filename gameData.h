#ifndef GAME_DATA_H
#define GAME_DATA_H
#include <memory>
#include "gameUtils.h";
#include <glfw3.h>
#include <windows.h>
using namespace gameUtils;

namespace gameData {

	//variables to store (effective static field-variables
	extern std::shared_ptr<Field> field; //representation of the field (without current brick)
	extern std::shared_ptr<BrickDroppingField> brickDroppingField; // combination of variable 'field' and the current brick

	//width of the main field (in opengl-coordinates)
	const float widthMainField = 1.4f;
	//width of the preview field (in opengl-coordinates)
	const float widthPreviewField = 0.5f;

	//size of the main field
	const int fieldX = 10, fieldY = 18;
	//width and height of every single field
	const float mx = widthMainField / fieldX, my = 2.0f / fieldY;
	//width and height of every single preview field
	const float mxPreview = widthPreviewField / 4, myPreview = my / mx * mxPreview;
	//spacing between single fields (spacing*2*width in x- and spacing*2*height in y-direction)
	static float spacing = 0.04f;
	//enable brick boundary highlighting (four debugging purposed only)
	constexpr bool debug = false;
	//the aspect ratio of the main field (important for window-size calculation)
	const double aspectRatio = 1.0f * fieldX / fieldY;

	//template of a rectangle represented by two triangles (those values are transformed to calculate the vertices of every single field)
	static GLfloat vertex_buffer_single[3 * 6]{
		0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};

	//vertex and color buffers (containing all vertices of the main (fieldX*fieldY) and the preview(16)-field
	static GLfloat g_vertex_buffer_data[3 * 6 * (fieldX * fieldY + 16)]{};
	static GLfloat g_color_buffer_data[3 * 6 * (fieldX * fieldY + 16)]{};

	//constants to represent the type of a field (used to calculate it's color)
	constexpr int FIELD_TYPE_EMPTY = -1, FIELD_TYPE_I = 0, FIELD_TYPE_J = 1, FIELD_TYPE_L = 2,
		FIELD_TYPE_O = 3, FIELD_TYPE_S = 4, FIELD_TYPE_T = 5, FIELD_TYPE_Z = 6;

	//bricks represented as arrays of length 4, 9 or 16
	const std::shared_ptr<int> BRICK_I{ new int[16]{
		0,0,0,0,
		1,1,1,1,
		0,0,0,0,
		0,0,0,0
	},std::default_delete<int[]>() };
	const std::shared_ptr<int> BRICK_J{ new int[9]{
		1,0,0,
		1,1,1,
		0,0,0
	},std::default_delete<int[]>() };
	const std::shared_ptr<int> BRICK_L{ new int[9]{
		0,0,1,
		1,1,1,
		0,0,0
	},std::default_delete<int[]>() };
	const std::shared_ptr<int> BRICK_O{ new int[4]{
		1,1,
		1,1
	},std::default_delete<int[]>() };
	const std::shared_ptr<int> BRICK_S{ new int[9]{
		0,1,1,
		1,1,0,
		0,0,0,
	},std::default_delete<int[]>() };
	const std::shared_ptr<int> BRICK_T{ new int[9]{
		0,1,0,
		1,1,1,
		0,0,0
	},std::default_delete<int[]>() };
	const std::shared_ptr<int> BRICK_Z{ new int[9]{
		1,1,0,
		0,1,1,
		0,0,0
	},std::default_delete<int[]>() };

	//brick count (equals size of arrays 'bricks' and 'brickSizes')
	constexpr int brickCount = 7;

	//sizes of the brick-rectangles (in one direction)
	const std::shared_ptr<int>brickSizes{
		new int[brickCount] {4, 3, 3, 2, 3, 3, 3}, 
		std::default_delete<int[]>()
	};

	//pointers to all the bricks
	const std::shared_ptr<std::shared_ptr<int>> bricks{
		new std::shared_ptr<int>[brickCount] {
			BRICK_I,
			BRICK_J,
			BRICK_L,
			BRICK_O,
			BRICK_S,
			BRICK_T,
			BRICK_Z
		},std::default_delete< std::shared_ptr<int>[]>()
	};
	
	//the game's background color
	const std::shared_ptr<float> backgroundColor{
		new float[3]{0.08f,0.08f,0.08f},
		std::default_delete<float[]>()
	};

	//the field's background color
	const std::shared_ptr<float> fieldBackgroundColor{
		new float[3]{0.13f,0.13f,0.13f},
		std::default_delete<float[]>()
	};

	//the colors for each brick
	const std::shared_ptr<float> brickColors{
		new float[3 * brickCount]{
		0.0f, 0.94f,0.94f,
		0.0f, 0.0f ,0.94f,
		0.94f,0.62f,0.0f,
		0.94f,0.94f,0.0f,
		0.0f, 0.94f,0.0f,
		0.62f,0.0f ,0.94f,
		0.94f,0.0f ,0.0f
		},std::default_delete<float[]>()
	};

	//matrices to calculate brick rotations
	const std::shared_ptr<int> rotations{
		new int[16]{
			// 0°
			1,0,
			0,1,
			// 90°
			0,-1,
			1,0,
			// 180°
			-1,0,
			0,-1,
			// 270°
			0,1,
			-1,0,
		},std::default_delete<int[]>()
	};

	/*
	* function to initialize the field;
	*/
	void initField();
	/*
	* function to get the color to a specific type
	* @param type the type to get the color from
	* @param r float, the color's red value will be applied to
	* @param g float, the color's green value will be applied to
	* @param b float, the color's blue value will be applied to
	* @param r,g,b the floats, the value data will be applied to
	*/
	void getTypeColor(const int type, float* r, float* g, float* b);

}


#endif
