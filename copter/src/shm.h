#pragma once

#include <unordered_map>
#include <vector>
#include <string>

class Shm {
	public:

		class Group;
		class Group_controller;
		class Group_controllerOut;
		class Group_deadman;
		class Group_desires;
		class Group_led;
		class Group_pitchConf;
		class Group_placement;
		class Group_power;
		class Group_remote;
		class Group_rollConf;
		class Group_switches;
		class Group_temperature;
		class Group_thrusters;
		class Group_yawConf;

		class Var {
			friend class Group;
			friend class Group_controller;
			friend class Group_controllerOut;
			friend class Group_deadman;
			friend class Group_desires;
			friend class Group_led;
			friend class Group_pitchConf;
			friend class Group_placement;
			friend class Group_power;
			friend class Group_remote;
			friend class Group_rollConf;
			friend class Group_switches;
			friend class Group_temperature;
			friend class Group_thrusters;
			friend class Group_yawConf;

			public:
				enum class Type { INT, FLOAT, BOOL, STRING };

				std::string name();
				Type type();
				int tag();
				std::string path();
				Group* group();

				template <typename T>
				void set(T value) {
					verifyType<T>();
					*(T*)m_value = value;
				}

				template <typename T>
				T get() {
					verifyType<T>();
					return *(T*)m_value;
				}

				template <typename T>
				T* ptr() {
					verifyType<T>();
					return (T*)m_value;
				}

				static std::string typeString(Type type);

			protected:
				template <typename T>
				Var(std::string name, Group* group, T* value, int tag);

			private:
				std::string m_name;
				Group* m_group;
				Type m_type;
				void* m_value;
				int m_tag;

				template <typename T>
				void verifyType();

				void verifyType(Type type);
		};

		class Group {
			public:
				std::string name();
				Var* var(std::string name);
				Var* varIfExists(std::string name);
				std::vector<Var*> vars();
				std::vector<Var*> array(std::string prefix);

			protected:
				Group(std::string name, std::vector<Var> vars);

			private:
				std::string m_name;
				std::unordered_map<std::string, Var> m_vars;
		};

		class Group_controller : public Group {
			public:
				Group_controller();

				bool enabled;
		};
		Group_controller controller;

		class Group_controllerOut : public Group {
			public:
				Group_controllerOut();

				float pitch;
				float roll;
				float yaw;
		};
		Group_controllerOut controllerOut;

		class Group_deadman : public Group {
			public:
				Group_deadman();

				bool enabled;
				float maxTilt;
		};
		Group_deadman deadman;

		class Group_desires : public Group {
			public:
				Group_desires();

				float force;
				float pitch;
				float roll;
				float yaw;
		};
		Group_desires desires;

		class Group_led : public Group {
			public:
				Group_led();

				int brightness;
				int maxHue;
				int minHue;
				int pattern;
				int periodMs;
		};
		Group_led led;

		class Group_pitchConf : public Group {
			public:
				Group_pitchConf();

				float d;
				bool enabled;
				float i;
				float p;
		};
		Group_pitchConf pitchConf;

		class Group_placement : public Group {
			public:
				Group_placement();

				float altitude;
				float pitch;
				float roll;
				float yaw;
		};
		Group_placement placement;

		class Group_power : public Group {
			public:
				Group_power();

				bool critical;
				bool low;
				float voltage;
		};
		Group_power power;

		class Group_remote : public Group {
			public:
				Group_remote();

				bool connected;
		};
		Group_remote remote;

		class Group_rollConf : public Group {
			public:
				Group_rollConf();

				float d;
				bool enabled;
				float i;
				float p;
		};
		Group_rollConf rollConf;

		class Group_switches : public Group {
			public:
				Group_switches();

				bool calibrateEscs;
				bool calibrateImu;
				bool softKill;
		};
		Group_switches switches;

		class Group_temperature : public Group {
			public:
				Group_temperature();

				float gyro;
		};
		Group_temperature temperature;

		class Group_thrusters : public Group {
			public:
				Group_thrusters();

				float t0;
				float t1;
				float t2;
				float t3;
				float t4;
				float t5;
				float t6;
				float t7;
		};
		Group_thrusters thrusters;

		class Group_yawConf : public Group {
			public:
				Group_yawConf();

				float d;
				bool enabled;
				float i;
				float p;
		};
		Group_yawConf yawConf;

		Shm();
		Shm(const Shm&) = delete;
		Shm(Shm&&) = delete;
		Shm& operator=(const Shm&) = delete;
		Shm& operator=(Shm&&) = delete;

		Var* var(std::string name);
		Var* var(int tag);
		Var* varIfExists(std::string name);
		Var* varIfExists(int tag);

		Group* group(std::string name);
		Group* groupIfExists(std::string name);
		std::vector<Group*> groups();
	
	private:
		std::unordered_map<std::string, Group*> m_groups;
		Var* m_tagMap[47];
};

template <>
void Shm::Var::set(int value);

template <>
void Shm::Var::set(const char* value);

template <>
float Shm::Var::get();

template <>
void Shm::Var::verifyType<int>();

template <>
void Shm::Var::verifyType<float>();

template <>
void Shm::Var::verifyType<bool>();

template <>
void Shm::Var::verifyType<std::string>();

Shm& shm();
