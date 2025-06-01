#pragma once

#define Singleton(m_class) \
	static inline m_class *self = nullptr; \
	m_class() { \
		if (self == nullptr) { \
			self = this; \
		} \
	}