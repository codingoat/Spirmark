#include "StructuralSimilarity.h"
#include <glm/glm.hpp>


namespace Spirmark 
{

double CalculateStructuralSimilarity(const GLubyte* original, const GLubyte* modified, int resx, int resy)
{
	// implementation based on SSIM paper
	using llu = long long unsigned int;
	double k1 = 0.01, k2 = 0.01;
	double mux = 0, muy = 0, sigx = 0, sigy = 0, sigxy = 0, c1 = k1 * k1, c2 = k2 * k2;

	for (int i = 0; i < resx * resy; i++)
	{
		glm::vec3 oPixel = glm::u8vec3(original[i * 3] / 255.0, original[i * 3 + 1] / 255.0, original[i * 3 + 2] / 255.0);
		mux += 0.375 * oPixel.r + 0.5 * oPixel.g + 0.125 * oPixel.b;
		glm::u8vec3 mPixel = glm::u8vec3(modified[i * 3] / 255.0, modified[i * 3 + 1] / 255.0, modified[i * 3 + 2] / 255.0);
		muy += 0.375 * mPixel.r + 0.5 * mPixel.g + 0.125 * mPixel.b;
	}
	mux /= (llu)resx * resy;
	muy /= (llu)resx * resy;

	for (int i = 0; i < resx * resy; i++)
	{
		glm::vec3 oPixel = glm::u8vec3(original[i * 3] / 255.0, original[i * 3 + 1] / 255.0, original[i * 3 + 2] / 255.0);
		glm::u8vec3 mPixel = glm::u8vec3(modified[i * 3] / 255.0, modified[i * 3 + 1] / 255.0, modified[i * 3 + 2] / 255.0);

		double lumx = 0.375 * oPixel.r + 0.5 * oPixel.g + 0.125 * oPixel.b;
		double lumy = 0.375 * mPixel.r + 0.5 * mPixel.g + 0.125 * mPixel.b;
		sigx += (lumx - mux) * (lumx - mux);
		sigy += (lumy - muy) * (lumy - muy);
		sigxy += (lumx - mux) * (lumy - muy);
	}

	sigx = std::sqrt(sigx / ((llu)resx * resy - 1));
	sigy = std::sqrt(sigy / ((llu)resx * resy - 1));
	sigxy = sigxy / ((llu)resx * resy - 1);

	double ssim = double(2 * mux * muy + c1) * double(2 * sigxy + c2) / double(mux * mux + muy * muy + c1) / double(sigx * sigx + sigy * sigy + c2);

	return ssim;
}

}

