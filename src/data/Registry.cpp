#include "Registry.hpp"
#include <matjson.hpp>

namespace playersprites {

	using geode::Ok;
	using geode::Err;
	using geode::Result;

	fs::path Registry::packsDir() {
		return geode::Mod::get()->getConfigDir() / "packs";
	}

	Registry Registry::loadOrCreate() {
		Registry reg;
		reg.m_registryPath = geode::Mod::get()->getConfigDir() / "registry.json";

		std::error_code ec;
		fs::create_directories(packsDir(), ec);

		if (!fs::exists(reg.m_registryPath)) {
			auto saveRes = reg.save();
			if (saveRes.isErr()) {
				geode::log::error("failed to create registry.json: {}", saveRes.unwrapErr());
			}
			return reg;
		}

		auto readRes = geode::utils::file::readString(reg.m_registryPath);
		if (readRes.isErr()) {
			geode::log::error("failed to read registry.json: {}", readRes.unwrapErr());
			return reg;
		}

		auto parseRes = matjson::parse(readRes.unwrap());
		if (parseRes.isErr()) {
			geode::log::error("registry.json is not valid json, ignoring it");
			return reg;
		}

		auto json = parseRes.unwrap();
		auto const& packsVal = json["packs"];
		if (packsVal.isArray()) {
			for (auto const& entry : packsVal) {
				auto strRes = entry.asString();
				if (strRes.isOk()) {
					reg.m_packIds.push_back(strRes.unwrap());
				}
			}
		}

		return reg;
	}

	Result<void, std::string> Registry::save() const {
		auto json = matjson::makeObject({
			{ "packs", m_packIds }
		});

		auto writeRes = geode::utils::file::writeString(m_registryPath, json.dump(matjson::TAB_INDENTATION));
		if (writeRes.isErr()) {
			return Err(writeRes.unwrapErr());
		}
		return Ok();
	}

	bool Registry::contains(std::string const& id) const {
		return std::find(m_packIds.begin(), m_packIds.end(), id) != m_packIds.end();
	}

	bool Registry::addPack(std::string const& id) {
		if (contains(id)) return false;
		m_packIds.push_back(id);
		return true;
	}

	bool Registry::removePack(std::string const& id) {
		auto it = std::find(m_packIds.begin(), m_packIds.end(), id);
		if (it == m_packIds.end()) return false;
		m_packIds.erase(it);
		return true;
	}

}
