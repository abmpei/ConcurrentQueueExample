#include "stdafx.h"
#include "Stopper.h"

// Stopper without any flag
NoStop* NoStop::self = 0;

NoStop::operator bool() const 
{ 
	return false; 
}

NoStop& NoStop::Instance()
{
	if (!self)
		SelfCreate();
	return *self;
}

void NoStop::SelfCreate()
{
	static NoStop obj;
	self = &obj;
}


// Thread safe stopper
Stopper::Stopper() 
{ 
	m_stop = false; 
}

Stopper::operator bool() const 
{ 
	return m_stop; 
}

inline void Stopper::stop(bool bFlag /*= true*/) 
{ 
	m_stop = bFlag; 
}
