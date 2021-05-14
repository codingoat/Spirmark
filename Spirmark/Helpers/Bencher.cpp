#include <Bencher.h>
#include <numeric>
#include <ostream>
#include <fstream>
#include <algorithm>
#include <iostream>



Bencher::Bencher()
{
	lastFrame = lastFrameTimeHistory = std::chrono::high_resolution_clock::now();
	glGenQueries(GPUTIME_RINGBUFFER_SIZE, gpuTimeQueries);
	frameTimeHistory.reserve(FRAMETIME_HISTORY_COUNT);
}

Bencher::~Bencher()
{
	glDeleteQueries(GPUTIME_RINGBUFFER_SIZE, gpuTimeQueries);
}

FrameTime sqrt(const FrameTime& ft)
{
	return { sqrt(ft.total), sqrt(ft.gpu) };
}

std::ostream& operator<<(std::ostream& os, BenchmarkResult b)
{
	return os << "Mean: " << b.mean.total << " | Stdev: " << b.stdev.total << std::endl
		<< "Mean GPU: " << b.mean.gpu << " | Stdev GPU: " << b.stdev.gpu << std::endl;
}

void Bencher::FrameDone()
{
	auto now = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrame).count();
	float gpuTime = totalFrames >= GPUTIME_RINGBUFFER_SIZE - 1 ? GetGpuTime() : 0; // we might have stale data in the ringbuffer for the first BUFFER_SIZE-1 frames

	AddFrameTime({ duration * 0.001, gpuTime });
	lastFrame = now;
}

void Bencher::AddFrameTime(FrameTime frameTime)
{
	lastFrameTime = frameTime;
	timeSinceLastBenchmark += frameTime.total / 1000;

	// update history twice per second
	if (std::chrono::duration_cast<std::chrono::milliseconds>(lastFrame - lastFrameTimeHistory).count() > 500)
	{
		// push new to end, delete oldest if needed
		if (frameTimeHistory.size() < FRAMETIME_HISTORY_COUNT) frameTimeHistory.push_back(frameTime);
		else
		{
			std::copy(frameTimeHistory.begin() + 1, frameTimeHistory.end(), frameTimeHistory.begin());
			frameTimeHistory[FRAMETIME_HISTORY_COUNT - 1] = frameTime;
		}
		
		lastFrameTimeHistory = lastFrame;
	}
	
	// sum and squared sum to calc avg and stdev later
	if (benchmarking && totalFrames > DISREGARD_FIRST_N_FRAMES)
	{
		// naive implementation
        //benchedSum = benchedSum + frameTime;
        //benchedSumSq = benchedSumSq + frameTime * frameTime;

		// kahan
		FrameTime corrected = frameTime - benchedComp;
		FrameTime newSum = benchedSum + corrected;
		benchedComp = (newSum - benchedSum) - corrected;
		benchedSum = newSum;
		// kahan sq
		FrameTime frameTimeSq = frameTime * frameTime;
		FrameTime correctedSq = frameTimeSq - benchedCompSq;
		FrameTime newSumSq = benchedSumSq + correctedSq;
		benchedCompSq = (newSumSq - benchedSumSq) - correctedSq;
		benchedSumSq = newSumSq;
		// counter
		benchedFrameCounter++;

		//benchmarkTimer -= frameTime.total / 1000.0f;
		benchmarking = !IsBenchmarkDone();
	}

	totalFrames++;
}

int Bencher::GetFps()
{
	return 1000 / lastFrameTime.total;
}

void Bencher::StartBenchmark(float secs)
{
	benchmarking = true;
	desiredBenchmarkLength = secs;
	benchedSum = benchedSumSq = benchedComp = benchedCompSq = FrameTime();
	timeSinceLastBenchmark = totalFrames = benchedFrameCounter = 0;

	lastFrame = std::chrono::high_resolution_clock::now(); // init should not count in the benchmark
}

void Bencher::CancelBenchmark()
{
	benchmarking = false;
	totalFrames = 0;
}

bool Bencher::IsBenchmarkDone()
{
	return timeSinceLastBenchmark > desiredBenchmarkLength;
}

BenchmarkResult Bencher::GetBenchmarkResults()
{
	FrameTime counter = FrameTime(benchedFrameCounter, benchedFrameCounter);
	FrameTime mean = benchedSum / counter;
    FrameTime stdev = sqrt((benchedSumSq - (benchedSum * benchedSum) / counter) / counter);

	return { mean, stdev };
}

float Bencher::GetGpuTime()
{
	GLuint64 time = 0;
	glGetQueryObjectui64v(gpuTimeQueries[(totalFrames - GPUTIME_RINGBUFFER_SIZE + 1) % GPUTIME_RINGBUFFER_SIZE], GL_QUERY_RESULT_NO_WAIT, &time);
	if (time == 0 && totalFrames > 5) std::cout << "GPU frametime error: result not ready" << std::endl;
	return time / 1000000.0;
}