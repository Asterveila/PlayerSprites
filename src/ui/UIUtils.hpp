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

	inline CCMenu* columnMenu(float gap = 4.f, AxisAlignment axisAlign = AxisAlignment::Center, AxisAlignment crossAlign = AxisAlignment::Center, bool growCross = false, const char* id = nullptr, float anchorX = 0.5f, float anchorY = 0.5f) {
		auto menu = CCMenu::create();
		menu->setContentSize({ 0.f, 0.f });
		menu->setAnchorPoint({ anchorX, anchorY });
		menu->setLayout(
			ColumnLayout::create()
				->setGap(gap)
				->setAxisAlignment(axisAlign)
				->setCrossAxisAlignment(crossAlign)
				->setAutoScale(false)
				->setAutoGrowAxis(true)
				->setGrowCrossAxis(growCross)
				->setAxisReverse(true)
		);
		if (id) menu->setID(id);
		return menu;
	}

	inline CCMenu* rowMenu(float gap = 4.f, AxisAlignment axisAlign = AxisAlignment::Start, AxisAlignment crossAlign = AxisAlignment::Center, bool growCross = false, const char* id = nullptr, float anchorX = 0.5f, float anchorY = 0.5f) {
		auto menu = CCMenu::create();
		menu->setContentSize({ 0.f, 0.f });
		menu->setAnchorPoint({ anchorX, anchorY });
		menu->setLayout(
			RowLayout::create()
				->setGap(gap)
				->setAxisAlignment(axisAlign)
				->setCrossAxisAlignment(crossAlign)
				->setAutoScale(false)
				->setAutoGrowAxis(true)
				->setGrowCrossAxis(growCross)
		);
		if (id) menu->setID(id);
		return menu;
	}

	inline CCNode* column(float gap = 4.f, AxisAlignment axisAlign = AxisAlignment::Center, AxisAlignment crossAlign = AxisAlignment::Center, bool growCross = false, const char* id = nullptr) {
		auto node = CCNode::create();
		node->setContentSize({ 0.f, 0.f });
		node->setAnchorPoint({ 0.5f, 0.5f });
		node->setLayout(
			ColumnLayout::create()
				->setGap(gap)
				->setAxisAlignment(axisAlign)
				->setCrossAxisAlignment(crossAlign)
				->setAutoScale(false)
				->setAutoGrowAxis(true)
				->setGrowCrossAxis(growCross)
				->setAxisReverse(true)
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

	inline VisualToggler visToggler(bool initialState, CCObject* target, SEL_MenuHandler selector, float togglerScale = 0.7f, const char* id = nullptr, CCSprite* offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"), CCSprite* onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png")) {
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

		auto toggler = CCMenuItemToggler::create(offSpr, onSpr, target, selector);
		toggler->setScale(togglerScale);
		menu->addChild(toggler);
		menu->updateLayout();

		toggler->toggle(initialState);

		if (id) menu->setID(id);

		return { menu, toggler };
	}

	inline CCMenuItemToggler* rawVisToggler(bool initialState, CCObject* target, SEL_MenuHandler selector, float togglerScale = 0.7f, const char* id = nullptr, CCSprite* offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"), CCSprite* onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png")) {
		auto toggler = CCMenuItemToggler::create(offSpr, onSpr, target, selector);
		toggler->setScale(togglerScale);
		toggler->toggle(initialState);

		if (id) toggler->setID(id);

		return toggler;
	}

}
