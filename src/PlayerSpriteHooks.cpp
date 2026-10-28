#include "core/PlayerSprite.hpp"
#include "GamemodeUtils.hpp"
#include "data/PackManager.hpp"
#include "data/PackSettings.hpp"
#include "core/SoundPlayer.hpp"

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/GhostTrailEffect.hpp>

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
			if (gmIt == pack.gamemodes.end() || (gmIt->second.events.empty() && gmIt->second.taunts.empty())) continue;

			if (!settings::isGamemodeEnabled(pack.id, gamemode)) continue;

			return &pack;
		}
		return nullptr;
	}

	AnimEvent const* findEnabledEvent(SpritePack const& pack, std::string const& gamemode, std::string const& triggerName) {
		if (!settings::isGamemodeEnabled(pack.id, gamemode)) return nullptr;
		for (auto const* anim : pack.findEvents(gamemode, triggerName)) {
			if (settings::isEventEnabled(pack.id, gamemode, anim->id)) return anim;
		}
		return nullptr;
	}
}

class $modify(PSPlayerObject, PlayerObject) {
	struct Fields {
		PlayerSprite* modSprite = nullptr;
		
		geode::Ref<CCSprite> ghostGuide;
		geode::Ref<CCSprite> ghostOriginal;
		float ghostRotation = 0.f;

		bool levelComplete = false;
		bool hasCompleteAnim = false;

		// taunt stuff
		std::unordered_map<std::string, size_t> tauntNext;
		std::unordered_map<std::string, size_t> tauntLast;

		struct TauntTimer {
			std::chrono::steady_clock::time_point at;
			float animDuration = 0.f;
			float resetAfter = 0.f;
		};
		std::unordered_map<std::string, TauntTimer> tauntTimer;
		bool tauntInterruptByTaunts = true;
		bool tauntAllowInMotion = false;
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

	float getghostRotation() {
		return m_fields->ghostRotation;
	}

	CCSprite* getGhostGuide() {
		return m_fields->ghostGuide.data();
	}

	void restoreGhostTrail() {
		auto flds = m_fields.self();
		flds->ghostRotation = 0.f;
		if (!m_ghostTrail || !flds->ghostGuide) return;
		if (m_ghostTrail->m_iconSprite != flds->ghostGuide.data()) return;

		m_ghostTrail->m_iconSprite = flds->ghostOriginal ? flds->ghostOriginal.data() : m_iconSprite;
	}

