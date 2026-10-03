#pragma once

#include <string>

namespace playersprites::settings {
	bool isPackEnabled(std::string const& packId);
	void setPackEnabled(std::string const& packId, bool enabled);

	bool isGamemodeEnabled(std::string const& packId, std::string const& gamemode);
	void setGamemodeEnabled(std::string const& packId, std::string const& gamemode, bool enabled);

	bool isEventEnabled(std::string const& packId, std::string const& gamemode, std::string const& eventId);
	void setEventEnabled(std::string const& packId, std::string const& gamemode, std::string const& eventId, bool enabled);

}
