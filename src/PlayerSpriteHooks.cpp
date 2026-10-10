#include "core/PlayerSprite.hpp"
#include "GamemodeUtils.hpp"
#include "data/PackManager.hpp"
#include "data/PackSettings.hpp"
#include "core/SoundPlayer.hpp"

#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;
using namespace playersprites;

namespace {
	bool anyPackActive(bool platformer) {
		for (auto const& pack : PackManager::get().getLoadedPacks()) {
			if (settings::isPackActive(pack.id, platformer)) return true;
		}
		return false;
	}

	SpritePack const* activePackFor(bool platformer, std::string const& gamemode) {
		for (auto const& pack : PackManager::get().getLoadedPacks()) {
			if (!settings::isPackActive(pack.id, platformer)) continue;

			auto gmIt = pack.gamemodes.find(gamemode);
			if (gmIt == pack.gamemodes.end() || gmIt->second.events.empty()) continue;

			if (!settings::isGamemodeEnabled(pack.id, gamemode)) continue;

			return &pack;
		}
		return nullptr;
	}

	AnimEvent const* findEnabledEvent(SpritePack const& pack, std::string const& gamemode, std::string const& triggerName) {
		if (!settings::isGamemodeEnabled(pack.id, gamemode)) return nullptr;
		auto* anim = pack.findEvent(gamemode, triggerName);
		if (!anim) return nullptr;
		if (!settings::isEventEnabled(pack.id, gamemode, anim->id)) return nullptr;
		return anim;
	}
}

class $modify(PSPlayerObject, PlayerObject) {
	struct Fields {
		PlayerSprite* modSprite = nullptr;
	};

	bool init(int player, int ship, GJBaseGameLayer* gameLayer, cocos2d::CCLayer* layer, bool playLayer) {
		if (!PlayerObject::init(player, ship, gameLayer, layer, playLayer)) return false;

		if (auto* sprite = PlayerSprite::create()) {
			m_mainLayer->addChild(sprite, 100);
			m_fields->modSprite = sprite;
		}

		return true;
	}

	void update(float dt) {
		PlayerObject::update(dt);
		this->updateModSprite();
	}

	void updateModSprite() {
		auto flds = m_fields.self();
		auto* sprite = flds->modSprite;
		if (!sprite) return;

		auto gamemode = currentGamemodeName(this);
		auto* pack = activePackFor(m_isPlatformer, gamemode);

		if (!pack) {
			if (!anyPackActive(m_isPlatformer)) return;

			if (sprite->isPlaying()) sprite->stopAnim();
			sprite->setVisible(false);
			this->setVanillaVisible(true, GlobalAttributes{});
			return;
		}
		sprite->setVisible(true);

		if (sprite->isPlaying() && sprite->currentGamemode() != gamemode) {
			sprite->stopAnim();
		}

		bool moving = m_holdingLeft != m_holdingRight;
		// bool moving = std::fabs(m_platformerXVelocity) > 0.1f;
		std::string stateEventName = (m_isPlatformer && !moving) ? "Mod:Idle" : "Mod:Update";

		if (!sprite->isPlaying() || sprite->currentIsStateDriven()) {
			auto* anim = findEnabledEvent(*pack, gamemode, stateEventName);
			if (anim) {
				bool alreadyPlayingThis =
					sprite->isPlaying() &&
					sprite->currentGamemode() == gamemode &&
					sprite->currentEventName() == anim->id;

				if (!alreadyPlayingThis) {
					sprite->triggerAnim(*pack, *anim, gamemode, anim->id, false, true);
				}
			}
		}

		if (sprite->currentLockRotation()) {
			this->setRotation(0.f);
			sprite->setFlipped(m_isUpsideDown && !m_isRobot);
		} else {
			sprite->setFlipped(false);
		}

		bool showVanilla = !sprite->isPlaying() || sprite->currentKeepPlayer();
		this->setVanillaVisible(showVanilla, pack->globalAttributes);

		if (m_ghostTrail) {
			auto behavior = Mod::get()->getSettingValue<std::string>("ghost-trail-behavior");

			if (behavior == "replace") {
				m_ghostTrail->m_iconSprite = sprite->getSprite();
				m_ghostTrail->m_color = { 255, 255, 255 };

				if (sprite->isPlaying()) {
					auto* inner = sprite->getSprite();
					auto size = sprite->getContentSize();
					CCPoint offset = { inner->getPositionX() - size.width / 2.f, inner->getPositionY() - size.height / 2.f };

					if (m_isRobot && m_isUpsideDown) offset.y = -offset.y;
					if (m_isRobot) {
						float trailScale = inner->getScale();
						if (trailScale > 0.f) m_ghostTrail->m_position = offset / trailScale;
					} else {
						m_ghostTrail->m_position += offset;
					}
				}
			} else if (behavior == "disable") {
				m_ghostTrail->setVisible(false);
			}
		}
	}

