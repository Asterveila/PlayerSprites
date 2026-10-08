#pragma once

#include "../data/SpritePackTypes.hpp"
#include <string>

namespace playersprites::audio {

	void playForEvent(bool platformer, std::string const& gamemode, std::string const& eventName);
	void playFile(fs::path const& file);

	void resetSequences();

	void preloadAll();
	void unloadAll();

}
