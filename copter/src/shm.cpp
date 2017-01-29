#include <Arduino.h>
#include "log.h"
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

std::string Shm::Var::path() {
	return m_group->name() + "." + m_name;
}

Shm::Group* Shm::Var::group() {
	return m_group;
}

void Shm::Var::set(int value) {
	if (m_type == Type::FLOAT) {
		set((float)value);
	} else {
		verifyType(Type::INT);
		m_intValue = value;
	}
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

void Shm::Var::setGroup(Group* group) {
	m_group = group;
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
		Log::fatal("Variable %s has type %s not type %s",
				m_name.c_str(), typeString(m_type).c_str(), typeString(type).c_str());
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
		Log::fatal("Variable %s not found", name.c_str());
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
		Log::fatal("Variable %s not found", name.c_str());
	}

	return v;
}

Shm::Var* Shm::var(int tag) {
	auto v = varIfExists(tag);
	if (!v) {
		Log::fatal("Variable tag %d not found", tag);
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
	if (tag < 0 || 
			(size_t)tag >= sizeof(get().m_tagMap) / sizeof(get().m_tagMap[0])) {
		return nullptr;
	} else {
		return get().m_tagMap[tag];
	}
}

Shm::Group* Shm::group(std::string name) {
	auto g = groupIfExists(name);
	if (!g) {
		Log::fatal("Group %s not found", name.c_str());
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
		auto& placedG = m_groups.emplace(g.name(), g).first->second;
		for (auto var : placedG.vars()) {
			m_tagMap[var->tag()] = var;
			var->setGroup(&placedG);
		}
	}
}
