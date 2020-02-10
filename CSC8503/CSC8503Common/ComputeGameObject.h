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

			virtual void OnSetup();
			virtual void OnDraw();
			virtual void SendUniforms(OGLShader* s);

		protected:
			OGLComputeShader* colourShader;
			OGLComputeShader* positionShader;
			GLuint colourSSBO;

			GLuint posSSBO; // will be used as the persistent buffer in this case
			Vector3* posPtr;

			const int noOfColours = 2;
			int colourCount = 0;
		};
	}
}

