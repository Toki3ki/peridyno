/**
 * Copyright 2017-2022 Jian SHI
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

#include "GLSurfaceVisualModule.h"

namespace dyno
{
	class GLInstanceVisualModule : public GLSurfaceVisualModule
	{
		DECLARE_CLASS(GLSurfaceVisualModule)
	public:
		GLInstanceVisualModule();
		~GLInstanceVisualModule();

		virtual std::string caption() override;

	public:
		// for instanced rendering
		DEF_ARRAY_IN(Transform3f, InstanceTransform, DeviceType::GPU, "");
		DEF_ARRAY_IN(Vec3f, InstanceColor, DeviceType::GPU, "");

	protected:
		virtual void updateImpl() override;

		virtual bool initializeGL() override;
		virtual void releaseGL() override;
		virtual void updateGL() override;

	private:

		gl::XBuffer<Transform3f> mInstanceTransforms;
		gl::XBuffer<Vec3f>		 mInstanceColors;

	};
};