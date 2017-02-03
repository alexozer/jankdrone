#include <Arduino.h>
#include <limits>
#include "log.h"
#include "shm.h"

template <>
Shm::Var::Var(std::string name, Group* group, int* value, int tag):
	m_name{name}, m_group{group}, m_type{Type::INT}, m_value{value}, m_tag{tag} {}

template <>
Shm::Var::Var(std::string name, Group* group, float* value, int tag):
	m_name{name}, m_group{group}, m_type{Type::FLOAT}, m_value{value}, m_tag{tag} {}

template <>
Shm::Var::Var(std::string name, Group* group, bool* value, int tag):
	m_name{name}, m_group{group}, m_type{Type::BOOL}, m_value{value}, m_tag{tag} {}

template <>
Shm::Var::Var(std::string name, Group* group, std::string* value, int tag):
	m_name{name}, m_group{group}, m_type{Type::STRING}, m_value{value}, m_tag{tag} {}

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

std::string Shm::Var::typeString(Type type) {
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

template <>
void Shm::Var::set(int value) {
	if (m_type == Type::FLOAT) {
		set((float)value);
	} else {
		verifyType<int>();
		*(int*)m_value = value;
	}
}

template <>
void Shm::Var::set(const char* value) {
	set(std::string(value));
}

template <>
float Shm::Var::get() {
	if (m_type == Type::INT) {
		return *(int*)m_value;
	} else {
		verifyType<float>();
		return *(float*)m_value;
	}
}

template <>
void Shm::Var::verifyType<int>() {
	verifyType(Type::INT);
}

template <>
void Shm::Var::verifyType<float>() {
	verifyType(Type::FLOAT);
}

template <>
void Shm::Var::verifyType<bool>() {
	verifyType(Type::BOOL);
}

template <>
void Shm::Var::verifyType<std::string>() {
	verifyType(Type::STRING);
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

std::vector<Shm::Var*> Shm::Group::array(std::string prefix) {
	std::unordered_map<int, Shm::Var*> indexMap;
	int minI = std::numeric_limits<int>::max();
	int maxI = std::numeric_limits<int>::min();
	for (auto& vItem : m_vars) {
		auto v = &vItem.second;
		auto name = v->name();
		if (name.size() > prefix.size() && name.substr(0, prefix.size()) == prefix) {
			int i = atoi(name.substr(prefix.size()).c_str());
			indexMap.emplace(i, v);
			if (i > maxI) maxI = i;
			if (i < minI) minI = i;
		}
	}

	// There's probably an elegant way to show this proves the correctness of
	// the array
	if (minI != 0 || (size_t)maxI != indexMap.size() - 1) {
		Log::fatal("Invalid shm array: %s.%s[]", m_name.c_str(), prefix.c_str());
	}

	std::vector<Shm::Var*> array(maxI + 1);
	for (auto& item : indexMap) {
		array[item.first] = item.second;
	}
	return array;
}

Shm::Shm(): m_groups{
	{"controller", &controller},
	{"controllerOut", &controllerOut},
	{"deadman", &deadman},
	{"desires", &desires},
	{"led", &led},
	{"pitchConf", &pitchConf},
	{"placement", &placement},
	{"power", &power},
	{"remote", &remote},
	{"rollConf", &rollConf},
	{"switches", &switches},
	{"temperature", &temperature},
	{"thrusters", &thrusters},
	{"yawConf", &yawConf},
} {
	for (auto& g : m_groups) {
		for (auto v : g.second->vars()) {
			m_tagMap[v->tag()] = v;
		}
	}
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
			(size_t)tag >= sizeof(m_tagMap) / sizeof(m_tagMap[0])) {
		return nullptr;
	} else {
		return m_tagMap[tag];
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
	auto it = m_groups.find(name);
	if (it == m_groups.end()) {
		return nullptr;
	} else {
		return it->second;
	}
}

std::vector<Shm::Group*> Shm::groups() {
	std::vector<Shm::Group*> gs;
	for (auto& groupPair : m_groups) {
		gs.push_back(groupPair.second);
	}
	return gs;
}


Shm::Group_controller::Group_controller():
	Group{"controller", {
		{"enabled", this, &enabled, 0},
	}},

	enabled{false}
{}
	
Shm::Group_controllerOut::Group_controllerOut():
	Group{"controllerOut", {
		{"pitch", this, &pitch, 1},
		{"roll", this, &roll, 2},
		{"yaw", this, &yaw, 3},
	}},

	pitch{0.0},
	roll{0.0},
	yaw{0.0}
{}
	
Shm::Group_deadman::Group_deadman():
	Group{"deadman", {
		{"enabled", this, &enabled, 4},
		{"maxTilt", this, &maxTilt, 5},
	}},

	enabled{true},
	maxTilt{30.0}
{}
	
Shm::Group_desires::Group_desires():
	Group{"desires", {
		{"force", this, &force, 6},
		{"pitch", this, &pitch, 7},
		{"roll", this, &roll, 8},
		{"yaw", this, &yaw, 9},
	}},

	force{0.0},
	pitch{0.0},
	roll{0.0},
	yaw{0.0}
{}
	
Shm::Group_led::Group_led():
	Group{"led", {
		{"brightness", this, &brightness, 10},
		{"maxHue", this, &maxHue, 11},
		{"minHue", this, &minHue, 12},
		{"pattern", this, &pattern, 13},
		{"periodMs", this, &periodMs, 14},
	}},

	brightness{8},
	maxHue{188},
	minHue{114},
	pattern{1},
	periodMs{250}
{}
	
Shm::Group_pitchConf::Group_pitchConf():
	Group{"pitchConf", {
		{"d", this, &d, 15},
		{"enabled", this, &enabled, 16},
		{"i", this, &i, 17},
		{"p", this, &p, 18},
	}},

	d{0.0},
	enabled{false},
	i{0.0},
	p{0.01}
{}
	
Shm::Group_placement::Group_placement():
	Group{"placement", {
		{"altitude", this, &altitude, 19},
		{"pitch", this, &pitch, 20},
		{"roll", this, &roll, 21},
		{"yaw", this, &yaw, 22},
	}},

	altitude{0.0},
	pitch{0.0},
	roll{0.0},
	yaw{0.0}
{}
	
Shm::Group_power::Group_power():
	Group{"power", {
		{"critical", this, &critical, 23},
		{"low", this, &low, 24},
		{"voltage", this, &voltage, 25},
	}},

	critical{false},
	low{false},
	voltage{0.0}
{}
	
Shm::Group_remote::Group_remote():
	Group{"remote", {
		{"connected", this, &connected, 26},
	}},

	connected{false}
{}
	
Shm::Group_rollConf::Group_rollConf():
	Group{"rollConf", {
		{"d", this, &d, 27},
		{"enabled", this, &enabled, 28},
		{"i", this, &i, 29},
		{"p", this, &p, 30},
	}},

	d{0.0},
	enabled{false},
	i{0.0},
	p{0.01}
{}
	
Shm::Group_switches::Group_switches():
	Group{"switches", {
		{"calibrateEscs", this, &calibrateEscs, 31},
		{"calibrateImu", this, &calibrateImu, 32},
		{"softKill", this, &softKill, 33},
	}},

	calibrateEscs{false},
	calibrateImu{false},
	softKill{true}
{}
	
Shm::Group_temperature::Group_temperature():
	Group{"temperature", {
		{"gyro", this, &gyro, 34},
	}},

	gyro{0.0}
{}
	
Shm::Group_thrusters::Group_thrusters():
	Group{"thrusters", {
		{"t0", this, &t0, 35},
		{"t1", this, &t1, 36},
		{"t2", this, &t2, 37},
		{"t3", this, &t3, 38},
		{"t4", this, &t4, 39},
		{"t5", this, &t5, 40},
		{"t6", this, &t6, 41},
		{"t7", this, &t7, 42},
	}},

	t0{0.0},
	t1{0.0},
	t2{0.0},
	t3{0.0},
	t4{0.0},
	t5{0.0},
	t6{0.0},
	t7{0.0}
{}
	
Shm::Group_yawConf::Group_yawConf():
	Group{"yawConf", {
		{"d", this, &d, 43},
		{"enabled", this, &enabled, 44},
		{"i", this, &i, 45},
		{"p", this, &p, 46},
	}},

	d{0.0},
	enabled{false},
	i{0.0},
	p{0.001}
{}
	

Shm& shm() {
	static Shm shm;
	return shm;
}