	void updateModSprite() {
		auto flds = m_fields.self();
		auto* sprite = flds->modSprite;
		auto gm = GameManager::sharedState();
		if (!sprite) return;

		if (m_ghostTrail && m_ghostTrail->m_iconSprite && m_ghostTrail->m_iconSprite != flds->ghostGuide.data()) {
			flds->ghostOriginal = m_ghostTrail->m_iconSprite;
		}

		if (flds->levelComplete) {
			sprite->setVisible(flds->hasCompleteAnim);
			return;
		}

		auto gamemode = currentGamemodeName(this);
		auto* pack = activePackFor(m_isPlatformer, gamemode);

		if (!pack) {
			this->restoreGhostTrail();

			if (!anyPackActive(m_isPlatformer)) return;

			if (sprite->isPlaying()) sprite->stopAnim();
			sprite->setVisible(false);
			this->setVanillaVisible(true, GlobalAttributes{});
			return;
		}
		sprite->setVisible(true);
		sprite->setPlayerColours();

		if (sprite->isPlaying() && sprite->currentGamemode() != gamemode) {
			sprite->stopAnim();
		}

		bool standingStill = isStandingStill();
		std::string stateEventName = standingStill ? "Mod:Idle" : "Mod:Update";

		if (!standingStill && !flds->tauntAllowInMotion && sprite->isPlaying() && sprite->currentEventName() == "Mod:Taunt") {
			sprite->stopAnim();
		}

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

		if (!sprite->isPlaying() && !m_isDead) {
			sprite->getSprite()->setVisible(false);
		}

		if (sprite->currentLockRotation()) {
			this->setRotation(0.f);

			// im just gonna assume the spider behaves  the same. for now.
			bool flipped = !(m_isRobot || m_isSpider) && (m_isUpsideDown != m_isSideways);
			sprite->setFlipped(flipped);
		} else {
			sprite->setFlipped(false);
		}

		bool showVanilla = !sprite->isPlaying() || sprite->currentKeepPlayer();
		this->setVanillaVisible(showVanilla, pack->globalAttributes);

		if (m_ghostTrail) {
			auto behavior = Mod::get()->getSettingValue<std::string>("ghost-trail-behavior");

			if (behavior == "replace" && !sprite->isPlaying()) {
				this->restoreGhostTrail();
			} else if (behavior == "replace") {
				auto* pSprite = sprite->getSprite();

				if (!flds->ghostGuide) flds->ghostGuide = CCSprite::create();
				auto* guide = flds->ghostGuide.data();

				bool facingLeft = this->getScaleX() < 0.f;
				bool flipX = facingLeft != (m_mainLayer->getScaleX() < 0.f);
				bool flipY = pSprite->isFlipY() != (m_mainLayer->getScaleY() < 0.f);
				float spriteScale = std::abs(pSprite->getScaleX());

				if (auto* frame = pSprite->displayFrame()) {
					if (sprite->isPlaying() && spriteScale > 0.f) {
						auto size = sprite->getContentSize();
						float rawX = pSprite->getPositionX() - size.width / 2.f;
						float rawY = pSprite->getPositionY() - size.height / 2.f;
						if (pSprite->isFlipY()) rawY = -rawY;
						frame->setOffset({ rawX / spriteScale, rawY / spriteScale });
						guide->setDisplayFrame(frame);
					}
				}

				guide->setScaleX(spriteScale * (flipX ? -1.f : 1.f));
				guide->setScaleY(std::abs(pSprite->getScaleY()) * (flipY ? -1.f : 1.f));

				m_ghostTrail->m_iconSprite = guide;
				m_ghostTrail->m_color = pSprite->getColor();
				m_ghostTrail->m_position = CCPoint(0, 0);

				float mainRotation = m_mainLayer->getRotation();
				flds->ghostRotation = facingLeft ? -mainRotation : mainRotation;
			} else if (behavior == "disable") {
				m_ghostTrail->setVisible(false);
			}
		}
	}

