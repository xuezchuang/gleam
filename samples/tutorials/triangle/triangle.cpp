#include "triangle.h"
#include <base/context.h>
#include <render/render_engine.h>
#include <glm/gtx/transform.hpp>
#include <render/frame_buffer.h>
#include <boost/lexical_cast.hpp>
namespace gleam {

	class RenderTriangle : public RenderableHelper
	{
	public:
		RenderTriangle()
		{
			RenderEngine &re = Context::Instance().RenderEngineInstance();
			effect_ = LoadRenderEffect("triangle.xml");
			technique_ = effect_->GetTechniqueByName("Triangle");

			glm::vec3 xyzs[] =
			{
				//glm::vec3(0, 0.8f,0),
				//glm::vec3(0.7f,-0.8f,0),
				//glm::vec3(-0.7f,-0.8f,0),

				glm::vec3(0.8f,0.8f,0.0f),
				glm::vec3(0.8f,-0.8f,0.0f),
				glm::vec3(-0.8f,-0.8f,0.0f),
				glm::vec3(-0.8f,0.8f,0.0f),
			};
			int index_xyz[] = 
			{
				0,1,3,
				1,2,3
			};

			layout_ = re.MakeRenderLayout();

			GraphicsBufferPtr pos_vb = re.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzs), xyzs);
			GraphicsBufferPtr pos_ib = re.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(index_xyz), index_xyz);
			layout_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_BGR32F));
			layout_->BindIndexStream(pos_ib, EF_R32UI);
			layout_->TopologyType(TT_TriangleList);
		}
	};

	TriangleApp::TriangleApp(): Framework3D("Triangle")
	{
		ResLoader::Instance().AddPath("../../samples/tutorials/triangle");
	}
	void TriangleApp::OnCreate()
	{
		polygon_ = std::make_shared<SceneObjectHelper>(std::make_shared<RenderTriangle>(), SOA_Cullable);
		//polygon_ = std::make_unique<SceneObjectHelper>(std::make_unique<RenderTriangle>(), SOA_Cullable);
		
		polygon_->ModelMatrix(glm::mat4());
		polygon_->AddToSceneManager();

		//this->LookAt(glm::vec3(2, 0, -2), glm::vec3(0, 0, 0));
		//this->LookAt(glm::vec3(2, -2, -2), glm::vec3(0, 0, 0));
		//this->Proj(0.1f, 100);


		this->RegisterAfterFrameFunc([&](float app_time, float elapsed_time) -> int 
		{
			static float acc_time = 0;
			static int32_t frames = 0;
			++frames;
			acc_time += elapsed_time;

			if (acc_time > 1.0f)
			{
				RenderEngine& re = Context::Instance().RenderEngineInstance();
				re.SetRenderWindowTitle(std::string("Triangle. FPS : ") + boost::lexical_cast<std::string>(frames));
				frames = 0;
				acc_time = 0;
			}
			return 0;
		});
	}
	uint32_t TriangleApp::DoUpdate(uint32_t render_index)
	{
		RenderEngine &re = Context::Instance().RenderEngineInstance();

		Color clear_color(0.2f, 0.4f, 0.6f, 1);
		re.CurrentFrameBuffer()->Clear(CBM_Color | CBM_Depth,clear_color, 1.0f, 0);
		return UR_NeedFlush | UR_Finished;
	}
}

int main()
{
	gleam::TriangleApp app;
	app.Create();
	app.Run();
}
