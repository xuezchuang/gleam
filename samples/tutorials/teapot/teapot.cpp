
#include <base/framework.h>
#include <base/context.h>
#include <render/frame_buffer.h>
#include <render/uniform.h>
#include <base/bbox.h>
#include <render/renderable.h>
#include <scene/scene_object.h>
#include <render/camera_controller.h>
#include <render/mesh.h>
#include <glm/gtc/matrix_transform.hpp>
#include <render/texture.h>
#include <boost/lexical_cast.hpp>
using namespace gleam;

class TexturedRenderPolygon : public Mesh
{
public:
	TexturedRenderPolygon(const std::string &name, const RenderModelPtr &model): Mesh(name, model)
	{
		RenderEffectPtr effect = LoadRenderEffect("renderable.xml");
		RenderTechnique *technique = effect->GetTechniqueByName("TexTec");
		this->BindRenderTechnique(effect, technique);
		ShaderObject &shader = *technique_->GetShaderObject(*effect_);
		mvp_ = shader.GetUniformByName("mvp");
		albedo_tex_ = shader.GetSamplerByName("albedo_tex");
	}

	void OnRenderBegin() override
	{
		Renderable::OnRenderBegin();
		Framework3D &framework = Context::Instance().FrameworkInstance();

		glm::mat4 mvp = framework.ActiveCamera().ProjViewMatrix() * model_matrix_;
		*mvp_ = mvp;
	}
};

class TeapotFramework : public Framework3D
{
public:
	TeapotFramework(): Framework3D("Line")
	{
		ResLoader::Instance().AddPath("../../resource/common/teapot");
	}
protected:
	void OnCreate() override
	{
		bool btest = false;
		btest = true;
		if (btest)
		{
			RenderModelPtr teapot_model = LoadModel("11.obj", EAH_GPU_Read, CreateModelFunc<RenderModel>(), CreateMeshFunc<TexturedRenderPolygon>());
			teapot_ = std::make_shared<SceneObjectHelper>(teapot_model, SOA_Cullable);
			glm::mat4 model = glm::scale(glm::mat4(), glm::vec3(1.0));
			teapot_->ModelMatrix(model);
			teapot_->AddToSceneManager();

			this->LookAt(glm::vec3(2, 0, -2), glm::vec3(0, 0, 0));
			this->Proj(0.1f, 100);
		}
		else
		{
			RenderModelPtr teapot_model = LoadModel("teapot.obj", EAH_GPU_Read, CreateModelFunc<RenderModel>(), CreateMeshFunc<TexturedRenderPolygon>());
			teapot_ = std::make_shared<SceneObjectHelper>(teapot_model, SOA_Cullable);
			glm::mat4 model = glm::scale(glm::mat4(), glm::vec3(0.02f));
			model = glm::translate(model, glm::vec3(0, -50.0f, 0));
			teapot_->ModelMatrix(model);
			teapot_->AddToSceneManager();

			this->LookAt(glm::vec3(2, 0, -2), glm::vec3(0, 0, 0));
			this->Proj(0.1f, 100);
		}


		controller.AttachCamera(this->ActiveCamera());
		//controller.SetScalers(0.01f, 0.05f);

		this->RegisterAfterFrameFunc([&](float app_time, float elapsed_time) -> int {
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

private:
	uint32_t DoUpdate(uint32_t render_index) override
	{
		RenderEngine &re = Context::Instance().RenderEngineInstance();

		Color clear_color(0.2f, 0.4f, 0.6f, 1.0f);

		re.CurrentFrameBuffer()->Clear(CBM_Color | CBM_Depth, clear_color, 1.0f, 0);
		return UR_NeedFlush | UR_Finished;
	}

	SceneObjectHelperPtr teapot_;

	TrackballCameraController controller;
	//FirstPersonCameraController controller;
};

void main()
{
	TeapotFramework app;
	app.Create();
	app.Run();
}
