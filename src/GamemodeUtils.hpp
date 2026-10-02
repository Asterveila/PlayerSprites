#pragma once

#include <Geode/binding/PlayerObject.hpp>
#include <Geode/Enums.hpp>
#include <enchantum/enchantum.hpp>

namespace playersprites {

	inline std::string gameEventName(GJGameEvent event) {
		for (auto const& [value, name] : enchantum::entries_generator<GJGameEvent>) {
			if (value == event) return std::string(name);
		}
		return "";
	}

	inline std::string currentGamemodeName(PlayerObject* player) {
		if (player->m_isRobot) return "Robot";
		if (player->m_isSpider) return "Spider";
		if (player->m_isSwing) return "Swing";
		if (player->m_isDart) return "Wave";
		if (player->m_isBird) return "UFO";
		if (player->m_isShip) return "Ship";
		if (player->m_isBall) return "Ball";
		return "Cube";
	}

}
