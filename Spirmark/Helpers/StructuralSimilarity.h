#pragma once
#include <GL/glew.h>
#include <numeric>
#include <cmath>
#include <UniformValues.h>

namespace Spirmark
{

/// Stores image sim values of one frame
struct ImageSimilarity
{
	GLuint absoluteDifferenceSum = 0;
	double ssim = 1;

	ImageSimilarity() {};
	ImageSimilarity(GLuint absoluteDifferenceSum, double ssim) : absoluteDifferenceSum(absoluteDifferenceSum), ssim(ssim) {};
};

/// Stores avg and worst imagesim values (with uniform variable values for the latter)
struct ImageSimilarityStats
{
	double absoluteDifferencAvg = 0;
	double ssimAvg = 1;
	long long unsigned int frameCounter = 0;

	ImageSimilarity worstSimilarity;
	UniformValues worstUniformValues;
	
	void AddStat(ImageSimilarity is, UniformValues uv)
	{
		frameCounter++;
		absoluteDifferencAvg += (is.absoluteDifferenceSum - absoluteDifferencAvg) / (double)frameCounter;
		ssimAvg += (is.ssim - ssimAvg) / frameCounter;

		if (frameCounter == 1 || is.absoluteDifferenceSum > worstSimilarity.absoluteDifferenceSum)
		{
			worstSimilarity.absoluteDifferenceSum = is.absoluteDifferenceSum;
			worstUniformValues = uv;
		}
		if (frameCounter == 1 || is.ssim < worstSimilarity.ssim)
		{
			worstSimilarity.ssim = is.ssim;
		}
	}
};

/// original, modified: 8 bit RGB pixels
double CalculateStructuralSimilarity(const GLubyte* original, const GLubyte* modified, int resx, int resy);

}