#include "playground.h"

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <common/shader.hpp>

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// gameData and gameUtils
#include "gameData.h"
#include "gameUtils.h"

// some libraries for sleeping, time measurement, calculation
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <math.h>

/* library for playing music under windows
*  music files "Tetris.wav" and "TetrisIntro.wav" need to be located in the output directory and all comments starting with '//music:' need to be uncommented
*  to enable game-music support
*/
// music: #include <Windows.h>
// music: #pragma comment(lib, "winmm.lib")

using namespace gameData;
using namespace gameUtils;
using namespace std::chrono;

void staticInitVertexBuffer() {

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexbuffer); 
	glGenBuffers(1, &colorbuffer);

	//getting the background color for every single field
	float r = fieldBackgroundColor.get()[0];
	float g = fieldBackgroundColor.get()[1];
	float b = fieldBackgroundColor.get()[2];

	//iterating through every field of the preview section
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			//calculation the single field offset
			float px = 1.0f - (4 - x) * mxPreview - spacing * mxPreview;
			float py = 1.0f - (4 - y) * myPreview - spacing * myPreview;

			//transforming the data in template "vertex_buffer_single" to the vertex buffer (and inserting the colors in the color-buffer)
			for (int i = 0; i < 6; i++)
			{
				g_vertex_buffer_data[18 * (x + (3 - y) * 4) + 3 * i] = px + vertex_buffer_single[3 * i] * mxPreview * (1 - spacing * 2);
				g_vertex_buffer_data[18 * (x + (3 - y) * 4) + 3 * i + 1] = py + vertex_buffer_single[3 * i + 1] * myPreview * (1 - spacing * 2);
				g_vertex_buffer_data[18 * (x + (3 - y) * 4) + 3 * i + 2] = vertex_buffer_single[3 * i + 2];
				g_color_buffer_data[18 * (x + (3 - y) * 4) + 3 * i] = r;
				g_color_buffer_data[18 * (x + (3 - y) * 4) + 3 * i + 1] = g;
				g_color_buffer_data[18 * (x + (3 - y) * 4) + 3 * i + 2] = b;
			}
		}
	}

	//iterating through every field of the main field
	for (int y = 0; y < fieldY; y++) {
		for (int x = 0; x < fieldX; x++)
		{
			//calculation the single field offset
			float px = x * mx + spacing * mx - 1.0f;
			float py = y * my + spacing * my - 1.0f;

			//transforming the data in template "vertex_buffer_single" to the vertex buffer (and inserting the colors in the color-buffer)
			for (int i = 0; i < 6; i++)
			{
				g_vertex_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i] = px + vertex_buffer_single[3 * i] * mx * (1 - spacing * 2);
				g_vertex_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i + 1] = py + vertex_buffer_single[3 * i + 1] * my * (1 - spacing * 2);
				g_vertex_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i + 2] = vertex_buffer_single[3 * i + 2];
				g_color_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i] = r;
				g_color_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i + 1] = g;
				g_color_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i + 2] = b;
			}
		}
	}
}

void updateField(int x, int y) {
	//getting the new type from the field-array
	int type = brickDroppingField->get(x, y);

	float r, g, b;
	//getting the type-specific color
	getTypeColor(type, &r, &g, &b);

	//applying the color to the corresponding vertices
	for (int i = 0; i < 6; i++)
	{
		g_color_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i] = r;
		g_color_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i + 1] = g;
		g_color_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i + 2] = b;
	}
}

void updatePreviewField(int x, int y, int type) {
	float r, g, b;
	//getting the type-specific color
	getTypeColor(type, &r, &g, &b);
	
	//applying the color to the corresponding vertices
	for (int i = 0; i < 6; i++)
	{
		g_color_buffer_data[18 * (x + y * 4) + 3 * i] = r;
		g_color_buffer_data[18 * (x + y * 4) + 3 * i + 1] = g;
		g_color_buffer_data[18 * (x + y * 4) + 3 * i + 2] = b;
	}
}

