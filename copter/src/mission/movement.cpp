#include "mission.h"

using namespace mission;

Setter::Setter(V<float> target, 
		std::function<void(float)> desireSetter, 
		V<float> current, 
		float deadBand, 
		bool moduloError):
	m_target{target},
	m_desireSetter{desireSetter},
	m_current{current},
	m_deadBand{deadBand},
	m_moduloError{moduloError} {}
