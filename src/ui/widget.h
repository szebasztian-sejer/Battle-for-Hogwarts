#pragma once
#include <raylib.h>
#include <string>
#include <vector>
#include <assetManager.h>

enum class WidgetTypes
{
	None,
	Button,
	Title,
	TextBox
};

struct Widget
{
	WidgetTypes type = WidgetTypes::None;
	Rectangle drawRec = {};
	Texture2D* texture = nullptr;
	Color color = { 127,127,127,255 };
	Font font = {};

	bool isHovered = false;
	bool isClicked = false;
	bool isReleased = false;
	bool enabled = false;

	std::string name = "";
	std::string text = "";
	std::vector<Widget> widgets = {};
	std::vector<Widget> lastFrameWidgets = {};

	void updateAndRenderSelf(bool visible);
	void updateAndRenderWidgets();

	bool addButton(const std::string& name, Texture2D* t = nullptr, Color c = { 127,127,127,255 });
	void addTitle(const std::string& txt);
	std::string addTextBox(std::string txt);
};