void updateGameMechanics() {
	//handle key event of left arrow key
	if (glfwGetKey(window, GLFW_KEY_LEFT)) {
		//some time-related switch-code to loop the related action when the key is pressed
		bool run = false;
		high_resolution_clock::time_point now{ high_resolution_clock::now() };
		if (!pressedLeft) {
			//key is pressed the first time -> action needs to be executed (run = true)
			tStartKeyLeft = high_resolution_clock::now();
			pressedLeft = true;
			run = true;
		}
		else if (duration_cast<milliseconds>(now - tStartKeyLeft).count() > dTLoopKey) {
			//since the last execution of the related action 'dTLoopKey' milliseconds have passed
			// -> execute action again (run = true)
			tStartKeyLeft = now;
			run = true;
		}

		//related action (run=true) checks if the field has a brick and whether it can move this brick
		if (run && brickDroppingField->hasBrick() && brickDroppingField->canMoveBrick(BrickDroppingField::TYPE_MOVE_LEFT)) {
			//if the action is active and the brick can be moved, this movement-function is called.
			brickDroppingField->moveBrick(BrickDroppingField::TYPE_MOVE_LEFT);
		}
	}
	else pressedLeft = false;
	//works similar to the handling of GLFW_KEY_LEFT (everything called '...Left' in handling of GLFW_KEY_LEFT is replaced by '...Right' here)
	if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
		bool run = false;
		high_resolution_clock::time_point now{ high_resolution_clock::now() };
		if (!pressedRight) {
			tStartKeyRight = high_resolution_clock::now();
			pressedRight = true;
			run = true;
		}
		else if (duration_cast<milliseconds>(now - tStartKeyRight).count() > dTLoopKey) {
			tStartKeyRight = now;
			run = true;
		}

		if (run && brickDroppingField->hasBrick() && brickDroppingField->canMoveBrick(BrickDroppingField::TYPE_MOVE_RIGHT)) {
			brickDroppingField->moveBrick(BrickDroppingField::TYPE_MOVE_RIGHT);
		}
	}
	else pressedRight = false;
	
	if (glfwGetKey(window, GLFW_KEY_UP)) {
		if (!pressedRotateRight) {//rotation key is pressed the first time
			pressedRotateRight = true;
			//flag to store side effects that will be important for brick rotation (sometimes the brick may be translated to)
			int flag;
			//check if the field has a brick and this brick can be rotated
			if (brickDroppingField->hasBrick() && (flag = brickDroppingField->canRotateBrick(BrickDroppingField::TYPE_ROTATE_RIGHT))) {
				//rotate the brick
				brickDroppingField->rotateBrick(BrickDroppingField::TYPE_ROTATE_RIGHT, flag);
			}
		}
	}
	else pressedRotateRight = false;
	//whether the faster button is pressed or not
	pressedFaster = glfwGetKey(window, GLFW_KEY_DOWN);

	//measuring of elapsed time since last brick-movement (downwards)
	auto now = high_resolution_clock::now();
	long elapsed = duration_cast<milliseconds>(now - tStart).count();

	//check if enough time is elapsed
	if (elapsed >= dTDrop || pressedFaster && elapsed >= dTDrop * (1 - factorPressedFaster)) {
		
		if (brickDroppingField->hasBrick()) {//check if the field has a brick
			if (brickDroppingField->canMoveBrick(BrickDroppingField::TYPE_MOVE_DOWN)) { //check if this brick can be moved down
				brickDroppingField->moveBrick(BrickDroppingField::TYPE_MOVE_DOWN); // move brick down
			}
			else {
				//brick can't be moved -> needs to be integrated in the field

				//increasing drop-speed (by factorIncreaseSpeed)
				if (dTDrop > dTDropMax)
					dTDrop = dTDrop / factorIncreaseSpeed;
				else dTDrop = dTDropMax;


				if (brickDroppingField->placeBrick()) {//place the brick on the field (and check if the field is full)
					//counting full rows in field
					int collapseCount = 0;

					for (int y = 0; y < fieldY; y++)
					{
						//check if the row is filled
						bool filled = true;
						for (int x = 0; x < fieldX; x++) {
							if (brickDroppingField->get(x, y) == -1) {
								filled = false;
								break;
							}
						}

						//increasing collapseCount if row is filled and changing collapseConfiguration (which is needed for the collapse-animation)
						//refer to 'playground.h' for further detail on 'collapseConfiguration'
						if (filled) {
							collapseCount++;
							collapseConfiguration.get()[y] = -collapseCount;
						}
						else collapseConfiguration.get()[y] = collapseCount;

					}
					collapseRowCount = collapseCount;
					if (collapseCount > 0)//whether there are rows to collapse or not
						//switch to collapseAnimation
						setProgramState(PROGRAM_STATE_ANIMATE_COLLAPSE);
					else {
						//go on if no rows need to be collapsed
						startBrick();
					}
				}
				else {//field is full
					//music: PlaySound((LPCSTR)NULL, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
					//calling a delay of 'waitDelay' before switching to the end animation
					programStateAfterWait = PROGRAM_STATE_ANIMATE_END;
					setProgramState(PROGRAM_STATE_WAIT_DELAY);
				}

			}
		}
		//setting the time of the last drop-movement to reset the 'elapsed' time
		tStart = now;
	}

}

