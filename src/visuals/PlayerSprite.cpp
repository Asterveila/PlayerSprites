#include "PlayerSprite.hpp"

namespace playersprites {

	using namespace cocos2d;

	namespace {

		std::string paddedFrameIndex(int index, int frameCount) {
			int width = 2;
			int maxIndexDigits = static_cast<int>(std::to_string(frameCount).size());
			if (maxIndexDigits > width) width = maxIndexDigits;

			std::string num = std::to_string(index);
			if (static_cast<int>(num.size()) < width) {
				num = std::string(width - num.size(), '0') + num;
			}
			return num;
		}

		void applyTextureMode(CCTexture2D* texture, bool pixelMode) {
			if (pixelMode) {
				texture->setAliasTexParameters();
			} else {
				texture->setAntiAliasTexParameters();
			}
		}

	}

	PlayerSprite* PlayerSprite::create() {
		auto ret = new PlayerSprite();
		if (ret && ret->init()) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}

	bool PlayerSprite::init() {
		if (!CCNode::init()) return false;

		m_sprite = CCSprite::create();
		if (!m_sprite) return false;
		m_sprite->setAnchorPoint({ 0.5f, 0.5f });
		this->addChild(m_sprite);

		m_basePosition = this->getPosition(); // just in case bruh u never fucking know

		this->setContentSize(m_sprite->getContentSize());
		this->setAnchorPoint({ 0.5f, 0.5f });

		return true;
	}

	std::vector<CCSpriteFrame*> PlayerSprite::buildFrames(SpritePack const& pack, AnimEvent const& anim) {
		std::vector<CCSpriteFrame*> frames;
		auto folder = pack.rootPath / anim.subfolder;

		if (anim.singleFrame) {
			auto path = folder / fmt::format("{}.png", anim.spriteName);
			if (!fs::exists(path)) {
				geode::log::warn("pack '{}' is missing single-frame file {}", pack.id, path.string());
				return frames;
			}
			auto* texture = CCTextureCache::sharedTextureCache()->addImage(path.string().c_str(), false);
			if (!texture) {
				geode::log::warn("pack '{}' failed to load texture {}", pack.id, path.string());
				return frames;
			}
			applyTextureMode(texture, pack.globalAttributes.pixelMode);
			auto rect = CCRect(0, 0, texture->getContentSize().width, texture->getContentSize().height);
			if (auto* frame = CCSpriteFrame::createWithTexture(texture, rect)) frames.push_back(frame);
			return frames;
		}

		frames.reserve(anim.frameCount);

		for (int i = 1; i <= anim.frameCount; ++i) {
			auto fileName = fmt::format("{}{}.png", anim.spriteBaseName, paddedFrameIndex(i, anim.frameCount));
			auto path = folder / fileName;

			if (!fs::exists(path)) {
				geode::log::warn("pack '{}' is missing frame file {}", pack.id, path.string());
				continue;
			}

			auto* texture = CCTextureCache::sharedTextureCache()->addImage(path.string().c_str(), false);
			if (!texture) {
				geode::log::warn("pack '{}' failed to load texture {}", pack.id, path.string());
				continue;
			}
			applyTextureMode(texture, pack.globalAttributes.pixelMode);

			auto rect = CCRect(0, 0, texture->getContentSize().width, texture->getContentSize().height);
			auto* frame = CCSpriteFrame::createWithTexture(texture, rect);
			if (frame) frames.push_back(frame);
		}

		return frames;
	}

