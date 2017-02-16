#pragma once

#include <Arduino.h>

template <typename T, size_t size>
class Queue {
	public:
		Queue(): m_back{0} {}

		constexpr size_t size() {
			return size;
		}

		T& front() {
			return m_back != 0 ? m_elems[0] : T();
		}

		T& back() {
			return m_back != 0 ? m_elems[m_back - 1] : T();
		}

		void push(T v) {
			if (m_back == size) {
				pop();
				m_back--;
			}
			m_elems[m_back++] = v;
		}

		void pop() {
			if (m_back != 0) {
				for (int i = 1; i < m_back; i++) {
					m_elems[i-1] = std::ref(m_elems[i]);
				}
				m_back--;
			}
		}

	private:
		T m_elems[size];
		size_t m_back;
};