void setProgramState(int state) {
	if (state != programState) {
		programState = state;
		if (state == PROGRAM_STATE_GAME) {
			// if the state is switch to 'game' a new brick is started (called when the user starts the game or the collapse-animation is finished)
			startBrick();
		}
		else if (state == PROGRAM_STATE_ANIMATE_END) {//resetting the animation time for the end-animation
			tAnimationStart = high_resolution_clock::now();
			animationPhase = 0;
		}
		else if (state == PROGRAM_STATE_ANIMATE_COLLAPSE) {//resetting the animation time for the collapse-animation
			tAnimationStart = high_resolution_clock::now();
			animationPhase = 0;
		}
		else if (state == PROGRAM_STATE_IDLE) {//stopping any music if this feature is enabled
			//music: PlaySound((LPCSTR)NULL, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
		}
		else if (state == PROGRAM_STATE_WAIT_DELAY) {//resetting the wait time
			tAnimationStart = high_resolution_clock::now();
		}
	}
}

void applyTransformToSingleField(const int x, const int y, const mat3* const transform) {
	//calculating the initial coordinates of the field
	float px = x * mx + spacing * mx - 1.0f;
	float py = y * my + spacing * my - 1.0f;

	//applying the transformation to every vertex
	for (int i = 0; i < 6; i++)
	{
		//getting the coordinates of a vertex inside the field
		float x1 = vertex_buffer_single[3 * i] * mx * (1 - spacing * 2);
		float y1 = vertex_buffer_single[3 * i + 1] * my * (1 - spacing * 2);

		//transforming the vertex-coordinates (simple matrix multiplication)
		float x2 = x1 * (*transform)[0][0] + y1 * (*transform)[0][1] + (*transform)[0][2];
		float y2 = x1 * (*transform)[1][0] + y1 * (*transform)[1][1] + (*transform)[1][2];

		//setting the new values of the field
		g_vertex_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i] = px + x2;
		g_vertex_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i + 1] = py + y2;
		g_vertex_buffer_data[18 * (16 + x + (fieldY - 1 - y) * fieldX) + 3 * i + 2] = vertex_buffer_single[3 * i + 2];
	}
}

