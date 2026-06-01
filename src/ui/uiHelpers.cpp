#include <uiHelpers.h>

Rectangle placeRectangleTopRightCorner(Rectangle r, Rectangle dest)
{
	r.x = dest.width - r.width;
	r.y = dest.y;
	return r;
}

Rectangle placeRectangleTopLeftCorner(Rectangle r, Rectangle dest)
{
	r.x = dest.x;
	r.y = dest.y;
	return r;
}

Rectangle placeRectangleBottomRightCorner(Rectangle r, Rectangle dest)
{
	r.x = dest.width - r.width;
	r.y = dest.height - r.height;
	return r;
}

Rectangle placeRectangleBottomLeftCorner(Rectangle r, Rectangle dest)
{
	r.x = dest.x;
	r.y = dest.height - r.height;
	return r;
}

Rectangle placeRectangleCenter(Rectangle r, Rectangle dest)
{
	r.x = (dest.width - r.width) / 2.0f;
	r.y = (dest.height - r.height) / 2.0f;
	return r;
}

Rectangle placeRectangleCenterTop(Rectangle r, Rectangle dest)
{
	r.x = (dest.width - r.width) / 2.0f;
	r.y = dest.y;
	return r;
}

Rectangle placeRectangleCenterBottom(Rectangle r, Rectangle dest)
{
	r.x = (dest.width - r.width) / 2.0f;
	r.y = dest.height - r.height;
	return r;
}

Rectangle placeRectangleCenterLeft(Rectangle r, Rectangle dest)
{
	r.x = dest.x;
	r.y = (dest.height - r.height) / 2.0f;
	return r;
}

Rectangle placeRectangleCenterRight(Rectangle r, Rectangle dest)
{
	r.x = dest.width - r.width;
	r.y = (dest.height - r.height) / 2.0f;
	return r;
}

Rectangle enlargeRectanglePixels(Rectangle r, float pixelsX, float pixelsY)
{
	r.width += pixelsX;
	r.height += pixelsY;

	r.x -= pixelsX / 2.f;
	r.y -= pixelsY / 2.f;

	return r;
}

Rectangle shrinkRectanglePercentage(Rectangle r, float percentageX, float percentageY)
{
	float shrinkX = r.width * percentageX;
	float shrinkY = r.height * percentageY;

	r.width -= shrinkX;
	r.height -= shrinkY;

	r.x += shrinkX / 2.f;
	r.y += shrinkY / 2.f;

	return r;
}