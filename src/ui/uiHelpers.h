#pragma once
#include <raylib.h>

Rectangle placeRectangleTopRightCorner(Rectangle r, Rectangle dest);

Rectangle placeRectangleTopLeftCorner(Rectangle r, Rectangle dest);

Rectangle placeRectangleBottomRightCorner(Rectangle r, Rectangle dest);

Rectangle placeRectangleBottomLeftCorner(Rectangle r, Rectangle dest);

Rectangle placeRectangleCenter(Rectangle r, Rectangle dest);

Rectangle placeRectangleCenterTop(Rectangle r, Rectangle dest);

Rectangle placeRectangleCenterBottom(Rectangle r, Rectangle dest);

Rectangle placeRectangleCenterLeft(Rectangle r, Rectangle dest);

Rectangle placeRectangleCenterRight(Rectangle r, Rectangle dest);

Rectangle enlargeRectanglePixels(Rectangle r, float pixelsX, float pixelsY);

Rectangle shrinkRectanglePercentage(Rectangle r, float percentageX, float percentageY);
