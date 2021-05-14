#pragma once

#include <vector>
#include <chrono>
#include <string>
#include <GL/glew.h>
#include <cmath>

#define GPUTIME_RINGBUFFER_SIZE 5
#define DISREGARD_AT_LEAST_FIRST_N_FRAMES 5
#define FRAMETIME_HISTORY_COUNT 20
#define DISREGARD_FIRST_N_FRAMES std::max<int>(GPUTIME_RINGBUFFER_SIZE, DISREGARD_AT_LEAST_FIRST_N_FRAMES)

struct FrameTime {
	double total = 0;
	double gpu = 0;

	FrameTime() {}
	FrameTime(double total, double gpu) : total(total), gpu(gpu){}

	FrameTime operator+(const FrameTime& ft)
	{
		return FrameTime{ this->total + ft.total, this->gpu + ft.gpu };
	}
	FrameTime operator-(const FrameTime& ft)
	{
		return FrameTime{ this->total - ft.total, this->gpu - ft.gpu };
	}
	FrameTime operator*(const FrameTime& ft)
	{
		return FrameTime{ this->total * ft.total, this->gpu * ft.gpu };
	}
	FrameTime operator/(const FrameTime& ft)
	{
		return FrameTime{ this->total / ft.total, this->gpu / ft.gpu };
	}
};
FrameTime sqrt(const FrameTime& ft);

struct BenchmarkResult {
	FrameTime mean, stdev;
};
std::ostream& operator<<(std::ostream& os, BenchmarkResult b);

/// Get performance data and benchmark running application, frame by frame
class Bencher
{
public:
	Bencher();
	~Bencher();

	/// Call just before starting to draw objects
	void GpuStart() {
		glBeginQuery(GL_TIME_ELAPSED, gpuTimeQueries[totalFrames % GPUTIME_RINGBUFFER_SIZE]);
	}
	/// Call just after being done with drawing
	void GpuDone() { glEndQuery(GL_TIME_ELAPSED); }
	/// Call after being done with the entire frame
	void FrameDone();

	FrameTime GetLastFrameTime() { return lastFrameTime; }
	const std::vector<FrameTime>& GetFrameTimeHistory() { return frameTimeHistory; }
	int GetFps();
	/// Amount of frames registered so far
	int GetTotalFrameCount() { return totalFrames; };
	double GetTimeSinceLastBenchmark() { return timeSinceLastBenchmark; }

	/// secs: approximate length of benchmark in seconds
	void StartBenchmark(float secs);
	void CancelBenchmark();
	bool IsBenchmarkDone();
	float GetRemainingBenchmarkTime() { return desiredBenchmarkLength - timeSinceLastBenchmark; };
	BenchmarkResult GetBenchmarkResults();

private:
	FrameTime lastFrameTime;
	int totalFrames = 0;
	std::vector<FrameTime> frameTimeHistory; // the last few frameTimes, max size is FRAMETIME_HISTORY_COUNT
	double timeSinceLastBenchmark = 0;

	std::chrono::time_point<std::chrono::high_resolution_clock> lastFrame;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTimeHistory;

	void AddFrameTime(FrameTime frameTime);

	bool benchmarking = false;
	FrameTime benchedSum, benchedSumSq, benchedComp, benchedCompSq;
	int benchedFrameCounter;
	double desiredBenchmarkLength;

	GLuint gpuTimeQueries[GPUTIME_RINGBUFFER_SIZE] = { 0 };
	float GetGpuTime();
};