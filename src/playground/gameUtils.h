#ifndef GAME_UTILS_H
#define GAME_UTILS_H

#include <memory>
#include <glfw3.h>
#include <iostream>

namespace gameUtils {

	/*
	* class that wraps an array of brick-types which represent the static background of the game
	* this class provides some util functions to get/set values and listen to changes of single fields
	*/
	class Field {
		//values of this field (containing the type of every single field)
		std::shared_ptr<int> field;
		//dimensions of this field
		const int sX, sY;
		//function to be called when a field is changed
		void (*onChanged)(int, int, int);
	public:
		/*
		* creates a new field of the given dimensions
		* @param _sX, _sY the dimensions of the field
		*/
		Field(const int _sX, const int _sY);
		/*
		* change the type of a single field
		* @param x, y the field's coordinates
		* @param v the type the field wil be set to
		*/
		inline void set(const int x, const int y, const int v) {
			//set the field
			field.get()[x + y * sX] = v;
			//call the 'onChanged'-function if it's set yet
			if (onChanged != nullptr)onChanged(x, y, v);
		}
		/*
		* get the type of a single field
		* @param x, y the field's coordinates
		* @returns type of the field
		*/
		inline const int get(const int x, const int y) {
			return field.get()[x + y * sX];
		}
		/*
		* set which function will be called when a field changes
		* @param _onChanged function that will be called with f(x,y,type) on every field-change
		*/
		inline void setOnChanged(void (*_onChanged)(int, int, int)) {
			onChanged = _onChanged;
		}
		/*
		* checks whether the field contains the given coordinates
		* @param x, y the coordinates to be checked
		* @returns whether the coordinates are contained in this field or not
		*/
		inline bool contains(const int x, const int y);
		/*
		* clear the whole field
		*/
		inline void clear() {
			//clear every single field
			for (int y = 0; y < sY; y++)
				for (int x = 0; x < sX; x++)
					set(x, y, -1);
		}
		/*
		* clear a single row
		* @param y index of the row that will be cleared
		*/
		inline void clearRow(static int y) {
			for (int x = 0; x < sX; x++)
				set(x, y, -1);
		}
	};

	/*
	* class that wraps a gameUtils::Field object and projects a dynamic brick on it's output
	* this class is an extended interface for 'gameUtils::Field' and further implements:
	*  - brick-transformations (translation and rotation)
	*  - collision detection (whether an operation is possible or not)
	*  - a combined view of background ('field') and current brick
	*  - a listener that is directly connected to the update-methods of the main field
	*/
	class BrickDroppingField {
		//object this class wraps around
		const std::shared_ptr<Field> field;
		/*
		* function to be called when a field needs to be updated
		* (on brick position/rotation change or changes of the underlying field)
		*/
		void (*onChanged)(int, int, int) = nullptr;
		//current brick pointer and brick-data
		std::shared_ptr<int> brick = nullptr;
		int brickType = 0, brickSize = 0;
		//current brick translation and rotation
		int brickX = 0, brickY = 0, brickRot = 0;
		/*
		* update the view of a given region
		* @param x region's x-coordinate
		* @param y region's y-coordinate
		* @param w region's width
		* @param h region's height
		*/
		void updateRegion(const int x, const int y, const int w, const int h);
		/*
		* get the template brick-state of the current brick
		* @param x brick's field x-coordinate
		* @param y brick's field y-coordinate
		* @returns template brick state
		*/
		int getBrickState(int x, int y);
		/*
		* get the brick state on a viewport-field
		* (translation and rotation of the brick is taken into account)
		* @param x field's x coordinate
		* @param y field's y coordinate
		*/
		int getFieldBrickState(const int x, const  int y);
		/*
		* get the brick state on a viewport-field
		* (translation and rotation of the brick is taken into account)
		* @param x field's x coordinate
		* @param y field's y coordinate
		* @param rotation brick's rotation
		*/
		int getFieldBrickState(const int x, const  int y, const int rotation);
		/*
		* check if a given brick-configuration will overlap with the underlying field
		* @param brickX brick's x-offset
		* @param brickY brick's y-offset
		* @param brickRot brick's rotation
		* @return whether the configuration overlaps with the field or not
		*/
		bool willOverlap(int brickX, int brickY, int brickRot);
	public:
		/*
		* redirection method to Field::clear()
		*/
		inline void clear() {
			field->clear();
		}
		/*
		* redirection method to Field::clearRow(y)
		* @param y row's index
		*/
		inline void clearRow(static int y) {
			field->clearRow(y);
		}
		/*
		* movement and rotation types
		*/
		const static int TYPE_MOVE_DOWN = 0;
		const static int TYPE_MOVE_LEFT = 1;
		const static int TYPE_MOVE_RIGHT = 2;
		const static int TYPE_ROTATE_LEFT = 3;
		const static int TYPE_ROTATE_RIGHT = 4;
		/*
		* constructs a BrickDroppingField
		* @param _field gameUtils::Field to be wrapped by this class
		*/
		BrickDroppingField(const std::shared_ptr<Field> _field);
		/*
		* get type of a single field
		* returns the brick-type if the brick overlaps with the field or redirects to Field::get(x,y)
		* @param x, y the field's coordinates
		* @returns type of the field
		*/
    const int get(const int x, const int y);
		/*
		* redirection method to Field::set()
		*/
		inline void set(const int x, const int y, const int v) {
			field->set(x, y, v);
		}
		/*
		* set the brick of this field and initialize it's attributes
		* @param type brick's type
		*/
		void startBrick(int type);
		/*
		* translate the current brick by one unit into the given direction
		* @param direction movement type
		*/
		void moveBrick(int direction);
		/*
		* translate the current brick by one unit into the given direction
		* @param direction movement type
		* @param flag returned by canRotateBrick(direction) to apply side effects (normally translation)
		*/
		void rotateBrick(int direction, int flag);
		/*
		* check whether a brick is set to this field or not
		* @return if a brick is set
		*/
		bool hasBrick() {
			return brick != nullptr;
		}
		/*
		* check if movement of the current brick in the given direction is allowed
		* @param direction direction the brick may be moved to
		*/
		bool canMoveBrick(int direction);
		/*
		* check if movement of the current brick in the given direction is allowed
		* @param direction direction the brick may be moved to
		* @returns flag that is 0 when the brick can't be moved and may otherwise contain side effects pf this movement
		*/
		int canRotateBrick(int direction);

		/*
		* places the brick in the underlying gameUtils::Field object
		* @return whether the brick was placed inside the field's are or not
		*/
		bool placeBrick();

		/*
		* redirection method to Field::setOnChanged()
		* and further support for brick-change events
		*/
		inline void setOnChanged(void (*_onChanged)(int, int, int)) {
			field->setOnChanged(_onChanged);
			onChanged = _onChanged;
		}

	};

	/*
	* translates brick-view-coordinates to brick-template-coordinates
	* @param x brick's view x coordinate
	* @param y brick's view y coordinate
	* @param rotation brick's rotation
	* @param brickSize brick's size (width and height)
	*/
	void transformFieldToPoint(int* x, int* y, const int rotation, const int brickSize);
}

using namespace gameUtils;



#endif

