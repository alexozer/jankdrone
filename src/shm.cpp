#include <Arduino.h>
#include "logger.h"
#include "shm.h"

Shm::Var::Var(std::string name, int value):
	m_name{name}, m_type{Type::INT}, m_intValue{value} {}

Shm::Var::Var(std::string name, float value):
	m_name{name}, m_type{Type::FLOAT}, m_floatValue{value} {}

Shm::Var::Var(std::string name, bool value):
	m_name{name}, m_type{Type::BOOL}, m_boolValue{value} {}

Shm::Var::Var(std::string name, std::string value):
	m_name{name}, m_type{Type::STRING}, m_stringValue{value} {}

Shm::Var::Var(std::string name, const char* value):
	Var{name, std::string(value)} {}

std::string Shm::Var::name() {
	return m_name;
}

Shm::Var::Type Shm::Var::type() {
	return m_type;
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

std::string Shm::Var::typeString(Type type) {
	switch (type) {
		case Type::INT:
			return "int";
		case Type::FLOAT:
			return "float";
		case Type::BOOL:
			return "bool";
		case Type::STRING:
			return "string";
	}
	return "";
}

void Shm::Var::verifyType(Type type) {
	std::string typeName;
	if (type != m_type) {
		Logger::error("Variable " + m_name + 
				" has type " + typeString(m_type) + 
				", not type " + typeString(type));
		exit(1);
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
	auto it = m_vars.find(name);
	if (it == m_vars.end()) {
		Logger::error("Variable " + name + " not found");
		exit(1);
	}

	return &it->second;
}

std::vector<Shm::Var*> Shm::Group::vars() {
	std::vector<Var*> varList(m_vars.size());
	for (auto& varPair : m_vars) {
		varList.push_back(&varPair.second);
	}
	return varList;
}

Shm::Var* Shm::var(std::string name) {
	auto dotPos = name.find('.');
	if (dotPos == std::string::npos) {
		Logger::error(name + " is not a valid variable path");
		exit(1);
	}
	auto groupName = name.substr(0, dotPos);
	auto varName = name.substr(dotPos + 1);

	return group(groupName)->var(varName);
}

Shm::Group* Shm::group(std::string name) {
	auto it = get().m_groups.find(name);
	if (it == get().m_groups.end()) {
		Logger::error("Group " + name + " not found");
		exit(1);
	}

	return &it->second;
}

Shm::Shm(std::vector<Group> groups) {
	for (auto& g : groups) {
		m_groups.emplace(g.name(), g);
	}
}

Shm& Shm::get() {
	static Shm shm{{
		{"motorDesires", {
			{"front", 0},
			{"left", 0},
			{"right", 0},
			{"back", 0},
		}},

		{"controller", {
			{"enabled", false},
			{"heading", 0.0},
			{"tiltHeading", 0.0},
			{"tiltAngle", 0.0},
			{"verticalForce", 0.0},
		}},

		{"led", {
			{"front", 0},
			{"left", 0},
			{"right", 0},
			{"back", 0},
		}},
	}};

	return shm;
}
