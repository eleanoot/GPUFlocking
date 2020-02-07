#pragma once
#include "GameObject.h"
#include "../../Plugins/OpenGLRendering/OGLComputeShader.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"

namespace NCL {
	namespace CSC8503 {
		class ComputeGameObject :
			public GameObject
		{
		public:
			ComputeGameObject(string name = "");
			~ComputeGameObject();

			virtual void OnDraw();
			virtual void SendUniforms(OGLShader* s);

		protected:
			OGLComputeShader* colourShader;
			GLuint colourSSBO;
			const int noOfColours = 2;

			int colourCount = 0;
		};
	}
}

