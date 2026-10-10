#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace playersprites::UIUtils {

	inline CCNode* row(float gap = 4.f, AxisAlignment axisAlign = AxisAlignment::Start, AxisAlignment crossAlign = AxisAlignment::Center, bool growCross = false, const char* id = nullptr) {
		auto node = CCNode::create();
		node->setContentSize({ 0.f, 0.f });
		node->setAnchorPoint({ 0.5f, 0.5f });
		node->setLayout(
			RowLayout::create()
				->setGap(gap)
				->setAxisAlignment(axisAlign)
				->setCrossAxisAlignment(crossAlign)
				->setAutoScale(false)
				->setAutoGrowAxis(true)
				->setGrowCrossAxis(growCross)
		);
		if (id) node->setID(id);
		return node;
	}

	struct TogglerRow {
		CCMenu* container;
		CCMenuItemToggler* toggler;
	};

	inline TogglerRow togglerRow(const std::string& labelText, bool initialState, CCObject* target, SEL_MenuHandler selector, float width = 140.f, float labelScale = 0.4f, float togglerScale = 0.7f, const char* id = nullptr) {
		auto menu = CCMenu::create();
		// menu->setContentSize({ width, 20.f });
		menu->setLayout(
			RowLayout::create()
				->setAxisAlignment(AxisAlignment::Start)
				->setCrossAxisAlignment(AxisAlignment::Center)
				->setAutoScale(false)
				->setGrowCrossAxis(true)
				->setAxisReverse(true)
				->setAutoGrowAxis(true)
		);

		auto label = CCLabelBMFont::create(labelText.c_str(), "bigFont.fnt");
		label->setScale(labelScale);
		label->setAnchorPoint({ 0.f, 0.5f });
		menu->addChild(label);

		auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
		auto onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
		auto toggler = CCMenuItemToggler::create(offSpr, onSpr, target, selector);
		toggler->setScale(togglerScale);
		menu->addChild(toggler);
		menu->updateLayout();

		toggler->toggle(initialState);

		if (id) menu->setID(id);

		return { menu, toggler };
	}

	struct VisualToggler {
		CCMenu* container;
		CCMenuItemToggler* toggler;
	};

	inline VisualToggler visToggler(bool initialState, CCObject* target, SEL_MenuHandler selector, float togglerScale = 0.7f, const char* id = nullptr, const char* offSpr = "GJ_checkOff_001.png", const char* onSpr = "GJ_checkOn_001.png") {
		auto menu = CCMenu::create();
		menu->setLayout(
			RowLayout::create()
				->setAxisAlignment(AxisAlignment::Start)
				->setCrossAxisAlignment(AxisAlignment::Center)
				->setAutoScale(false)
				->setGrowCrossAxis(true)
				->setAxisReverse(true)
				->setAutoGrowAxis(true)
		);

		auto offSprite = CCSprite::createWithSpriteFrameName(offSpr);
		auto onSprite = CCSprite::createWithSpriteFrameName(onSpr);

		auto toggler = CCMenuItemToggler::create(offSprite, onSprite, target, selector);
		toggler->setScale(togglerScale);
		menu->addChild(toggler);
		menu->updateLayout();

		toggler->toggle(initialState);

		if (id) menu->setID(id);

		return { menu, toggler };
	}

}
