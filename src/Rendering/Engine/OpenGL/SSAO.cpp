#include "SSAO.h"

#include <random>
#include <glad/glad.h>

#include "screen.vert.h"
#include "ssao.frag.h"

namespace dyno
{
	SSAO::SSAO()
	{
		mWidth = mHeight = 0;

		// shader programs
		mSSAOProgram = gl::Program::createProgramSPIRV(
			SCREEN_VERT, sizeof(SCREEN_VERT),
			SSAO_FRAG, sizeof(SSAO_FRAG));

		// SSAO kernel
		mSSAOKernelUBO.create(GL_UNIFORM_BUFFER, GL_STATIC_DRAW);
		mSSAOKernelUBO.bindBufferBase(3);

		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
		std::default_random_engine generator;
		std::vector<glm::vec3> ssaoKernel;
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)
			);
			sample = glm::normalize(sample);
			//sample *= randomFloats(generator);
			//ssaoKernel.push_back(sample);
			float scale = (float)i / 64.0;
			//scale = lerp(0.1f, 1.0f, scale * scale);
			scale = 0.1f + scale * scale * 0.9f;
			sample *= scale;
			ssaoKernel.push_back(sample);
		}

		mSSAOKernelUBO.load(ssaoKernel.data(), ssaoKernel.size() * sizeof(glm::vec3));

		// create SSAO noise here...
		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		mSSAONoiseTex.format = GL_RGB;
		mSSAONoiseTex.internalFormat = GL_RGB32F;
		mSSAONoiseTex.create();
		mSSAONoiseTex.load(4, 4, &ssaoNoise[0]);
	}

	SSAO::~SSAO()
	{

	}

	void SSAO::resize(unsigned int w, unsigned int h)
	{
		if (w == mWidth && h == mHeight)
			return;

		mWidth = w;
		mHeight = h;
	}
}


