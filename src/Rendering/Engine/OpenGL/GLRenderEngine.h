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

#include <memory>
#include <vector>

#include <RenderEngine.h>

#include "gl/Buffer.h"
#include "gl/Texture.h"
#include "gl/Framebuffer.h"
#include "gl/Shader.h"
#include "gl/Mesh.h"


namespace dyno
{
	class SSAO;
	class FXAA;
	class ShadowMap;
	class GLRenderHelper;
	class GLVisualModule;
	class SceneGraph;

	class GLRenderEngine : public RenderEngine
	{
	public:
		GLRenderEngine();
		~GLRenderEngine();
			   
		virtual void initialize() override;
		virtual void terminate() override;

		virtual void draw(dyno::SceneGraph* scene, const RenderParams& rparams) override;

		virtual std::string name() const override;

		// get the selected nodes on given rect area
		Selection select(int x, int y, int w, int h) override;

	private:
		void createFramebuffer();
		void resizeFramebuffer(int w, int h);		
		void setupTransparencyPass();
		void updateRenderItems(dyno::SceneGraph* scene);

	private:

		// objects to render
		struct RenderItem {
			std::shared_ptr<Node>			node;
			std::shared_ptr<GLVisualModule> visualModule;

			bool operator==(const RenderItem& item) {
				return node == item.node && visualModule == item.visualModule;
			}
		};

		std::vector<RenderItem> mRenderItems;

	private:
		// internal framebuffer
		gl::Framebuffer	mFramebuffer;
		gl::Texture2D	mColorTex;
		gl::Texture2D	mDepthTex;
		gl::Texture2D	mIndexTex;			// indices for object/mesh/primitive etc.

		// for linked-list OIT
		const int		MAX_OIT_NODES = 1024 * 1024 * 8;
		gl::Buffer		mFreeNodeIdx;
		gl::Buffer		mLinkedListBuffer;
		gl::Texture2D	mHeadIndexTex;
		gl::Program*	mBlendProgram;

		GLRenderHelper* mRenderHelper;
		ShadowMap*		mShadowMap;
		// anti-aliasing
		bool			bEnableFXAA = true;
		FXAA*			mFXAAFilter;

		gl::Mesh* mScreenQuad = 0;
	};
};
