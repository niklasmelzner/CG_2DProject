#include "gameUtils.h"
#include "gameData.h"
#include <memory>
#include <iostream>


using namespace gameUtils;
using namespace gameData;


namespace gameUtils {

	Field::Field(const int _sX, const int _sY) :sX(_sX), sY(_sY), onChanged(nullptr) {
		field = std::shared_ptr<int>{ new int[sX * sY],std::default_delete<int>() };
	}

	inline bool Field::contains(const int x, const int y) {
		return 0 <= x && x < sX && 0 <= y && y < sY;
	}

	BrickDroppingField::BrickDroppingField(const std::shared_ptr<Field> _field) :field(_field) {}

	int BrickDroppingField::getBrickState(const int x, const int y) {
		return brick.get()[x + (brickSize - 1 - y) * brickSize];
	}

	const int BrickDroppingField::get(const int x, const int y) {
		//check if a brick is applied and the current field is in the are of this brick
		if (brick != nullptr && brickX <= x && x < brickX + brickSize && brickY <= y && y < brickY + brickSize) {
			//transformation to brick-view coordinates
			int x1 = x - brickX;
			int y1 = y - brickY;
			//transformation to brick-template coordinates
			transformFieldToPoint(&x1, &y1, brickRot, brickSize);
			
			if (getBrickState(x1, y1)) {//checking the template-value of this brick
				return brickType;//return the brick's state
			}
			else if (debug) return -3 - field->get(x, y);//only used for debugging-types (see gameData::debug and gameData::getTypeColor)
		}

		//return the underlying field's state
		return field->get(x, y);
	}

	void BrickDroppingField::updateRegion(const int x, const int y, const int w, const int h) {
		//iterate through every field in the given region
		for (int y1 = 0; y1 < h; y1++)
		{
			for (int x1 = 0; x1 < w; x1++) {
				int x2 = x + x1;
				int y2 = y + y1;
				//call the onChanged-function if the coordinates are contained in this field
				if (field->contains(x2, y2)) {
					onChanged(x2, y2, get(x2, y2));
				}
			}
		}

	}

	void BrickDroppingField::startBrick(int _type) {
		//updating brick, brickType and brickSize
		brickType = _type;
		brick = bricks.get()[brickType];
		brickSize = brickSizes.get()[brickType];

		//calculating initial brick-coordinates (in the center above the field)
		int x = fieldX / 2 - brickSize / 2 - brickSize % 2;
		int y = fieldY;

		//dropping the brick as long as it's bottom rows are empty
		for (int y1 = brickSize - 1; y1 >= 0; y1--)
		{
			bool empty = true;
			//check if the current brick-row is empty
			for (int x1 = brickSize - 1; x1 >= 0; x1--)
			{
				if (brick.get()[x1 + brickSize * y1]) {
					empty = false;
					break;
				}
			}
			//break if not
			if (!empty)break;
			//drop
			y--;
		}

		//setting brick coordinates and rotation
		brickX = x;
		brickY = y;
		brickRot = 0;

		//updating the region, the brick has been set to
		updateRegion(x, y, 4, 4);
	}

	void BrickDroppingField::rotateBrick(int direction, int flag) {
		//applying the given direction to the brickRot-value
		if (direction == TYPE_ROTATE_LEFT) {
			brickRot = (brickRot + 3) % 4;
		}
		else if (direction == TYPE_ROTATE_RIGHT) {
			brickRot = (brickRot + 1) % 4;
		}
		//translating the brick depending on the flag's value
		if (flag == 2) {
			brickX++;
			updateRegion(brickX - 1, brickY, 5, 4);
		}
		else if (flag == 3) {
			brickX--;
			updateRegion(brickX, brickY, 5, 4);
		}
		else if (flag == 4) {
			brickX += 2;
			updateRegion(brickX - 2, brickY, 6, 4);
		}
		else if (flag == 5) {
			brickX -= 2;
			updateRegion(brickX, brickY, 6, 4);
		}
		else updateRegion(brickX, brickY, 4, 4);
	}

	void BrickDroppingField::moveBrick(int direction) {
		//applying the given direction to the brick's coordinates and updating the modified region
		if (direction == TYPE_MOVE_DOWN) {
			brickY--;
			updateRegion(brickX, brickY, brickSize, brickSize + 1);
		}
		else if (direction == TYPE_MOVE_LEFT) {
			brickX--;
			updateRegion(brickX, brickY, brickSize + 1, brickSize);
		}
		else if (direction == TYPE_MOVE_RIGHT) {
			brickX++;
			updateRegion(brickX - 1, brickY, brickSize + 1, brickSize);
		}

	}

