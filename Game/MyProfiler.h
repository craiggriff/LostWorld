#pragma once

namespace Game
{

	class MyProfiler
	{
	public:
		MyProfiler(AllResources* p_Resources);
		~MyProfiler(void);

		void Start();
		void Stop();
		void Display();

		double tick_seconds;
		unsigned long ticks;

	private:
		AllResources* m_Resources;
	};
};