	bool PlayerSprite::triggerAnim(SpritePack const& pack, AnimEvent const& anim, std::string const& gamemode, std::string const& eventName, bool force, bool isStateDriven) {
		if (isBlocking() && !force) {
			m_pendingPack = &pack;
			m_pendingGamemode = gamemode;
			m_pendingEventName = eventName;
			return false;
		}

		m_pendingPack = nullptr;

		auto frames = buildFrames(pack, anim);
		if (frames.empty()) {
			geode::log::warn("pack '{}' event {}/{} has no loadable frames, ignoring.", pack.id, gamemode, eventName);
			return false;
		}

		m_sprite->stopAllActions();
		this->stopAllActions();
		m_sprite->setOpacity(255);
		this->unschedule(schedule_selector(PlayerSprite::onHoldFinished));

		this->setPosition(m_basePosition);
		m_sprite->setVisible(true);
		m_sprite->setDisplayFrame(frames[0]);
		this->setContentSize(m_sprite->getContentSize());

		m_sprite->setScale(pack.globalAttributes.scale);

		auto size = this->getContentSize();
		m_currentOffsetY = anim.offsetY;
		m_sprite->setPosition({ size.width / 2.f + anim.offsetX, size.height / 2.f + anim.offsetY });

		m_currentCanBeInterrupted = anim.canBeInterrupted;
		m_currentInterruptBySelf = anim.interruptBySelf;
		m_currentLockRotation = anim.lockRotation;
		m_currentKeepPlayer = anim.keepPlayer;
		m_currentHoldFor = anim.holdFor;
		m_currentCancelOn = anim.cancelOn;
		m_currentFadeOutTime = anim.fadeOutTime;
		m_currentIsStateDriven = isStateDriven;
		m_currentGamemode = gamemode;
		m_currentEventName = eventName;

		if (!anim.lockRotation) {
			this->setRotation(0.f);
			m_sprite->setFlipY(false);
		}

		if (anim.singleFrame) {
			if (m_currentFadeOutTime.has_value()) {
				this->runAction(CCFadeOut::create(*m_currentFadeOutTime));
			}
			if (m_currentHoldFor.has_value() && *m_currentHoldFor < 0.f) {
				m_state = State::Holding;
			} else if (m_currentHoldFor.has_value()) {
				m_state = State::Holding;
				this->scheduleOnce(schedule_selector(PlayerSprite::onHoldFinished), *m_currentHoldFor);
			} else {
				m_state = State::Idle;
			}
			return true;
		}

		float frameTime = anim.rollFrameTime();

		auto* frameArray = CCArray::createWithCapacity(frames.size());
		for (auto* frame : frames) {
			frameArray->addObject(frame);
		}

		auto* animation = CCAnimation::createWithSpriteFrames(frameArray, frameTime);
		auto* animate = CCAnimate::create(animation);

		if (anim.loopAnim) {
			m_sprite->runAction(CCRepeatForever::create(animate));
			if (m_currentHoldFor.has_value() && *m_currentHoldFor >= 0.f) {
				this->scheduleOnce(schedule_selector(PlayerSprite::onHoldFinished), *m_currentHoldFor);
			}
		} else {
			auto* onFinished = CCCallFunc::create(this, callfunc_selector(PlayerSprite::onNonLoopFinished));
			m_sprite->runAction(CCSequence::create(animate, onFinished, nullptr));
		}

		m_state = State::Playing;

		return true;
	}

	void PlayerSprite::onNonLoopFinished() {
		if (m_currentFadeOutTime.has_value()) {
			this->runAction(CCFadeOut::create(*m_currentFadeOutTime));
		}
		if (m_currentHoldFor.has_value() && *m_currentHoldFor < 0.f) {
			m_state = State::Holding;
		} else if (m_currentHoldFor.has_value()) {
			m_state = State::Holding;
			this->scheduleOnce(schedule_selector(PlayerSprite::onHoldFinished), *m_currentHoldFor);
		} else if (!tryConsumePending()) {
			m_state = State::Idle;
		}
	}

	void PlayerSprite::onHoldFinished(float) {
		if (!tryConsumePending()) {
			m_state = State::Idle;
		}
	}

	bool PlayerSprite::tryConsumePending() {
		if (!m_pendingPack) return false;

		auto* pack = m_pendingPack;
		auto gamemode = m_pendingGamemode;
		auto eventName = m_pendingEventName;
		m_pendingPack = nullptr;

		auto* anim = pack->findEventById(gamemode, eventName);
		if (!anim) return false;

		return triggerAnim(*pack, *anim, gamemode, eventName, true);
	}

	bool PlayerSprite::currentCancelsOn(std::string const& eventName) const {
		for (auto const& name : m_currentCancelOn) {
			if (name == eventName) return true;
		}
		return false;
	}

	void PlayerSprite::stopAnim() {
		m_sprite->stopAllActions();
		this->stopAllActions();
		m_sprite->setOpacity(255);
		this->unschedule(schedule_selector(PlayerSprite::onHoldFinished));
		m_state = State::Idle;
		m_currentCanBeInterrupted = true;
		m_currentLockRotation = false;
		m_currentKeepPlayer = false;
		m_currentHoldFor.reset();
		m_currentCancelOn.clear();
		m_currentFadeOutTime.reset();
		m_currentOffsetY = 0.f;
		m_currentIsStateDriven = false;
		m_pendingPack = nullptr;
	}

	void PlayerSprite::resetState() {
		this->stopAnim();
		this->setPosition(m_basePosition);
		this->setRotation(0.f);
		m_sprite->setFlipY(false);
		m_sprite->setVisible(false);
	}

	void PlayerSprite::setFlipped(bool flipped) {
		m_sprite->setFlipY(flipped);

		auto size = this->getContentSize();
		float baseY = size.height / 2.f;
		float y = flipped ? (baseY - m_currentOffsetY) : (baseY + m_currentOffsetY);
		m_sprite->setPositionY(y);
	}

	void PlayerSprite::playRetroDeathAction() {
		auto* move = CCEaseBackIn::create(CCMoveBy::create(0.75f, { 0.f, -300.f }));
		this->runAction(move);

		auto* fadeAway = CCSequence::create(
			CCDelayTime::create(0.7f),
			CCFadeOut::create(0.05f),
			nullptr
		);
		this->runAction(fadeAway);
	}

}
