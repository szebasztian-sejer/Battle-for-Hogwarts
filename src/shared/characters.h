#pragma once
#include <string>

enum class CharacterID
{
	None,
	Harry,
	Ron,
	Hermione,
	Neville,
	Luna,
	CHARACTER_COUNT,
};

inline std::string getCharacterName(CharacterID charID)
{
	switch (charID)
	{
	case CharacterID::Harry:
		return "Harry Potter";
	case CharacterID::Ron:
		return "Ronald Weasley";
	case CharacterID::Hermione:
		return "Hermione";
	case CharacterID::Neville:
		return "Neville Longbottom";
	case CharacterID::Luna:
		return "Luna Lovegood";
	default:
		return "None";
	}
}