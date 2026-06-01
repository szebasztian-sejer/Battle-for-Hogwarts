#include <assetManager.h>

void AssetManager::loadAll()
{
	loadFonts();
}
void AssetManager::loadFonts()
{
	fonts["Peaberry"] = LoadFont("fonts/PeaberryBase.ttf");
}