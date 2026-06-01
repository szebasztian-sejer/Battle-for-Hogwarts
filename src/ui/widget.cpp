#include <widget.h>
#include <settings.h>
#include <algorithm>
#include <uiHelpers.h>

void Widget::updateAndRenderSelf(bool visible)
{

	auto drawText = [&]()
	{
		int textWidth = MeasureText(text.c_str(), FONT_SIZE);
		int textX = drawRec.x + (drawRec.width - textWidth) / 2;
		int textY = drawRec.y + (drawRec.height - FONT_SIZE) / 2;

		DrawText(text.c_str(), textX, textY, FONT_SIZE, WHITE);
	};

	switch (type)
	{
		case WidgetTypes::Button:
		{
			if (texture)
			{
				DrawTexturePro(*texture, { 0.0f,0.0f,(float)(texture->width), (float)(texture->height) }, drawRec, { 0,0 }, 0.0f, WHITE);
			}
			else
			{
				DrawRectangleRec(drawRec, color);
			}

			drawText();

			isHovered = false;
			isClicked = false;
			isReleased = false;

			if (CheckCollisionPointRec(GetMousePosition(), drawRec))
			{
				isHovered = true;
				if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
				{
					isClicked = true;
				}
				if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
				{
					isReleased = true;
				}
			}
			break;
		}

		case WidgetTypes::Title:
		{
			drawText();
			break;
		}
		case WidgetTypes::TextBox:
		{
			int framesCounter = 0;
			bool mouseOnBox = CheckCollisionPointRec(GetMousePosition(), drawRec);
			if (mouseOnBox)
			{
				SetMouseCursor(MOUSE_CURSOR_IBEAM);

				int key = GetCharPressed();

				// Check if more characters have been pressed on the same frame
				while (key > 0)
				{
					// NOTE: Only allow keys in range [32..125]
					if ((key >= 32) && (key <= 125))
					{
						text.push_back((char)key);
					}

					key = GetCharPressed();  // Check next character in the queue
				}

				if (IsKeyPressed(KEY_BACKSPACE))
				{
					if (!text.empty()) { text.pop_back(); }
				}
			}
			else SetMouseCursor(MOUSE_CURSOR_DEFAULT);
			if (mouseOnBox) framesCounter++;
			else framesCounter = 0;

			DrawRectangleRec(drawRec, LIGHTGRAY);
			if (mouseOnBox) DrawRectangleLines((int)drawRec.x, (int)drawRec.y, (int)drawRec.width, (int)drawRec.height, RED);
			else DrawRectangleLines((int)drawRec.x, (int)drawRec.y, (int)drawRec.width, (int)drawRec.height, DARKGRAY);

			DrawText(text.c_str(), (int)drawRec.x + 5, (int)drawRec.y + 8, FONT_SIZE, MAROON);

			if (mouseOnBox)
			{
				if (((framesCounter / 20) % 2) == 0)
				{
					DrawText("_", (int)drawRec.x + 8 + MeasureText(text.c_str(), FONT_SIZE), (int)drawRec.y + 12, 40, MAROON);
				}
			}
			break;
		}

	} //end switch

}

void Widget::updateAndRenderWidgets()
{

	Rectangle oneButtonRectangle;
	oneButtonRectangle.width = drawRec.width * 0.8f;
	oneButtonRectangle.height = drawRec.height / (widgets.size() + 1);

	//so that buttons don't occupy the max ammount of space available
	oneButtonRectangle.height = std::min(oneButtonRectangle.height, oneButtonRectangle.width / 8.f);

	oneButtonRectangle = placeRectangleCenterTop(oneButtonRectangle, drawRec);
	oneButtonRectangle.y += oneButtonRectangle.height / 2.f;

	//int fontSize = (int)(oneButtonRectangle.height * 0.5f);

	for (auto& w : widgets)
	{
		w.drawRec = shrinkRectanglePercentage(oneButtonRectangle, 0.01, 0.1);
		w.updateAndRenderSelf(true);

		oneButtonRectangle.y += oneButtonRectangle.height;
	}
}

bool Widget::addButton(const std::string& name, Texture2D* t, Color c)
{
	Widget w;
	w.type = WidgetTypes::Button;
	w.name = name;
	w.text = name;
	if (t) { w.texture = t; }
	w.color = c;

	widgets.push_back(w);

	if (lastFrameWidgets.size() < widgets.size())
	{
		return false;
	}

	if (lastFrameWidgets[widgets.size() - 1].type != WidgetTypes::Button)
	{
		return false;
	}

	return lastFrameWidgets[widgets.size() - 1].isReleased;

}
void Widget::addTitle(const std::string& text)
{
	Widget w;
	w.type = WidgetTypes::Title;
	w.text = text;
	widgets.push_back(w);
}
std::string Widget::addTextBox(std::string txt)
{
	Widget w;
	w.type = WidgetTypes::TextBox;

	int index = widgets.size();

	if (lastFrameWidgets.size() > index &&
		lastFrameWidgets[index].type == WidgetTypes::TextBox)
	{
		w.text = lastFrameWidgets[index].text;
	}
	else
	{
		w.text = txt;
	}

	widgets.push_back(w);

	return w.text;
}