#include "GLRenderHelper.h"

#include "gl/Shader.h"
#include "gl/Mesh.h"
#include "gl/Texture.h"

#include <glad/glad.h>
#include <vector>

#include "plane.vert.h"
#include "plane.frag.h"
#include "bbox.vert.h"
#include "bbox.frag.h"
#include "screen.vert.h"
#include "background.frag.h"

namespace dyno
{
	class GroundRenderer
	{
	public:
		GroundRenderer()
		{
			mProgram = gl::Program::createProgramSPIRV(
				PLANE_VERT, sizeof(PLANE_VERT),
				PLANE_FRAG, sizeof(PLANE_FRAG));
			mPlane = gl::Mesh::Plane(1.f);

			// create ruler texture
			const int k = 50;
			const int w = k * 10 + 1;
			const int h = k * 10 + 1;
			std::vector<char> img(w * h, 0);
			for (int j = 0; j < h; ++j)
			{
				for (int i = 0; i < w; ++i)
				{
					if (j == 0 || i == 0 || j == (h - 1) || i == (w - 1))
						img[j * w + i] = 192;
					else if ((j % k) == 0 || (i % k) == 0)
						img[j * w + i] = 128;
				}
			}

			mRulerTex.minFilter = GL_LINEAR_MIPMAP_LINEAR;
			mRulerTex.maxFilter = GL_LINEAR;
			mRulerTex.format = GL_RED;
			mRulerTex.internalFormat = GL_RED;
			mRulerTex.type = GL_UNSIGNED_BYTE;

			mRulerTex.create();
			mRulerTex.load(w, h, img.data());
			mRulerTex.genMipmap();
			// set anisotropy
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16);

			mUniformBlock.create(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
		}

		~GroundRenderer()
		{
			mRulerTex.release();
			
			mPlane->release();
			delete mPlane;

			mProgram->release();
			delete mProgram;

			mUniformBlock.release();
		}


		void draw(const RenderParams& rparams, float planeScale, float rulerScale)
		{
			mUniformBlock.load((void*)&rparams, sizeof(RenderParams));
			mUniformBlock.bindBufferBase(0);

			mRulerTex.bind(GL_TEXTURE1);

			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			mProgram->use();
			mProgram->setFloat("uPlaneScale", planeScale);
			mProgram->setFloat("uRulerScale", rulerScale);

			mPlane->draw();

			glDisable(GL_BLEND);

			gl::glCheckError();
		}

	private:
		gl::Mesh*			mPlane;
		gl::Texture2D 		mRulerTex;
		gl::Program*		mProgram;

		gl::Buffer			mUniformBlock;
	};

	class BBoxRenderer
	{
	public:
		BBoxRenderer()
		{
			mProgram = gl::Program::createProgramSPIRV(
				BBOX_VERT, sizeof(BBOX_VERT), 
				BBOX_FRAG, sizeof(BBOX_FRAG));

			mCubeVBO.create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
			mCubeVBO.load(0, 8 * 3 * sizeof(float));
			mCubeVAO.create();
			mCubeVAO.bindVertexBuffer(&mCubeVBO, 0, 3, GL_FLOAT, 0, 0, 0);

			mUniformBlock.create(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
		}

		~BBoxRenderer()
		{
			mCubeVBO.release();
			mCubeVAO.release();

			mProgram->release();
			delete mProgram;

			mUniformBlock.release();
		}

		void draw(const RenderParams& rparams, Vec3f p0, Vec3f p1, int type)
		{
			float vertices[]{
				p0[0], p0[1], p0[2],
				p0[0], p0[1], p1[2],
				p1[0], p0[1], p1[2],
				p1[0], p0[1], p0[2],

				p0[0], p1[1], p0[2],
				p0[0], p1[1], p1[2],
				p1[0], p1[1], p1[2],
				p1[0], p1[1], p0[2],
			};

			mUniformBlock.load((void*)&rparams, sizeof(RenderParams));
			mUniformBlock.bindBufferBase(0);

			mCubeVBO.load(vertices, sizeof(vertices));
			mCubeVAO.bind();
			mProgram->use();
			mProgram->setVec4("uColor", Vec4f(0.75));

			if (true)
			{
				const unsigned int indices[]
				{
					0, 1, 1, 2, 2, 3, 3, 0,
					4, 5, 5, 6, 6, 7, 7, 4,
					0, 4, 1, 5, 2, 6, 3, 7,
				};

				glEnable(GL_DEPTH_TEST);
				glEnable(GL_BLEND);
				glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, indices);
				glDisable(GL_BLEND);
			}
			else
			{
				const unsigned int indices[]
				{
					0, 1, 2, 2, 3, 0, // bottom
					7, 6, 5, 5, 4, 7, // top
					4, 5, 1, 1, 0, 4, // left
					3, 2, 6, 6, 7, 3, // right
					0, 3, 7, 7, 4, 0, // front
					5, 6, 2, 2, 1, 5, // back
				};
				glEnable(GL_CULL_FACE);
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, indices);
				glDisable(GL_CULL_FACE);
			}

			mCubeVAO.unbind();
			gl::glCheckError();
		}

	private:
		gl::VertexArray	mCubeVAO;
		gl::Buffer		mCubeVBO;
		gl::Program*	mProgram;

		gl::Buffer		mUniformBlock;
	};

	class BackgroundRenderer
	{
	public:
		BackgroundRenderer()
		{
			// create a quad object
			mScreenQuad = gl::Mesh::ScreenQuad();
			mBackgroundProgram = gl::Program::createProgramSPIRV(
				SCREEN_VERT, sizeof(SCREEN_VERT),
				BACKGROUND_FRAG, sizeof(BACKGROUND_FRAG));
		}

		~BackgroundRenderer() {
			mScreenQuad->release();
			delete mScreenQuad;
			mBackgroundProgram->release();
			delete mBackgroundProgram;
		}

		void draw(Vec3f color0, Vec3f color1)
		{
			// render background
			mBackgroundProgram->use();
			mBackgroundProgram->setVec3("uColor0", color0);
			mBackgroundProgram->setVec3("uColor1", color1);
			mScreenQuad->draw();

			glClear(GL_DEPTH_BUFFER_BIT);
		}

	private:
		// background
		gl::Program*	mBackgroundProgram;
		gl::Mesh*		mScreenQuad;
	};



	GLRenderHelper::GLRenderHelper()
	{
		mBBoxRenderer = new BBoxRenderer();
		mGroundRenderer = new GroundRenderer();
		mBackgroundRenderer = new BackgroundRenderer();
	}

	GLRenderHelper::~GLRenderHelper()
	{
		if (mBBoxRenderer) delete mBBoxRenderer;
		if (mGroundRenderer) delete mGroundRenderer;
		if (mBackgroundRenderer) delete mBackgroundRenderer;
	}

	void GLRenderHelper::drawGround(const RenderParams& rparams, float planeScale, float rulerScale)
	{
		if (mGroundRenderer != NULL)
			mGroundRenderer->draw(rparams, planeScale, rulerScale);
	}

	void GLRenderHelper::drawBBox(const RenderParams& rparams, Vec3f p0, Vec3f p1, int type)
	{
		if (mBBoxRenderer != NULL)
			mBBoxRenderer->draw(rparams, p0, p1, type);
	}

	void GLRenderHelper::drawBackground(Vec3f color0, Vec3f color1)
	{
		if (mBackgroundRenderer != NULL)
			mBackgroundRenderer->draw(color0, color1);
	}
}
