#ifndef PLAYGROUND_H
#define PLAYGROUND_H

// Include GLEW
#include <GL/glew.h>
#include <memory>
#include <chrono>
#include <glfw3.h>
#include <glm/glm.hpp>
using namespace glm;

//some global variables for handling the vertex (and color) buffer
GLuint vertexbuffer;
GLuint colorbuffer;
GLuint VertexArrayID;
GLuint vertexbuffer_size;

//program ID of the shaders, required for handling the shaders with OpenGL
GLuint programID;

using namespace std::chrono;

//constant for calculating trigonometrical transformations
constexpr double PI = 3.141592653589793238462643383279502884;

//the maximum window size
const int maxWindowSizeX = 1000;
const int maxWindowSizeY = 900;

//current score and points (depending on row count during collapse) 
int score = 0;
constexpr int POINTS_SINGLE_ROW = 20;
constexpr int POINTS_DOUBLE_ROW = 50;
constexpr int POINTS_TRIPLE_ROW = 100;
constexpr int POINTS_QUADRUPLE_ROW = 180;
//brick to be dropped next (visualized in preview-field)
int nextBrick;

//program states and current program state
const int PROGRAM_STATE_GAME = 0,
PROGRAM_STATE_ANIMATE_END = 1,
PROGRAM_STATE_IDLE = 2,
PROGRAM_STATE_WAIT_DELAY = 3,
PROGRAM_STATE_ANIMATE_COLLAPSE = 4;
int programState = PROGRAM_STATE_IDLE;

//variables and constants in relation to the game-state(s) they are used in

//--PROGRAM_STATE_GAME--

//time point to calculate the elapsed time since the last brick movement (downwards)
high_resolution_clock::time_point tStart;
//time points to measure how long related keys are pressed
//the connected action gets executed every 'dTLoopKey' milliseconds
long dTLoopKey = 135;
high_resolution_clock::time_point tStartKeyLeft;
high_resolution_clock::time_point tStartKeyRight;
high_resolution_clock::time_point tStartKeyRotateRight;
//initial drop delay (in milliseconds)
constexpr float dTDropInit = 550;
//maximum drop speed / minimum drop delay (in milliseconds)
constexpr float dTDropMax = 80;
//how fast the brick will drop (every 'dTDrop' milliseconds)
float dTDrop = dTDropInit;
//how much faster the brick will drop if the faster-key is pressed
float factorPressedFaster = 0.85;
//factor to increase speed on every brick
float factorIncreaseSpeed = 1.017;
//states of keys that are pressed or not
bool pressedLeft = false, pressedRight = false, pressedRotateRight = false, pressedFaster = false;

//--PROGRAM_STATE_GAME->pause--

//time point to calculate elapsed time during the pause
high_resolution_clock::time_point pauseStart;
//whether the game is in pause state and the pause-button is pressed or not
bool pause = false, pressedPause = false;

//--PROGRAM_STATE_ANIMATE_END || PROGRAM_STATE_ANIMATE_COLLAPSE--

//time point to calculate the progress of an animation
high_resolution_clock::time_point tAnimationStart;
//variable to store the current phase of an animation
int animationPhase;

//--PROGRAM_STATE_ANIMATE_COLLAPSE--
/*
* contains the configuration for the collapse-animation
* each value represents one row
* all the related transformation
* 
* if the value in a row is positive then this row is translated during the animation downwards by this value
* if the value in a row is negative then this row is collapsed during the animation and moved downwards by -(value+1)
* 2 -> translate 2 rows downwards
* 1 -> translate 1 row downwards
* 0 -> keep position
* -1 -> collapse
* -2 -> collapse and translate 1 row downwards
*/
std::shared_ptr<int> collapseConfiguration;
int collapseRowCount;

//how long the two phases of the collapse-animation will take (im milliseconds)
//refer to playground.cpp -> updateAnimationCollapse() for further detail
const long dTAnimationCollapse1 = 300;
const long dTAnimationCollapse2 = 140;

//--PROGRAM_STATE_ANIMATE_END--

//how long the end animation will take
const long dTAnimationEnd = 1000;


//--PROGRAM_STATE_WAIT_DELAY--

//how long the game will stay in GAME_STATE_WAIT_DELAY
const long waitDelay = 2500;
//which state the program will switch to after waiting 'waitDelay' milliseconds
int programStateAfterWait;


/*
* initialize the vertex- (and color-) buffer
* both buffers get filled with their initial values (empty fields and background-colors)
*/
void staticInitVertexBuffer();

//updating fields (in main- or preview-area)

/*
* update the state of a single field
* @param x field's x-coordinate
* @param y field's y-coordinate
*/
void updateField(int x, int y);

/*
* update the brick that is shown in the preview-field
* works similar to updateField() but also consumes the type, the field needs to be set to
* @param x field's x-coordinate
* @param y field's y-coordinate
* @param type the field's new type
*/
void updatePreviewField(int x, int y, int type);

//small helper-functions
/*
* setting the current score (and view it in the window title)
* @param sc score that will be set
*/
void setScore(int sc);

/*
* function to start a brick on the field and calculate the next Brick
*/
void startBrick();

/*
* set the next brick (visualized in the preview-field)
* @param type of the next brick
*/
void setNextBrick(int type);

/*
* switch between program states
* @param state state the program will be in next
*/
void setProgramState(int state);

/*
* apply a transformation to a single field in the main-area
* @param x field's x-coordinate
* @param y field's y-coordinate
* @param transform the transformation-matrix that will be applied to the field (in homogenous coordinates)
*/
void applyTransformToSingleField(const int x, const int y, const mat3* const transform);

/*
* generate a random index for the next brick
*/
int generateRandomBrickIndex();

//called in update-loop (depending on the game state)
/*
* called by the game loop in programState 'PROGRAM_STATE_GAME'
*/
void updateGameMechanics();

/*
* called by the game loop in programState 'PROGRAM_STATE_ANIMATION_END'
*/
void updateAnimationEnd();

/*
* called by the game loop in programState 'PROGRAM_STATE_ANIMATION_COLLAPSE'
* the collapse animation has two different phases:
*    phase1 (collapse): the filled rows get scaled to height=0 and all other fields are moved downwards to produce a 'collapsing effect'
*	 phase2 (reconstruction): the collapsed fields are translated to the top of the field (which is now empty) and rebuild to produce an effect of 'reconstruction'
*/
void updateAnimationCollapse();

int main( void ); //<<< main function, called at startup
void updateAnimationLoop(); //<<< updates the animation loop
bool initializeWindow(); //<<< initializes the window using GLFW and GLEW
bool initializeVertexbuffer(); //<<< initializes the vertex buffer array and binds it OpenGL
bool cleanupVertexbuffer(); //<<< frees all resources from the vertex buffer
bool closeWindow(); //<<< Closes the OpenGL window and terminates GLFW

#endif
