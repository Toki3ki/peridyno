/**
 * Copyright 2017-2021 Jian SHI
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "gl/Buffer.h"
#include "gl/Framebuffer.h"
#include "gl/Texture.h"
#include "gl/Shader.h"
#include "gl/Mesh.h"

#include <vector>
#include <RenderEngine.h>

namespace dyno
{
	class Camera;
	class SceneGraph;

	class ShadowMap
	{
	public:
		ShadowMap(int w = 1024, int h = 1024);
		~ShadowMap();

		void update(dyno::SceneGraph* scene, const dyno::RenderParams& rparams);

		// bind uniform block and texture
		void bind(int shadowUniformLoc = 3, int shadowTexSlot = 5);

	private:
		// framebuffers
		gl::Framebuffer		mFramebuffer;
		gl::Texture2D		mShadowTex;
		gl::Texture2D		mShadowDepth;
		gl::Texture2D		mShadowBlur;

		gl::Program*		mBlurProgram;
		gl::Mesh*			mQuad;


		gl::Buffer			mShadowUniform;	// uniform buffer for shadow lookup matrices

	public:
		int				width;
		int				height;

		// patch to color bleeding, min p_max
		float			minValue = 0.1f;
		// num of blur interations for VSM
		const int		blurIters = 1;
	};
}