void updateAnimationCollapse() {
	//calculating the elapsed time since the start of the animation
	auto now = high_resolution_clock::now();
	long elapsed = duration_cast<milliseconds>(now - tAnimationStart).count() / (1l + (collapseRowCount - 1) / 2);
	
	//values to be modified for field-transformation
	//values that scale the field by an angle fTurn * fTurn2 (at least one of those values is 1 every time)
	float fTurn = 1, fTurn2 = 1;
	//fTrans translates the field downwards by it's value
	//fTrans2 translates the field upwards by fieldY-1-index multiplied by it's value
	//fTrans3 translates the field upwards depending on the value in collapseConfiguration 
	float fTrans = 0, fTrans2 = 0, fTrans3 = 0;

	//the animation is divided into 3 different states
	if (animationPhase == 0) {//state of collapsing the filled rows
		//progress of this state (elapsed==0 -> progress=0, elapsed==dTAnimationCollapse1 -> progress=1)
		float progress = min(1.0f, 1.0f * elapsed / dTAnimationCollapse1);
		if (progress >= 1.0) {//switching to the next program state if this animation-part is finished
			if (animationPhase == 0) {
				progress = 1.0;
				animationPhase = 1;
			}
		}

		//calculating the collapse-factor of the filled rows (between 1 and 0)
		float f = min(progress * 1.0f / (1.0f - 2 * spacing), 1);
		fTurn = max(0.0, 1 - std::sin(PI / 2 * f));

		//calculating the translation-factor of every other row (between 0 and 1) (downwards)
		fTrans = std::sin(PI / 2 * progress);
	}
	else { //state of reconstructing the field's top area
		//progress of this state (elapsed==0 -> progress=0, elapsed==dTAnimationCollapse2 -> progress=1)
		float progress = min(1.0f, 1.0f * (elapsed - dTAnimationCollapse1) / dTAnimationCollapse2);

		if (animationPhase == 1) {
			//adding a collapseRowCount-related value to the current score
			if (collapseRowCount == 1) setScore(score + POINTS_SINGLE_ROW);
			else if (collapseRowCount == 2) setScore(score + POINTS_DOUBLE_ROW);
			else if (collapseRowCount == 3) setScore(score + POINTS_TRIPLE_ROW);
			else if (collapseRowCount == 4) setScore(score + POINTS_QUADRUPLE_ROW);
			animationPhase = 2;
		}

		if (progress >= 1.0) {//switching to the next program state if this animation-part is finished
			if (animationPhase == 2) {//entered when the whole animation is finished
				//matrix to restore every field's coordinates
				mat3 transform = {
					1,0,0,
					0,1,0,
					0,0,1
				};
				//applying the matrix to every field and dropping the values in the field-array
				for (int y = 0; y < fieldY; y++)
				{
					for (int x = 0; x < fieldX; x++)
					{
						int mode = collapseConfiguration.get()[y];
						//dropping the values in the field-array
						if (mode > 0) {
							brickDroppingField->set(x, y - mode, brickDroppingField->get(x, y));
							brickDroppingField->set(x, y, -1);
						}
						//applying the matrix
						applyTransformToSingleField(x, y, &transform);
					}
				}
				//return to the main program
				setProgramState(PROGRAM_STATE_GAME);
				return;
			}
		}

		//fTrans = std::sin(progress * PI-PI/2)/2+0.5;
		fTrans = 1.0f;
		fTrans2 = 1.0f;
		fTurn2 = std::sin(progress * PI / 2);
		fTrans3 = 1 - fTurn2;
	}

	//loop to apply a transformation matrix (depending on previous values) to every row
	for (int y = 0; y < fieldY; y++)
	{
		//get the configuration for the current row
		int mode = collapseConfiguration.get()[y];
		mat3 transform;
		if (mode > 0) {//row needs to drop down
			//calculating the translation matrix depending on fTrans and mode
			transform = {
				1,0,0,
				0,1,-fTrans * my * mode,
				0,0,1
			};
		}
		else if (mode < 0) {//row needs to collapse (in first phase) and to restore (in second phase)
			if (animationPhase == 1) {
				//clearing the row (this animation state does only occur once in the whole animation and does only exist to clear rows and apply the score)
				brickDroppingField->clearRow(y);
			}

			//transforming mode to a usable form (see playground.h -> collapseConfiguration for further detail)
			mode = -mode - 1;

			//calculating the scale/translation matrix depending on mode, fTurn, fTurn2, fTrans, fTrans2 and fTrans3
			transform = {
				1,0,0,
				0,fTurn * fTurn2,my * (-fTrans * mode + fTrans2 * (fieldY - 1 - y) - fTrans3 * (collapseRowCount - 1 - mode)),
				0,0,1
			};
		}
		else continue;

		//applying the calculated transformation to every field in this row
		for (int x = 0; x < fieldX; x++)
		{
			applyTransformToSingleField(x, y, &transform);
		}
	}
}

void setScore(int sc) {
	score = sc;
	//calculating a string for the window title
	std::string strScore = std::to_string(sc);

	std::string title = "Tetris - Score: " + std::to_string(score);

	char* char_arr;
	char_arr = &title[0];

	//applying the window title
	glfwSetWindowTitle(window, char_arr);
}