	// TODO: this needs a LOOOOOOOOT of tweaking
	// ts still hiding and showing left and right when it shouldn't .
	void setVanillaVisible(bool visible, GlobalAttributes const& attrs) {
		bool complexIcon = m_isRobot || m_isSpider;

		if (complexIcon) {
			m_robotBatchNode->setVisible(visible);
			m_spiderBatchNode->setVisible(visible);

			if (m_isRobot && m_robotFire && !visible && !attrs.keepRobotFire) m_robotFire->setVisible(false);
		} else {
			m_iconSprite->setVisible(visible);
			m_iconSpriteSecondary->setVisible(visible);
			// m_iconSpriteWhitener->setVisible(visible);

			if (m_isShip || m_isBird) {
				m_vehicleSprite->setVisible(visible);
				m_vehicleSpriteSecondary->setVisible(visible);
				// m_vehicleSpriteWhitener->setVisible(visible);
				// m_vehicleGlow->setVisible(visible);
				if (m_isBird) m_birdVehicle->setVisible(visible);
			}

			if (m_isSwing && !attrs.keepSwingFires) {
				m_swingFireTop->setVisible(visible);
				m_swingFireMiddle->setVisible(visible);
				m_swingFireBottom->setVisible(visible);
			}
		}

		if (!attrs.keepDashFire) m_dashSpritesContainer->setVisible(visible);
	}

	PlayerSprite* getModSprite() {
		return m_fields->modSprite;
	}

	void playerDestroyed(bool noEffects) {
		auto gamemode = currentGamemodeName(this);
		auto* pack = activePackFor(m_isPlatformer, gamemode);

		if (pack && pack->globalAttributes.noDeathEffects) noEffects = true;

		PlayerObject::playerDestroyed(noEffects);

		audio::playForEvent(m_isPlatformer, gamemode, "Mod:Death");

		auto* sprite = m_fields->modSprite;
		if (!sprite || !pack) return;

		if (auto* anim = findEnabledEvent(*pack, gamemode, "Mod:RetroDeath")) {
			sprite->triggerAnim(*pack, *anim, gamemode, anim->id, true);
			sprite->playRetroDeathAction();
		} else if (auto* anim = findEnabledEvent(*pack, gamemode, "Mod:StaticDeath")) {
			sprite->triggerAnim(*pack, *anim, gamemode, anim->id, true);
		} else if (auto* anim = findEnabledEvent(*pack, gamemode, "Mod:StaticDeathDisappear")) {
			sprite->triggerAnim(*pack, *anim, gamemode, anim->id, true);
		}
	}

	void resetObject() {
		PlayerObject::resetObject();
		audio::resetSequences();
		if (auto* sprite = m_fields->modSprite) sprite->resetState();
	}
};

class $modify(PSBaseGameLayer, GJBaseGameLayer) {
	bool init() {
		if (!GJBaseGameLayer::init()) return false;

		audio::preloadAll();

		return true;
	}

	void gameEventTriggered(GJGameEvent event, int material, int playerID) {
		static bool ignoreNext = false;

		auto rawName = gameEventName(event);

		// geode::log::debug("-- EVENT CALLED RIGHT HERE: raw event={} material={} playerID={} ignoreNext={}", rawName, material, playerID, ignoreNext);

		if (ignoreNext) {
			ignoreNext = false;
			return GJBaseGameLayer::gameEventTriggered(event, material, playerID);
		}
		if (playerID != 0) {
			ignoreNext = true;
		}

		GJBaseGameLayer::gameEventTriggered(event, material, playerID);

		std::vector<PlayerObject*> targets;
		if (playerID == 1) targets.push_back(m_player1);
		else if (playerID == 2) targets.push_back(m_player2);
		else {
			if (m_player1) targets.push_back(m_player1);
			if (m_player2) targets.push_back(m_player2);
		}
		if (targets.empty()) return;

		// i used the GlobalSounds to destroy the GlobalSounds (again)
		std::unordered_set<std::string> soundGamemodes;

		for (auto* target : targets) {
			auto* psPlayer = static_cast<PSPlayerObject*>(target);
			auto gamemode = currentGamemodeName(target);

			if (soundGamemodes.insert(gamemode).second) {
				audio::playForEvent(m_isPlatformer, gamemode, rawName);
			}

			auto* sprite = psPlayer->getModSprite();
			if (!sprite) continue;

			auto* pack = activePackFor(m_isPlatformer, gamemode);
			if (!pack) continue;

			bool forceOverride = sprite->currentCancelsOn(rawName);

			auto* anim = findEnabledEvent(*pack, gamemode, rawName);
			// geode::log::debug("!! -- EVENT PASSED: event={} gamemode={} animFound={} forceOverride={}", rawName, gamemode, anim != nullptr, forceOverride);

			if (anim) {
				bool alreadyPlayingThis = sprite->isPlaying() && sprite->currentGamemode() == gamemode && sprite->currentEventName() == anim->id;

				if (!alreadyPlayingThis) {
					sprite->triggerAnim(*pack, *anim, gamemode, anim->id, forceOverride);
				} else if (sprite->currentInterruptBySelf()) {
					sprite->triggerAnim(*pack, *anim, gamemode, anim->id, true);
				}
			} else if (forceOverride) {
				sprite->stopAnim();
			}
		}
	}
};
