#pragma once

#include "../data/SpritePackTypes.hpp"

namespace playersprites {

	class PlayerSprite : public cocos2d::CCNode {
	protected:
		enum class State {
			Idle,
			Playing,
			Holding
		};

		cocos2d::CCSprite* m_sprite = nullptr;

		State m_state = State::Idle;
		bool m_currentCanBeInterrupted = true;
		bool m_currentLockRotation = false;
		bool m_currentKeepPlayer = false;
		std::optional<float> m_currentHoldFor;
		std::vector<std::string> m_currentCancelOn;
		std::optional<float> m_currentFadeOutTime;
		float m_currentOffsetY = 0.f;

		bool m_currentIsStateDriven = false;

		std::string m_currentGamemode;
		std::string m_currentEventName;

		SpritePack const* m_pendingPack = nullptr;
		std::string m_pendingGamemode;
		std::string m_pendingEventName;

		bool init();

		std::vector<cocos2d::CCSpriteFrame*> buildFrames(SpritePack const& pack, AnimEvent const& anim);

		void onNonLoopFinished();
		void onHoldFinished(float dt);

		bool tryConsumePending();

	public:
		static PlayerSprite* create();

		bool triggerAnim(SpritePack const& pack, AnimEvent const& anim, std::string const& gamemode, std::string const& eventName, bool force = false, bool isStateDriven = false);

		void stopAnim();

		bool isPlaying() const { return m_state != State::Idle; }
		bool isBlocking() const { return m_state != State::Idle && !m_currentCanBeInterrupted; }

		bool currentCancelsOn(std::string const& eventName) const;

		std::string const& currentGamemode() const { return m_currentGamemode; }
		std::string const& currentEventName() const { return m_currentEventName; }

		bool currentKeepPlayer() const { return m_currentKeepPlayer; }
		bool currentLockRotation() const { return m_currentLockRotation; }
		bool currentIsStateDriven() const { return m_currentIsStateDriven; }

		void setFlipped(bool flipped);

		void playRetroDeathAction();
	};

}