void updateAnimationEnd() {
	//calculating the current progress depending on the elapsed time divided by dTAnimationEnd
	auto now = high_resolution_clock::now();
	long elapsed = duration_cast<milliseconds>(now - tAnimationStart).count();
	float progress = min(1.0f, 1.0f * elapsed / dTAnimationEnd);

	if (progress >= 0.5f) {//only called twice per animation (exactly in the middle when the width of every field is scaled to 0)
		if (animationPhase == 0) {//ensuring that every field is scaled to 0
			progress = 0.5f;
			animationPhase = 1;
		}
		else if (animationPhase == 1) {//clearing the field in the next loop cycle
			brickDroppingField->clear();
			animationPhase = 2;
		}
	}
	if (progress >= 1.0f) {//only called twice per animation (at the end)
		if (animationPhase == 2) {//ensuring that every field is scaled to it's original width (progress=1.0f in this case)
			animationPhase = 3;
		}
		else if (animationPhase == 3) {//resetting game data
			dTDrop = dTDropInit; //reset drop speed
			setScore(0);//reset score
			setProgramState(PROGRAM_STATE_IDLE); // switch to idle state
			return;
		}
	}

	//calculate the current folding to produce the effect of a rotation
	//progress=0 -> folding=1
	//progress=0.5 -> folding=0
	//progress=1 -> folding=1
	float folding = std::sin(PI / 2 - (progress > 0.5 ? 1 - progress : progress) * PI);

	//calculating the scale matrix (scaled around the center of every field)
	mat3 transform{
		folding,0,mx * (1 - spacing * 2) * (1.0f - folding) / 2.0f,
		0,1,0,
		0,0,1
	};

	//applying the matrix to every field
	for (int y = 0; y < fieldY; y++)
	{
		for (int x = 0; x < fieldX; x++)
		{
			applyTransformToSingleField(x, y, &transform);
		}
	}
}

int generateRandomBrickIndex() {
	return rand() % brickCount;
}

void startBrick() {
	brickDroppingField->startBrick(nextBrick);
	setNextBrick(generateRandomBrickIndex());
}

void setNextBrick(int type) {
	//applying the type to the corresponding global variable
	nextBrick = type;

	//getting the brick's size
	int brickSize = brickSizes.get()[type];

	//updating every single field in the preview-area
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			//for each brickSize another translation is applied to the brick (which is represented as an array)
			//the translation takes part in the if-statements by deciding whether the field needs to be set or not
			if (brickSize == 2 && y > 0 && x > 0 && y - 1 < brickSize && x - 1 < brickSize && bricks.get()[type].get()[x - 1 + (y - 1) * brickSize]) {
				updatePreviewField(x, y, type);
			}
			else if (brickSize == 3 && y > 0 && y - 1 < brickSize && x < brickSize && bricks.get()[type].get()[x + (y - 1) * brickSize]) {
				updatePreviewField(x, y, type);
			}
			else if (brickSize == 4 && y < brickSize && x < brickSize && bricks.get()[type].get()[x + y * brickSize]) {
				updatePreviewField(x, y, type);
			}
			else {//clearing the field -> invalid state
				updatePreviewField(x, y, -1);
			}
		}
	}

}

