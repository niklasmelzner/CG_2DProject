#include <memory>
#include "gameData.h"

namespace gameData {

    //constructing the field
    std::shared_ptr<Field> field{ std::make_shared<Field>(fieldX , fieldY) };
    //constructing the brickDroppingField (combines static field and moving brick)
    std::shared_ptr<BrickDroppingField> brickDroppingField{ std::make_shared<BrickDroppingField>(field) };

    void initField() {
        //set every value in field to empty
        for (int x = 0; x < fieldX; x++)
        {
            for (int y = 0; y < fieldY; y++)
            {
                (*field).set(x, y, FIELD_TYPE_EMPTY);
            }
        }
    }

    void getTypeColor(const int type, float* r, float* g, float* b) {
        if (type >= 0) {//return the type color
            *r = brickColors.get()[3 * type];
            *g = brickColors.get()[3 * type + 1];
            *b = brickColors.get()[3 * type + 2];
        }
        else if (type == -2) {//return a changed background color (debugging only)
            *r = 0.3f;
            *g = 0.3f;
            *b = 0.3f;
        }
        else if (type < -2) {//return a changed type color (debugging only)
            *r = brickColors.get()[3 * (-3 - type)] / 2;
            *g = brickColors.get()[3 * (-3 - type) + 1] / 2;
            *b = brickColors.get()[3 * (-3 - type) + 2] / 2;
        }
        else {//return the background color
            *r = fieldBackgroundColor.get()[0];
            *g = fieldBackgroundColor.get()[1];
            *b = fieldBackgroundColor.get()[2];
        }
    }
}