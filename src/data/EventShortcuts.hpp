#pragma once

namespace playersprites::shortcuts {

	// events that act as shortcuts for plenty of other events, so pack creators dont have to write a million events on their triggerOn/cancelOn arrays.
	// specifically made for sound effects bc the list looked fucking stupid
	inline std::vector<std::pair<std::string, std::vector<std::string>>> const& table() {
		static std::vector<std::pair<std::string, std::vector<std::string>>> const shortcuts = {
			{ "Mod:AnyLanding", { "TinyLanding", "FeatherLanding", "SoftLanding", "NormalLanding", "HardLanding" } },
			{ "Mod:BoostOrbs", { "YellowOrb", "PinkOrb", "RedOrb", "GreenOrb" } },
			{ "Mod:AnyFall", { "FallLow", "FallMed", "FallHigh", "FallVHigh" } },
			{ "Mod:FallSpeeds", { "FallSpeedLow", "FallSpeedMed", "FallSpeedHigh" } },
		};
		return shortcuts;
	}

	inline std::vector<std::string> const* expand(std::string const& name) {
		for (auto const& [shortcutName, events] : table()) {
			if (shortcutName == name) return &events;
		}
		return nullptr;
	}

	inline bool covers(std::string const& entry, std::string const& eventName) {
		if (entry == eventName) return true;
		auto* events = expand(entry);
		return events && std::find(events->begin(), events->end(), eventName) != events->end();
	}

	inline std::vector<std::string> shortcutsFor(std::string const& eventName) {
		std::vector<std::string> result;
		for (auto const& [shortcutName, events] : table()) {
			if (std::find(events.begin(), events.end(), eventName) != events.end()) {
				result.push_back(shortcutName);
			}
		}
		return result;
	}

}
