#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/file.hpp>

using namespace geode::prelude;

namespace playersprites {

	class SpritePackListPopup : public Popup {
	protected:
		ScrollLayer* m_packScrollLayer = nullptr;

		bool init() override;
		void buildList();

		geode::async::TaskHolder<geode::Result<std::optional<std::filesystem::path>>> m_importListener;

		void onReload(CCObject* sender);
		void onImport(CCObject* sender);
		void onTogglePack(CCObject* sender);
		void onPackInfo(CCObject* sender);
		void onPackSettings(CCObject* sender);
		void onOpenFolder(CCObject* sender);
		void onModSettings(CCObject* sender);

	public:
		static SpritePackListPopup* create();
	};

}
