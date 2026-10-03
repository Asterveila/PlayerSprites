#include "PackSettings.hpp"

namespace playersprites::settings {

	namespace {
		std::string packKey(std::string const& packId) {
			return fmt::format("pack.{}.enabled", packId);
		}
		std::string gamemodeKey(std::string const& packId, std::string const& gamemode) {
			return fmt::format("gamemode.{}.{}.enabled", packId, gamemode);
		}
		std::string eventKey(std::string const& packId, std::string const& gamemode, std::string const& eventId) {
			return fmt::format("event.{}.{}.{}.enabled", packId, gamemode, eventId);
		}
	}

	bool isPackEnabled(std::string const& packId) {
		return geode::Mod::get()->getSavedValue<bool>(packKey(packId), true);
	}
	void setPackEnabled(std::string const& packId, bool enabled) {
		geode::Mod::get()->setSavedValue<bool>(packKey(packId), enabled);
	}

	bool isGamemodeEnabled(std::string const& packId, std::string const& gamemode) {
		return geode::Mod::get()->getSavedValue<bool>(gamemodeKey(packId, gamemode), true);
	}
	void setGamemodeEnabled(std::string const& packId, std::string const& gamemode, bool enabled) {
		geode::Mod::get()->setSavedValue<bool>(gamemodeKey(packId, gamemode), enabled);
	}

	bool isEventEnabled(std::string const& packId, std::string const& gamemode, std::string const& eventId) {
		return geode::Mod::get()->getSavedValue<bool>(eventKey(packId, gamemode, eventId), true);
	}
	void setEventEnabled(std::string const& packId, std::string const& gamemode, std::string const& eventId, bool enabled) {
		geode::Mod::get()->setSavedValue<bool>(eventKey(packId, gamemode, eventId), enabled);
	}

}
