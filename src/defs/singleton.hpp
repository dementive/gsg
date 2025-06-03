#pragma once

#define SINGLETON(m_class) \
public: \
	static inline m_class *self = nullptr; \
	m_class() { \
		if (self == nullptr) { \
			self = this; \
		} \
	}