	int BrickDroppingField::getFieldBrickState(const int x, const int y) {
		return getFieldBrickState(x, y, brickRot);
	}

	int BrickDroppingField::getFieldBrickState(const int x, const int y, const int rotation) {
		int x1 = x;
		int y1 = y;
		transformFieldToPoint(&x1, &y1, rotation, brickSize);
		return getBrickState(x1, y1);
	}

	int BrickDroppingField::canRotateBrick(int direction) {
		//applying the given direction to brickRot and and storing the result in rotation (variable brickRot is not modified by this operation)
		int rotation = 0;
		if (direction == TYPE_ROTATE_LEFT) {
			rotation = (brickRot + 3) % 4;
		}
		else if (direction == TYPE_ROTATE_RIGHT) {
			rotation = (brickRot + 1) % 4;
		}
		//check whether the brick will overlap with the field when applying it's new rotation
		//if the brick overlaps, checks for locations in x-direction are made and a flag is returned on success
		//the flag will by handed over to BrickDroppingField::rotateBrick
		if (!willOverlap(brickX, brickY, rotation)) {
			return 1;
		}
		else if (brickSize>2 &&!willOverlap(brickX + 1, brickY, rotation)) {
			return 2;
		}
		else if (brickSize > 2 && !willOverlap(brickX - 1, brickY, rotation)) {
			return 3;
		}
		else if (brickSize > 3 && !willOverlap(brickX +2, brickY, rotation)) {
			return 4;
		}
		else if (brickSize > 3 && !willOverlap(brickX - 2, brickY, rotation)) {
			return 5;
		}
		//brick can't be rotated
		return 0;
	}

	bool BrickDroppingField::willOverlap(int brickX, int brickY, int brickRot) {
		//check for every field in the brick's area
		for (int y = 0; y < brickSize; y++)
		{
			for (int x = 0; x < brickSize; x++)
			{
				//get the state of the brick (already transformed by x, y and brickRot)
				int brickState = getFieldBrickState(x, y, brickRot);

				if (brickState && y + brickY < fieldY) {
					//translating the brick to it's actual location
					int x1 = x + brickX;
					int y1 = y + brickY;

					// check if the field doesn't contain the given field or if the field is already set
					if (!field->contains(x1, y1) || field->get(x1, y1) != -1) {
						return true;
					}
				}
			}
		}
		return false;
	}

	bool BrickDroppingField::canMoveBrick(int direction) {
		//check whether the brick will overlap with the field when applying the given movement-direction
		if (direction == TYPE_MOVE_DOWN) {
			return !willOverlap(brickX, brickY - 1, brickRot);
		}
		else if (direction == TYPE_MOVE_LEFT) {
			return !willOverlap(brickX - 1, brickY, brickRot);
		}
		else if (direction == TYPE_MOVE_RIGHT) {
			return !willOverlap(brickX + 1, brickY, brickRot);
		}
		return true;
	}

	bool BrickDroppingField::placeBrick() {
		//variable to check if the brick is fully contained in the field
		bool inField = true;
		for (int y = 0; y < brickSize; y++) {
			for (int x = 0; x < brickSize; x++) {
				int brickState = getFieldBrickState(x, y);

				if (brickState) {//if brick has a value at location [x,y]
					if (field->contains(x + brickX, y + brickY)) {//if the field contains this location
						field->set(x + brickX, y + brickY, brickType);//the field is set to the brick's type
					}
					else {//the location isn't contained in the field
						inField = false;
					}
				}
			}
		}
		//deleting brick-data
		brick = nullptr;
		brickType = -1;
		//updating the modified region
		updateRegion(0, 0, fieldX, fieldY);

		return inField;
	}

	void transformFieldToPoint(int* x, int* y, const int rotation, const int brickSize) {
		//applying a matrix depending on the given rotation around the brickSize's center to x and y
		float cR = brickSize / 2.0 - 0.5;
		int x1 = rotations.get()[rotation * 4] * (*x - cR) + rotations.get()[rotation * 4 + 1] * (*y - cR) + cR;
		*y = rotations.get()[rotation * 4 + 2] * (*x - cR) + rotations.get()[rotation * 4 + 3] * (*y - cR) + cR;
		*x = x1;
	}


}

