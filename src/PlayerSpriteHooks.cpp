#include "visuals/PlayerSprite.hpp"
#include "GamemodeUtils.hpp"
#include "data/PackManager.hpp"

#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;
using namespace playersprites;

namespace {
	SpritePack const* activePack() {
		auto& loaded = PackManager::get().getLoadedPacks();
		if (loaded.empty()) return nullptr;
		return &loaded[0];
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
		auto* sprite = m_fields->modSprite;
		if (!sprite) return;

		auto* pack = activePack();
		if (!pack) return;

		auto gamemode = currentGamemodeName(this);

		bool gamemodeDeclared = pack->gamemodes.find(gamemode) != pack->gamemodes.end();
		if (!gamemodeDeclared) {
			if (sprite->isPlaying()) sprite->stopAnim();
			sprite->setVisible(false);
			this->setVanillaVisible(true);
			return;
		}
		sprite->setVisible(true);

		std::string stateEventName = (m_isPlatformer && !m_isMoving) ? "Mod:Idle" : "Mod:Update";

		if (!sprite->isPlaying() || sprite->currentIsStateDriven()) {
			auto* anim = pack->findEvent(gamemode, stateEventName);
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
			sprite->setFlipped(m_isUpsideDown);
		} else {
			sprite->setFlipped(false);
		}

		bool showVanilla = !sprite->isPlaying() || sprite->currentKeepPlayer();
		this->setVanillaVisible(showVanilla);
	}

	// TODO: this needs a LOOOOOOOOT of tweaking
	// ts still hiding and showing left and right when it shouldn't .
	void setVanillaVisible(bool visible) {
		bool complexIcon = m_isRobot || m_isSpider;

		if (complexIcon) {
			m_robotBatchNode->setVisible(visible);
			m_spiderBatchNode->setVisible(visible);
			return;
		}

		m_iconSprite->setVisible(visible);
		m_iconSpriteSecondary->setVisible(visible);
		// m_iconSpriteWhitener->setVisible(visible);

		if (m_isShip || m_isBird) {
			m_vehicleSprite->setVisible(visible);
			m_vehicleSpriteSecondary->setVisible(visible);
			// m_vehicleSpriteWhitener->setVisible(visible);
			// m_vehicleGlow->setVisible(visible);
			m_birdVehicle->setVisible(visible);
		}

		if (m_isSwing) {
			m_swingFireTop->setVisible(visible);
			m_swingFireMiddle->setVisible(visible);
			m_swingFireBottom->setVisible(visible);
		}
		m_dashSpritesContainer->setVisible(visible);
	}

	PlayerSprite* getModSprite() {
		return m_fields->modSprite;
	}

	void playerDestroyed(bool noEffects) {
		PlayerObject::playerDestroyed(noEffects);

		auto* sprite = m_fields->modSprite;
		if (!sprite) return;

		auto* pack = activePack();
		if (!pack) return;

		auto gamemode = currentGamemodeName(this);

		if (auto* anim = pack->findEvent(gamemode, "Mod:RetroDeath")) {
			sprite->triggerAnim(*pack, *anim, gamemode, anim->id, true);
			sprite->playRetroDeathAction();
		} else if (auto* anim = pack->findEvent(gamemode, "Mod:StaticDeath")) {
			sprite->triggerAnim(*pack, *anim, gamemode, anim->id, true);
		} else if (auto* anim = pack->findEvent(gamemode, "Mod:StaticDeathDisappear")) {
			sprite->triggerAnim(*pack, *anim, gamemode, anim->id, true);
		}
	}
};

class $modify(PSBaseGameLayer, GJBaseGameLayer) {
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

		auto* pack = activePack();
		if (!pack) return;

		for (auto* target : targets) {
			auto* psPlayer = static_cast<PSPlayerObject*>(target);
			auto* sprite = psPlayer->getModSprite();
			if (!sprite) continue;

			auto gamemode = currentGamemodeName(target);

			bool forceOverride = sprite->currentCancelsOn(rawName);

			auto* anim = pack->findEvent(gamemode, rawName);
			// geode::log::debug("!!!!-- EVENT PASSED HERE: gamemode={}, anim found={}, forceOverride={}", rawName, gamemode, anim != nullptr, forceOverride);

			if (anim) {
				bool alreadyPlayingThis =
					sprite->isPlaying() &&
					sprite->currentGamemode() == gamemode &&
					sprite->currentEventName() == anim->id;

				if (!alreadyPlayingThis) {
					sprite->triggerAnim(*pack, *anim, gamemode, anim->id, forceOverride);
				}
			} else if (forceOverride) {
				sprite->stopAnim();
			}
		}
	}
};
