#include "SpritePackCell.hpp"

namespace playersprites {

	namespace {

		std::string buildCreditsLine(SpritePackMeta const& meta) {
			if (meta.credits.empty()) {
				return fmt::format("By {}", meta.author);
			}
			std::string joined;
			for (size_t i = 0; i < meta.credits.size(); ++i) {
				if (i > 0) joined += ", ";
				joined += meta.credits[i];
			}
			return fmt::format("By {} - Assets by {}", meta.author, joined);
		}

		CCSprite* loadPackIcon(fs::path const& rootPath) {
			// @geode-ignore(unknown-resource)
			auto path = rootPath / "pack.png";
			if (!fs::exists(path)) return CCSprite::create();

			auto* texture = CCTextureCache::sharedTextureCache()->addImage(path.string().c_str(), false);
			if (!texture) return CCSprite::create();

			return CCSprite::createWithTexture(texture);
		}

	}

	SpritePackCell* SpritePackCell::createForPack(SpritePack const& pack, bool even, CCObject* target, SEL_MenuHandler settingsSelector) {
		auto ret = new SpritePackCell();
		if (ret->initForPack(pack, even, target, settingsSelector)) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	SpritePackCell* SpritePackCell::createForError(std::string const& id, std::string const& error, bool even) {
		auto ret = new SpritePackCell();
		if (ret->initForError(id, error, even)) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	bool SpritePackCell::initForPack(SpritePack const& pack, bool even, CCObject* target, SEL_MenuHandler settingsSelector) {
		if (!CCLayer::init()) return false;

		CCSize size = { WIDTH, HEIGHT };
		this->setContentSize(size);
		this->setID(fmt::format("{}-cell", pack.id));

		m_background = CCLayerColor::create();
		m_background->setContentSize(size);
		m_background->setOpacity(even ? 60 : 30);
		m_background->setColor({ 0, 0, 0 });
		this->addChild(m_background, -1);

		auto* icon = loadPackIcon(pack.rootPath);
		icon->setAnchorPoint({ 0.5f, 0.5f });
		
		auto iconSize = icon->getContentSize();
		float maxDim = std::max(iconSize.width, iconSize.height);
		if (maxDim > 0.f) icon->setScale((HEIGHT - 10.f) / maxDim);
		icon->setPosition({ (HEIGHT / 2.f), size.height / 2.f });
		this->addChild(icon);

		auto nameLabel = CCLabelBMFont::create(pack.meta.setName.c_str(), "bigFont.fnt");
		nameLabel->setAnchorPoint({ 0.f, 0.5f });
		nameLabel->setPosition({ HEIGHT + 8.f, size.height * 0.62f });
		nameLabel->setScale(0.5f);
		nameLabel->limitLabelWidth(WIDTH - HEIGHT - 60.f, 0.5f, 0.1f);
		this->addChild(nameLabel);

		auto creditsLine = buildCreditsLine(pack.meta);
		auto creditsLabel = CCLabelBMFont::create(creditsLine.c_str(), "chatFont.fnt");
		creditsLabel->setAnchorPoint({ 0.f, 0.5f });
		creditsLabel->setPosition({ HEIGHT + 8.f, size.height * 0.3f });
		creditsLabel->setScale(0.4f);
		creditsLabel->setOpacity(180);
		creditsLabel->limitLabelWidth(WIDTH - HEIGHT - 60.f, 0.4f, 0.1f);
		this->addChild(creditsLabel);

		auto menu = CCMenu::create();
		menu->setContentSize(size);
		menu->setPosition({ 0.f, 0.f });
		menu->setAnchorPoint({ 0.f, 0.f });
		this->addChild(menu);

		auto gearSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
		gearSprite->setScale(0.7f);
		m_settingsButton = CCMenuItemSpriteExtra::create(gearSprite, target, settingsSelector);
		m_settingsButton->setUserObject("pack-id"_spr, CCString::create(pack.id));
		m_settingsButton->setPosition({ size.width - 24.f, size.height / 2.f });
		m_settingsButton->setID(fmt::format("{}-settings-btn", pack.id));
		menu->addChild(m_settingsButton);

		return true;
	}

	bool SpritePackCell::initForError(std::string const& id, std::string const& error, bool even) {
		if (!CCLayer::init()) return false;

		CCSize size = { WIDTH, HEIGHT };
		this->setContentSize(size);
		this->setID(fmt::format("{}-error-cell", id));

		m_background = CCLayerColor::create();
		m_background->setContentSize(size);
		m_background->setOpacity(even ? 60 : 30);
		m_background->setColor({ 60, 0, 0 });
		this->addChild(m_background, -1);

		auto idLabel = CCLabelBMFont::create(id.c_str(), "bigFont.fnt");
		idLabel->setAnchorPoint({ 0.f, 0.5f });
		idLabel->setPosition({ 12.f, size.height * 0.62f });
		idLabel->setScale(0.5f);
		idLabel->limitLabelWidth(WIDTH - 24.f, 0.5f, 0.1f);
		this->addChild(idLabel);

		auto errorLabel = CCLabelBMFont::create(error.c_str(), "chatFont.fnt");
		errorLabel->setAnchorPoint({ 0.f, 0.5f });
		errorLabel->setPosition({ 12.f, size.height * 0.3f });
		errorLabel->setScale(0.4f);
		errorLabel->setColor({ 255, 90, 90 });
		errorLabel->limitLabelWidth(WIDTH - 24.f, 0.4f, 0.1f);
		this->addChild(errorLabel);

		return true;
	}

}
