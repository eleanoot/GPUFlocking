#pragma once
#include "../CSC8503Common/ComputeGameObject.h"
#include "../../Plugins/OpenGLRendering/OGLComputeShader.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
namespace NCL {
	namespace CSC8503 {
		class ComputeGoose :
			public ComputeGameObject
		{
		public:
			ComputeGoose(string name = "");
			~ComputeGoose();

			virtual void OnSetup();
			virtual void OnDraw();
			virtual void SendUniforms(OGLShader* s);

		protected:
			OGLComputeShader* colourShader;
			OGLComputeShader* positionShader;
			GLuint colourSSBO;

			GLuint posSSBO[2]; // will be used as the persistent buffer in this case
								// double buffering
			Vector3* posPtrBegin;
			Vector3* posPtrSecond;
			Vector3* posPtr;
			GLuint bufferIndex = 0;
			GLuint flags;

			const int noOfColours = 2;
			int colourCount = 0;
		};
	}

}
