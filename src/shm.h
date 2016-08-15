#pragma once

#include <unordered_map>
#include <vector>
#include <string>

class Shm {
	public:

		class Var {
			public:
				enum class Type {INT, FLOAT, BOOL, STRING};

				Var(std::string name, int value);
				Var(std::string name, float value);
				Var(std::string name, bool value);
				Var(std::string name, std::string value);
				Var(std::string name, const char* value);

				std::string name();
				Type type();

				void set(int value);
				void set(float value);
				void set(bool value);
				void set(std::string value);
				void set(const char* value);

				int getInt();
				float getFloat();
				bool getBool();
				std::string getString();

			private:
				std::string m_name;
				Type m_type;

				union {
					int m_intValue;
					float m_floatValue;
					bool m_boolValue;
				};
				std::string m_stringValue; // Can't go in union, can't destruct

				std::string typeString(Type type);
				void verifyType(Type type);
		};

		class Group {
			public:
				Group(std::string name, std::vector<Var> vars);

				std::string name();
				Var* var(std::string name);
				std::vector<Var*> vars();

			private:
				std::string m_name;
				std::unordered_map<std::string, Var> m_vars;
		};

		Shm(const Shm&) = delete;
		void operator=(const Shm&) = delete;

		static Var* var(std::string name);
		static Group* group(std::string name);
		static std::vector<Group*> groups();
	
	private:
		Shm(std::vector<Group> groups);
		static Shm& get();

		std::unordered_map<std::string, Group> m_groups;
};
