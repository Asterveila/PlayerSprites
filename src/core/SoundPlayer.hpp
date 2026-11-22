#pragma once

#include "../data/SpritePackTypes.hpp"
#include <string>

namespace playersprites::audio {

	void playForEvent(bool platformer, std::string const& gamemode, std::string const& eventName);
	void playFile(fs::Path const& file);
	void playEntry(SpritePack const& pack, bool platformer, std::string const& gamemode, std::string const& entryId);
	bool hasUpdateSounds(std::string const& gamemode);
	void playUpdateSounds(bool platformer, std::string const& gamemode);

	void resetSequences();

	void preloadAll();
	void unloadAll();

}
