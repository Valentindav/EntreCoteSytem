#pragma once

class GameTimer
{
public:
	GameTimer();

	float TotalTime()const;
	float DeltaTime()const;

	void Reset(); 
	void Start(); 
	void Stop();  
	void Tick();  

	void ToggleSlowTimeDown();
private:
	double mSecondsPerCount;
	double mDeltaTime;

	bool mIsSlowed = false;
	float mTimeScale = 1.0f;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurrTime;

	bool mStopped;
};
