#pragma once
#include <atomic>


// ===== Stopper flag classes
struct IStopper
{
	virtual ~IStopper() {}
	virtual operator bool() const = 0;
	virtual void stop(bool bFlag = true) = 0;
};

// Stopper without any flag
class NoStop : public IStopper
{
	NoStop() {};
	static void SelfCreate();

public:
	operator bool() const override;
	void stop(bool bFlag = true) override {};

	static NoStop& Instance();
	virtual ~NoStop() {};

private:
	static NoStop* self;
};


// Thread safe stopper
class Stopper : public IStopper
{
	Stopper(const Stopper&);

public:
	Stopper();
	operator bool() const override;
	inline void stop(bool bFlag = true) override;

private:
	std::atomic_bool m_stop;
};

