#pragma once

#include <vector>
#include <initializer_list>
#include "shm.h"

class Led {
	public:
		Led();

		void operator()();
		void showStatus();

	private:

		class PieceFunc {
			public:
				typedef std::pair<int, int> Coord;

				PieceFunc(std::initializer_list<Coord> pts);
				int operator()(int time);

			private:
				std::vector<Coord> m_pts;
		};

		Shm::Var *m_front, *m_back, *m_left, *m_right;
		PieceFunc m_unkilled;
};
