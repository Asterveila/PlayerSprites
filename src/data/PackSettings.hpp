#pragma once

#include <string>

namespace playersprites::settings {
	bool isPackEnabled(std::string const& packId);
	void setPackEnabled(std::string const& packId, bool enabled);

	bool isModeEnabled(std::string const& packId, bool platformer);
	void setModeEnabled(std::string const& packId, bool platformer, bool enabled);

	bool isPackActive(std::string const& packId, bool platformer);

	bool isGamemodeEnabled(std::string const& packId, std::string const& gamemode);
	void setGamemodeEnabled(std::string const& packId, std::string const& gamemode, bool enabled);

	bool isEventEnabled(std::string const& packId, std::string const& gamemode, std::string const& eventId);
	void setEventEnabled(std::string const& packId, std::string const& gamemode, std::string const& eventId, bool enabled);

	// sfx
	bool isSfxEnabled(std::string const& packId);
	void setSfxEnabled(std::string const& packId, bool enabled);

	bool isSoundEnabled(std::string const& packId, std::string const& gamemode, std::string const& entryId, bool defaultValue = true);
	void setSoundEnabled(std::string const& packId, std::string const& gamemode, std::string const& eventName, bool enabled);

}
