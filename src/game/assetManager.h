#pragma once
#include <raylib.h>
#include <vector>
#include <unordered_map>
#include <string>

struct AssetManager
{
	std::unordered_map<std::string, Font> fonts;

	void loadAll();
	void loadFonts();
};