#pragma once

#include "../data/SpritePackTypes.hpp"

using namespace geode::prelude;

namespace playersprites {

	class PlayerSprite : public CCNode {
	protected:
		enum class State {
			Idle,
			Playing,
			Holding
		};

		CCSprite* m_sprite = nullptr;

		State m_state = State::Idle;
		bool m_currentCanBeInterrupted = true;
		bool m_currentInterruptBySelf = false;
		bool m_currentLockRotation = false;
		bool m_currentKeepPlayer = false;
		std::optional<float> m_currentHoldFor;
		std::vector<std::string> m_currentCancelOn;
		std::optional<float> m_currentFadeOutTime;
		float m_currentOffsetY = 0.f;
		float m_currentOffsetX = 0.f;
		bool m_flipped = false;
		CCPoint m_basePosition = { 0.f, 0.f }; // the position of the PlayerSprite, NOT m_sprite, dumbass

		bool m_currentIsStateDriven = false;

		std::string m_currentGamemode;
		std::string m_currentEventName;

		SpritePack const* m_pendingPack = nullptr;
		std::string m_pendingGamemode;
		std::string m_pendingEventName;

		bool init();

		void onNonLoopFinished();
		void onFirstRunFinished(CCObject* loopAnimation);
		void onHoldFinished(float dt);

		bool tryConsumePending();

	public:
		static PlayerSprite* create();

		static std::vector<CCSpriteFrame*> buildFrames(SpritePack const& pack, AnimEvent const& anim, std::vector<int>* frameNumbers = nullptr);

		bool triggerAnim(SpritePack const& pack, AnimEvent const& anim, std::string const& gamemode, std::string const& eventName, bool force = false, bool isStateDriven = false);

		void stopAnim();
		void resetState();

		bool isPlaying() const { return m_state != State::Idle; }
		bool isBlocking() const { return m_state != State::Idle && !m_currentCanBeInterrupted; }

		bool currentCancelsOn(std::string const& eventName) const;

		std::string const& currentGamemode() const { return m_currentGamemode; }
		std::string const& currentEventName() const { return m_currentEventName; }

		bool currentKeepPlayer() const { return m_currentKeepPlayer; }
		bool currentLockRotation() const { return m_currentLockRotation; }
		bool currentIsStateDriven() const { return m_currentIsStateDriven; }
		bool currentInterruptBySelf() const { return m_currentInterruptBySelf; }

		CCSprite* getSprite() const { return m_sprite; }
		CCPoint getCurrentOffset() const;

		void setFlipped(bool flipped);

		void playRetroDeathAction();
	};

}
