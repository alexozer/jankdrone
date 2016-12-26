#include <Arduino.h>
#include "logger.h"
#include "shm.h"

Shm::Var::Var(std::string name, int value, int tag):
	m_name{name}, m_type{Type::INT}, m_tag{tag}, m_intValue{value} {}

Shm::Var::Var(std::string name, float value, int tag):
	m_name{name}, m_type{Type::FLOAT}, m_tag{tag}, m_floatValue{value} {}

Shm::Var::Var(std::string name, bool value, int tag):
	m_name{name}, m_type{Type::BOOL}, m_tag{tag}, m_boolValue{value} {}

Shm::Var::Var(std::string name, std::string value, int tag):
	m_name{name}, m_type{Type::STRING}, m_tag{tag}, m_stringValue{value} {}

Shm::Var::Var(std::string name, const char* value, int tag):
	Var{name, std::string(value), tag} {}

std::string Shm::Var::name() {
	return m_name;
}

Shm::Var::Type Shm::Var::type() {
	return m_type;
}

int Shm::Var::tag() {
	return m_tag;
}

void Shm::Var::set(int value) {
	verifyType(Type::INT);
	m_intValue = value;
}

void Shm::Var::set(float value) {
	verifyType(Type::FLOAT);
	m_floatValue = value;
}

void Shm::Var::set(bool value) {
	verifyType(Type::BOOL);
	m_boolValue = value;
}

void Shm::Var::set(std::string value) {
	verifyType(Type::STRING);
	m_stringValue = value;
}

int Shm::Var::getInt() {
	verifyType(Type::INT);
	return m_intValue;
}

float Shm::Var::getFloat() {
	verifyType(Type::FLOAT);
	return m_floatValue;
}

bool Shm::Var::getBool() {
	verifyType(Type::BOOL);
	return m_boolValue;
}

std::string Shm::Var::getString() {
	verifyType(Type::STRING);
	return m_stringValue;
}

std::string Shm::typeString(Var::Type type) {
	switch (type) {
		case Var::Type::INT:
			return "int";
		case Var::Type::FLOAT:
			return "float";
		case Var::Type::BOOL:
			return "bool";
		case Var::Type::STRING:
			return "string";
	}
	return "";
}

void Shm::Var::verifyType(Type type) {
	std::string typeName;
	if (type != m_type) {
		Logger::fatal("Variable {} has type {}, not type {}",
				m_name, typeString(m_type), typeString(type));
	}
}

Shm::Group::Group(std::string name, std::vector<Var> vars) {
	m_name = name;
	for (auto& v : vars) {
		m_vars.emplace(v.name(), v);
	}
}

std::string Shm::Group::name() {
	return m_name;
}

Shm::Var* Shm::Group::var(std::string name) {
	auto var = varIfExists(name);
	if (!var) {
		Logger::fatal("Variable {} not found", name);
	}

	return var;
}

Shm::Var* Shm::Group::varIfExists(std::string name) {
	auto it = m_vars.find(name);
	if (it != m_vars.end()) {
		return &it->second;
	} else {
		return nullptr;
	}
}

std::vector<Shm::Var*> Shm::Group::vars() {
	std::vector<Var*> varList;
	for (auto& varPair : m_vars) {
		varList.push_back(&varPair.second);
	}
	return varList;
}

Shm::Var* Shm::var(std::string name) {
	auto v = varIfExists(name);
	if (!v) {
		Logger::fatal("Variable {} not found", name);
	}

	return v;
}

Shm::Var* Shm::var(int tag) {
	auto v = varIfExists(tag);
	if (!v) {
		Logger::fatal("Variable tag {} not found", tag);
	}

	return v;
}

Shm::Var* Shm::varIfExists(std::string name) {
	auto dotPos = name.find('.');
	if (dotPos == std::string::npos) {
		return nullptr;
	}

	auto groupName = name.substr(0, dotPos);
	auto g = groupIfExists(groupName);
	if (g) {
		auto varName = name.substr(dotPos + 1);
		return g->varIfExists(varName);
	} else {
		return nullptr;
	}
}

Shm::Var* Shm::varIfExists(int tag) {
	auto it = get().m_tagMap.find(tag);
	if (it == get().m_tagMap.end()) {
		return nullptr;
	} else {
		return it->second;
	}
}

Shm::Group* Shm::group(std::string name) {
	auto g = groupIfExists(name);
	if (!g) {
		Logger::fatal("Group {} not found", name);
	}

	return g;
}

Shm::Group* Shm::groupIfExists(std::string name) {
	auto it = get().m_groups.find(name);
	if (it == get().m_groups.end()) {
		return nullptr;
	} else {
		return &it->second;
	}
}

std::vector<Shm::Group*> Shm::groups() {
	std::vector<Shm::Group*> gs;
	for (auto& groupPair : get().m_groups) {
		gs.push_back(&groupPair.second);
	}
	return gs;
}

Shm::Shm(std::vector<Group> groups) {
	for (auto& g : groups) {
		for (auto var : g.vars()) {
			int tag = var->tag();
			auto it = m_tagMap.find(tag);
			if (it != m_tagMap.end()) {
				Logger::fatal("Duplicate tag {}", tag);
			} else {
				m_tagMap.emplace(tag, var);
			}
		}
		m_groups.emplace(g.name(), g);
	}
}


Shm& Shm::get() {
	static Shm shm{{
		{"controller", {
			{"enabled", false, 1},
			{"pitch", 0.0, 2},
			{"pitchD", 0.0, 3},
			{"pitchEnabled", false, 4},
			{"pitchI", 0.0, 5},
			{"pitchP", 1.0, 6},
			{"roll", 0.0, 7},
			{"rollD", 0.0, 8},
			{"rollEnabled", true, 9},
			{"rollI", 0.0, 10},
			{"rollP", 1.0, 11},
			{"verticalForce", 0.0, 12},
			{"yaw", 0.0, 13},
			{"yawD", 0.0, 14},
			{"yawEnabled", true, 15},
			{"yawI", 0.0, 16},
			{"yawP", 1.0, 17},
		}},

		{"leds", {
			{"back", 0, 18},
			{"front", 0, 19},
			{"left", 0, 20},
			{"right", 0, 21},
		}},

		{"motors", {
			{"back", 0.0, 22},
			{"front", 0.0, 23},
			{"left", 0.0, 24},
			{"right", 0.0, 25},
		}},

		{"placement", {
			{"altitude", 0.0, 26},
			{"pitch", 0.0, 27},
			{"roll", 0.0, 28},
			{"yaw", 0.0, 29},
		}},

		{"temperature", {
			{"gyro", 0.0, 30},
		}},

	}};

	return shm;
}