	void setVanillaVisible(bool visible, GlobalAttributes const& attrs) {
		bool complexIcon = m_isRobot || m_isSpider;

		if (complexIcon) {
			m_robotBatchNode->setVisible(visible);
			m_spiderBatchNode->setVisible(visible);

			if (m_isRobot && m_robotFire && !visible && !attrs.keepRobotFire) m_robotFire->setVisible(false);
			if (m_isRobot && m_robotBurstParticles) m_robotBurstParticles->setVisible(visible || attrs.keepRobotParticles);
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

	bool isStandingStill() { return m_isPlatformer && m_holdingLeft == m_holdingRight; }

	bool runTaunt(std::unordered_set<std::string>& playedSounds) {
		auto* sprite = m_fields->modSprite;
		if (!sprite || m_isDead || m_fields->levelComplete) return false;

		if (sprite->isPlaying() && sprite->currentEventName() == "Mod:Taunt") {
			if (!m_fields->tauntInterruptByTaunts && !sprite->isHolding()) return false;
		} else if (sprite->isBlocking()) {
			return false;
		}

		auto gamemode = currentGamemodeName(this);
		auto* pack = activePackFor(m_isPlatformer, gamemode);
		if (!pack) return false;

		auto const& gm = pack->gamemodes.at(gamemode);
		auto const& taunts = gm.taunts;
		if (taunts.empty()) return false;

		auto const key = fmt::format("{}/{}", pack->id, gamemode);

		size_t index = 0;
		size_t nextAfter = 0;
		if (gm.tauntsOrdered) {
			size_t next = 0;
			if (auto it = m_fields->tauntNext.find(key); it != m_fields->tauntNext.end()) next = it->second;

			auto timerIt = m_fields->tauntTimer.find(key);
			if (timerIt != m_fields->tauntTimer.end() && timerIt->second.resetAfter > 0.f) {
				float secondsSince = std::chrono::duration<float>(std::chrono::steady_clock::now() - timerIt->second.at).count();
				if (secondsSince > timerIt->second.animDuration + timerIt->second.resetAfter) next = 0;
			}

			if (next >= taunts.size()) next = 0;
			index = next;
			nextAfter = (next + 1) % taunts.size();
		} else if (taunts.size() > 1) {
			static std::mt19937 gen{ std::random_device{}() };
			auto lastIt = m_fields->tauntLast.find(key);
			if (lastIt == m_fields->tauntLast.end()) {
				index = std::uniform_int_distribution<size_t>(0, taunts.size() - 1)(gen);
			} else {
				index = std::uniform_int_distribution<size_t>(0, taunts.size() - 2)(gen);
				if (index >= lastIt->second) index++;
			}
		}

		auto const& taunt = taunts[index];

		if (!taunt.allowInMotion && !isStandingStill()) return false;

		if (!sprite->triggerAnim(*pack, taunt.anim, gamemode, "Mod:Taunt", true)) return false;

		if (gm.tauntsOrdered) m_fields->tauntNext[key] = nextAfter;
		else m_fields->tauntLast[key] = index;
		m_fields->tauntInterruptByTaunts = taunt.interruptByTaunts;
		m_fields->tauntAllowInMotion = taunt.allowInMotion;
		m_fields->tauntTimer[key] = { std::chrono::steady_clock::now(), sprite->getCurrentAnimDuration(), taunt.resetAfter };

		if (!taunt.soundId.empty() && playedSounds.insert(fmt::format("{}/{}", key, taunt.soundId)).second) {
			audio::playEntry(*pack, m_isPlatformer, gamemode, taunt.soundId);
		}
		return true;
	}

	void resetObject() {
		PlayerObject::resetObject();

		audio::resetSequences();
		m_fields->levelComplete = false;
		m_fields->hasCompleteAnim = false;

		m_fields->tauntNext.clear();
		m_fields->tauntLast.clear();
		m_fields->tauntTimer.clear();

		if (auto* sprite = m_fields->modSprite) sprite->resetState();
	}

	void playCompleteEffect(bool noEffects, bool instant) {
		PlayerObject::playCompleteEffect(noEffects, instant);

		auto* sprite = m_fields->modSprite;
		if (!sprite) return;

		auto gamemode = currentGamemodeName(this);
		auto* pack = activePackFor(m_isPlatformer, gamemode);
		if (!pack) return;

		m_fields->levelComplete = true;
		audio::playForEvent(m_isPlatformer, gamemode, "Mod:LevelComplete");

		if (auto* anim = findEnabledEvent(*pack, gamemode, "Mod:LevelComplete")) {
			m_fields->hasCompleteAnim = true;
			sprite->triggerAnim(*pack, *anim, gamemode, anim->id, true);
			sprite->setVisible(true);
			this->setVanillaVisible(false, pack->globalAttributes);
		} else {
			m_fields->hasCompleteAnim = false;
			sprite->stopAnim();
			sprite->setVisible(false);
		}
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
			if (m_player2 && m_gameState.m_isDualMode) targets.push_back(m_player2);
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

class $modify(PSGhostTrailEffect, GhostTrailEffect) {
	void trailSnapshot(float dt) {
		auto* player = m_playerObject ? static_cast<PSPlayerObject*>(m_playerObject) : nullptr;
		auto* sprite = player ? player->getModSprite() : nullptr;
		float extraRotation = player ? player->getghostRotation() : 0.f;

		if (!sprite || !sprite->isPlaying() || extraRotation == 0.f || m_iconSprite != player->getGhostGuide()) {
			return GhostTrailEffect::trailSnapshot(dt);
		}

		float ogRot = player->getRotation();

		player->setRotation(ogRot + extraRotation);
		GhostTrailEffect::trailSnapshot(dt);
		player->setRotation(ogRot);
	}
};

class $modify(PSPlayLayer, PlayLayer) {
	void setupHasCompleted() {
		PlayLayer::setupHasCompleted();

		this->addEventListener(
			KeybindSettingPressedEventV3(Mod::get(), "taunt-keybind"),
			[this](Keybind const&, bool down, bool repeat, double) {
				if (!down || repeat || m_isPaused) return false;

				std::vector<PlayerObject*> players = { m_player1 };
				if (m_gameState.m_isDualMode) players.push_back(m_player2);

				bool tauntStarted = false;
				std::unordered_set<std::string> playedSounds;
				for (auto* player : players) {
					if (!player) continue;
					if (static_cast<PSPlayerObject*>(player)->runTaunt(playedSounds)) tauntStarted = true;
				}
				return tauntStarted;
			}
		);
	}
};