// template method main (modified)
// modified regions are marked by //<modified>, //</modified> comments
int main(void)
{
	//Initialize window
	bool windowInitialized = initializeWindow();
	if (!windowInitialized) return -1;

	//<modified>
	//seeding the random-function
	srand(time(NULL));

	//music: PlaySound((LPCSTR)"TetrisIntro.wav", NULL, SND_FILENAME | SND_ASYNC);
	
	//setting the background color
	glClearColor(backgroundColor.get()[0], backgroundColor.get()[1], backgroundColor.get()[2], 1.0f);

	//initializing the used vertex buffers
	staticInitVertexBuffer();

	//initializing the gameData-field
	initField();
	
	//initializing the animation-collapse configuration
	collapseConfiguration = { new int[fieldY] {}, std::default_delete<int[]>() };

	//adding a listener to the field to apply changes to the vertex/color-buffers
	brickDroppingField->setOnChanged([](int x, int y, int v) {
		updateField(x, y);
		});

	//setting the next brick (shown in the preview - field)
	setNextBrick(generateRandomBrickIndex());

	//setting the initial game score
	setScore(score);


	tStart = high_resolution_clock::now();

	high_resolution_clock::time_point tStartLoop = high_resolution_clock::now();
	//</modified>

	//Initialize vertex buffer
	bool vertexbufferInitialized = initializeVertexbuffer();
	if (!vertexbufferInitialized) return -1;

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	do {
		//<modified>
		//dividing the programStates by a simple if
		if (programState == PROGRAM_STATE_IDLE) {
			if (glfwGetKey(window, GLFW_KEY_SPACE)) { //space is pressed
				//music: PlaySound((LPCSTR)"Tetris.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
				//start the game
				setProgramState(PROGRAM_STATE_GAME);
			}
		}
		else if (programState == PROGRAM_STATE_GAME) {
			if (glfwGetKey(window, GLFW_KEY_P)) {//pause-button is pressed
				if (!pressedPause) {
					pressedPause = true;
					if (!pause) {
						//starting the pause and saving the current time
						pause = true;
						pauseStart = high_resolution_clock::now();
					}
					else {
						//stopping the pause and subtracting the pause-time from tStart (which represents the in-game-time)
						pause = false;
						tStart += high_resolution_clock::now() - pauseStart;
					}
				}
			}
			else pressedPause = false;
			if (!pause) {
				//updating the game mechanics if game isn't paused
				updateGameMechanics();
			}
			else {
				//receiving key events in pause
				glfwGetKey(window, GLFW_KEY_LEFT);
				glfwGetKey(window, GLFW_KEY_RIGHT);
				glfwGetKey(window, GLFW_KEY_UP);
				glfwGetKey(window, GLFW_KEY_DOWN);
			}
		}
		else if (programState == PROGRAM_STATE_ANIMATE_END) {
			//simple redirection of game-loop to updateAnimationEnd()
			updateAnimationEnd();
		}
		else if (programState == PROGRAM_STATE_WAIT_DELAY) {
			//measuring elapsed time (till tAnimationStart
			auto now = high_resolution_clock::now();
			long elapsed = duration_cast<milliseconds>(now - tAnimationStart).count();
			if (elapsed >= waitDelay) {
				//switching to the next program state when sleep is done
				setProgramState(programStateAfterWait);
			}
		}
		else if (programState == PROGRAM_STATE_ANIMATE_COLLAPSE) {
			//simple redirection of game-loop to updateAnimationCollapse()
			updateAnimationCollapse();
		}

		//consuming space events if they are not already consumed
		glfwGetKey(window, GLFW_KEY_SPACE);

		//updating buffered data
		initializeVertexbuffer();
		//</modified>
		updateAnimationLoop();
		//<modified>
		
		//delay game loop (down to one call every 5 milliseconds)
		auto now = high_resolution_clock::now();
		long elapsed = duration_cast<milliseconds>(now - tStartLoop).count();
		tStartLoop = now;
		if (elapsed < 5) {
			std::this_thread::sleep_for(milliseconds(5 - elapsed));
		}
		//</modified>
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	//Cleanup and close window
	cleanupVertexbuffer();
	glDeleteProgram(programID);
	closeWindow();

	return 0;
}

// template methods (partly modified)
// modified regions are marked by //<modified>, //</modified> comments
void updateAnimationLoop()
{
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT);

	// Use our shader
	glUseProgram(programID);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	//<modified> 2nd attribute buffer: colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 6 * (fieldX * fieldY + 16)); // (6 indices per rectangle)*(fieldcount+previewFieldCount)
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	//</modified>

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

bool initializeWindow()
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// <modified>
	// disabling window resizing
	glfwWindowHint(GLFW_RESIZABLE, false);

	// calculating window dimensions depending of the aspect ratio of the field
	// and the maximum dimensions (maxWindowSizeX and maxWindowSizeY)
	float f = 1.0f * fieldX / fieldY * 2.0f / widthMainField;

	int w = maxWindowSizeX;
	int h = w / f;
	if (h > maxWindowSizeY) {
		h = maxWindowSizeY;
		w = h * f;
	}

	//<modified>
	// Open a window and create its OpenGL context
	window = glfwCreateWindow(w, h, "Tetris", NULL, NULL);

	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return false;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	return true;
}

bool initializeVertexbuffer()
{

	// <modified>
	// moved to staticInitVertexBuffer because it only needs to be called one time
	// glGenVertexArrays(1, &VertexArrayID);
	// glBindVertexArray(VertexArrayID);

	// </modified>

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// <modified>
	
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	//</modified>

	return true;
}

bool cleanupVertexbuffer()
{
	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	//<modified> cleanup color buffer
	glDeleteBuffers(1, &colorbuffer);
	//</modified>
	glDeleteVertexArrays(1, &VertexArrayID);
	return true;
}

bool closeWindow()
{
	glfwTerminate();
	return true;
}

