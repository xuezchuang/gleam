#include "oit.h"
#include <base/window.h>
#include <base/context.h>
#include <base/resource_loader.h>
#include <render/mesh.h>
#include <render/query.h>
#include <render/frame_buffer.h>
#include <render/render_effect.h>
#include <render/render_engine.h>
#include <render/post_process.h>
#include <scene/scene_object.h>
#include <render/view_port.h>
#include <input/input_engine.h>
#include <input/input_record.h>

#include <boost/lexical_cast.hpp>
#include <GLFW/glfw3.h>

#include "glm/gtc/matrix_transform.hpp"

#include <render/ogl_util.h>

float ALPHA_VALUE = 0.5f;

#define TESTSIMPLE
//#define USE11OBJ
#ifdef 	TESTSIMPLE
static bool btestSimple = true;
#else
static bool btestSimple = false;
#endif


//#define ORDINARY
#ifdef ORDINARY
static bool bOrdinary = true;
#else
static bool bOrdinary = false;
#endif

#ifdef 	USE11OBJ
static bool buse11OBJ = true;
#else
static bool buse11OBJ = false;
#endif

enum OITStatus
{
	Ordinary,
	PeelingInit,
	PeelingPeel,
	//PeelingBlend,
	//PeelingFinal,

	WeightedBlendedBlend,
};

class RenderPolygon : public Mesh
{
public:
	RenderPolygon(const std::string& name, const RenderModelPtr& model) : Mesh(name, model)
	{
		oit_effect_ = LoadRenderEffect("oit.xml");

		if (peeling_init_tech_ == nullptr)
		{
			peeling_init_tech_ = oit_effect_->GetTechniqueByName("PeelingInitTech");
			peeling_peel_tech_ = oit_effect_->GetTechniqueByName("PeelingPeelTech");
			weighted_blended_blend_tech_ = oit_effect_->GetTechniqueByName("WeightedBlendedBlendTech");
		}

		effect_attrib_ = EA_Transparency;
	}

	void OnRenderBegin() override
	{
		RenderEngine &re = Context::Instance().RenderEngineInstance();
		Framework3D &app = Context::Instance().FrameworkInstance();

		Camera &camera = app.ActiveCamera();
		const ShaderObjectPtr &shader = technique_->GetShaderObject(*effect_);
		*(shader->GetUniformByName("mvp")) = camera.ProjViewMatrix() * ModelMatrix();
		glm::mat4 normal_matrix(glm::mat3(camera.ViewMatrix()));
		*(shader->GetUniformByName("normal_matrix")) = normal_matrix;
		*(shader->GetUniformByName("alpha")) = ALPHA_VALUE;

		switch (status_)
		{
		case PeelingPeel:
			*(shader->GetSamplerByName("depth_tex")) = depth_tex_;
			break;
		case WeightedBlendedBlend:
			*(shader->GetUniformByName("depth_scale")) = 0.5f;
			break;
		default:
			break;
		}
	}

	void SetOitStatus(OITStatus status)
	{
		status_ = status;
		RenderEffectPtr effect = oit_effect_;
		RenderTechnique *tech = nullptr;
		switch (status)
		{
		case PeelingInit:
			tech = peeling_init_tech_;
			break;
		case PeelingPeel:
			tech = peeling_peel_tech_;
			break;
		case WeightedBlendedBlend:
			tech = weighted_blended_blend_tech_;
			break;
		default:
			CHECK_INFO(false, "OIT status error.")
				break;
		}

		BindRenderTechnique(effect, tech);
	}

	void SetDepthTex(const TexturePtr &depth_tex)
	{
		depth_tex_ = depth_tex;
	}

private:
	RenderEffectPtr oit_effect_;

	RenderTechnique *peeling_init_tech_ = nullptr;
	RenderTechnique *peeling_peel_tech_ = nullptr;
	RenderTechnique *weighted_blended_blend_tech_ = nullptr;

	OITStatus status_;
	TexturePtr depth_tex_;
};

class RenderTriangle : public RenderableHelper
{
public:
	RenderTriangle()
	{
		RenderEngine& re = Context::Instance().RenderEngineInstance();

		oit_effect_ = LoadRenderEffect("oit_simple.xml");

		if(peeling_init_tech_ == nullptr)
		{
			ordinary = oit_effect_->GetTechniqueByName("Ordinary");
			peeling_init_tech_ = oit_effect_->GetTechniqueByName("PeelingInitTech");
			peeling_peel_tech_ = oit_effect_->GetTechniqueByName("PeelingPeelTech");
			weighted_blended_blend_tech_ = oit_effect_->GetTechniqueByName("WeightedBlendedBlendTech");
		}

		glm::vec3 xyzs[] =
		{
			glm::vec3(0.8f,0.8f,0.0f),
			glm::vec3(0.8f,-0.8f,0.0f),
			glm::vec3(-0.8f,-0.8f,0.0f),
			glm::vec3(-0.8f,0.8f,0.0f),
		};
		glm::vec2 xyzsNormal[] =
		{
			glm::vec2(1.0f,1.0f),
			glm::vec2(1.0f,0.0f),
			glm::vec2(0.0f,0.0f),
			glm::vec2(0.0f,1.0f),

			//glm::vec3(0.0f,0.0f,1.0f),
			//glm::vec3(0.0f,0.0f,1.0f),
			//glm::vec3(0.0f,0.0f,1.0f),
			//glm::vec3(0.0f,0.0f,1.0f),
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
		GraphicsBufferPtr Texture_GB = re.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzsNormal), xyzsNormal);
		layout_->BindVertexStream(Texture_GB, VertexElement(VEU_TextureCoord, 0, EF_BGR32F));
		
		layout_->BindIndexStream(pos_ib, EF_R32UI);
		layout_->TopologyType(TT_TriangleList);

		effect_attrib_ = EA_Transparency;

	}
	void OnRenderBegin() override
	{
		Framework3D &app = Context::Instance().FrameworkInstance();
		Camera& camera = app.ActiveCamera();
		const ShaderObjectPtr& shader = technique_->GetShaderObject(*effect_);
		*(shader->GetUniformByName("mvp")) = camera.ProjViewMatrix()* ModelMatrix();
		*(shader->GetUniformByName("color")) = color;
		switch(status_)
		{
			//case Ordinary:
			//case PeelingInit:
			//	*(shader->GetUniformByName("color")) = color;
			//	break;
			case PeelingPeel:
				*(shader->GetSamplerByName("depth_tex")) = depth_tex_;
				break;
			case WeightedBlendedBlend:
				*(shader->GetUniformByName("depth_scale")) = 0.5f;
				break;
			default:
				break;
		}

	}
	void SetOitStatus(OITStatus status)
	{
		status_ = status;
		RenderEffectPtr effect = oit_effect_;
		RenderTechnique* tech = nullptr;
		switch(status)
		{
			case Ordinary:
				tech = ordinary;
				break;
			case PeelingInit:
				tech = peeling_init_tech_;
				break;
			case PeelingPeel:
				tech = peeling_peel_tech_;
				break;
			case WeightedBlendedBlend:
				tech = weighted_blended_blend_tech_;
				break;
			default:
				CHECK_INFO(false, "OIT status error.")
					break;
		}

		BindRenderTechnique(effect, tech);
	}

	void SetDepthTex(const TexturePtr& depth_tex)
	{
		depth_tex_ = depth_tex;
	}
	void SetColor(glm::vec4 _color)
	{
		color = _color;
	}
public:
	glm::vec4 color;
	RenderEffectPtr oit_effect_;

	RenderTechnique* peeling_init_tech_ = nullptr;
	RenderTechnique* peeling_peel_tech_ = nullptr;
	RenderTechnique* weighted_blended_blend_tech_ = nullptr;
	RenderTechnique* ordinary = nullptr;

	OITStatus status_;
	TexturePtr depth_tex_;
};

OIT::OIT(): Framework3D("OIT Sample.")
{
	ResLoader::Instance().AddPath("../../samples/oit");
	ResLoader::Instance().AddPath("../../resource/common");
}

void OIT::OnCreate()
{
	RenderEngine &re = Context::Instance().RenderEngineInstance();
	re.BindFrameBuffer(FrameBufferPtr());

	if(btestSimple)
	{
		polygon_[0] = std::make_shared<SceneObjectHelper>(std::make_shared<RenderTriangle>(), SOA_Cullable);
		polygon_[1] = std::make_shared<SceneObjectHelper>(std::make_shared<RenderTriangle>(), SOA_Cullable);
		checked_pointer_cast<RenderTriangle>(polygon_[0]->GetRenderable())->SetColor(glm::vec4(1.0, 0.0, 0.0, 0.6));
		checked_pointer_cast<RenderTriangle>(polygon_[1]->GetRenderable())->SetColor(glm::vec4(0.0, 1.0, 0.0, 0.6));

		polygon_[0]->ModelMatrix(glm::mat4());
		polygon_[0]->AddToSceneManager();
		
		glm::mat4 trans = glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));

		polygon_[1]->ModelMatrix(trans);
		polygon_[1]->AddToSceneManager();

		this->LookAt(glm::vec3(2, 0, -2), glm::vec3(0, 0, 0));
		this->Proj(0.1f, 100);
		controller_.AttachCamera(this->ActiveCamera());
		//controller_.SetScalers(0.005f, 0.01f);
	}
	else
	{
		if(buse11OBJ)
		{
			dragon_ = LoadModel("11.obj", EAH_Immutable, CreateModelFunc<RenderModel>(), CreateMeshFunc<RenderPolygon>());
			this->LookAt(glm::vec3(2, 0, -2), glm::vec3(0, 0, 0));
			this->Proj(0.1f, 100.0f);
			controller_.AttachCamera(this->ActiveCamera());
			//controller_.SetScalers(0.005f, 0.01f);
		}
		else
		{
			 dragon_ = LoadModel("dragon.obj", EAH_Immutable, CreateModelFunc<RenderModel>(), CreateMeshFunc<RenderPolygon>());
			 this->LookAt(glm::vec3(0, 0.125f, -0.25f), glm::vec3(0, 0.125f, 0));
			 this->Proj(0.1f, 100.0f);
			 controller_.AttachCamera(this->ActiveCamera());
			 controller_.SetScalers(0.005f, 0.01f);
		}
		SceneObjectHelperPtr dragon_so = std::make_shared<SceneObjectHelper>(dragon_, SOA_Cullable);
		dragon_so->AddToSceneManager();
	}

	InitDepthPeeling();
	//InitWeightedBlended();

	this->RegisterAfterFrameFunc([this](float app_time, float frame_time) ->int 
	{
		RenderEngine &re = Context::Instance().RenderEngineInstance();
		static float accum_time = 0;
		accum_time += frame_time;
		std::string name = "OIT Sample. FPS : " + boost::lexical_cast<std::string>(this->FPS());
		if (accum_time > 2.0f)
		{
			switch (this->mod_)
			{
			case DepthPeeling:
				name += std::string(" Num Layers:") + boost::lexical_cast<std::string>(this->num_layers_);
				break;
			case WeightedBlended:
			default:
				break;
			}
			re.SetRenderWindowTitle(name);
			accum_time = 0;
		}
		return 0;
	});

	InputEngine &ie = Context::Instance().InputEngineInstance();
	ie.Register([this]() 
	{
		static Timer timer;
		RenderEngine &re = Context::Instance().RenderEngineInstance();
		WindowPtr window = re.GetWindow();
		InputRecord &record = window->GetInputRecord();
		float elapsed = timer.Elapsed();

		if (elapsed < 0.1f) return;

		if (record.keys[GLFW_KEY_SPACE])
		{
			ALPHA_VALUE += 0.1f;
			if (ALPHA_VALUE > 1.01f)
				ALPHA_VALUE = 0.1f;
		}

		if (record.keys[GLFW_KEY_E])
		{
			switch (mod_)
			{
			case OIT::DepthPeeling:
				mod_ = WeightedBlended;
				break;
			case OIT::WeightedBlended:
				mod_ = DepthPeeling;
				break;
			default:
				CHECK_INFO(false, "Wrong path!");
				break;
			}
		}
		timer.Restart();
	});
}

uint32_t OIT::DoUpdate(uint32_t render_index)
{
	if (mod_ == DepthPeeling)
		return DoUpdateDepthPeeling(render_index);
	else if (mod_ == WeightedBlended)
		return DoUpdateWeightedBlended(render_index);
	
	CHECK_INFO(false, "Wrong path!");
	return UR_Finished;
}

void OIT::InitDepthPeeling()
{
	RenderEngine &re = Context::Instance().RenderEngineInstance();
	const WindowPtr &window = re.GetWindow();
	int32_t width = window->Width(), height = window->Height();

	// Depth peeling init
	for (int i = 0; i < 2; ++i)
	{
		front_fbo_[i] = re.MakeFrameBuffer();

		front_depth_tex_[i] = re.MakeTexture2D(width, height, 0, EF_D32F, 1, EAH_GPU_Read | EAH_GPU_Write);
		front_color_tex_[i] = re.MakeTexture2D(width, height, 0, EF_ABGR8, 1, EAH_GPU_Read | EAH_GPU_Write);

		RenderViewPtr front_depth_rv = re.Make2DDepthStencilRenderView(*(front_depth_tex_[i]), 0);
		RenderViewPtr front_color_rv = re.Make2DRenderView(*(front_color_tex_[i]), 0);
		front_fbo_[i]->Attach(ATT_DepthStencil, front_depth_rv);
		front_fbo_[i]->Attach(ATT_Color0, front_color_rv);

		front_fbo_[i]->GetViewport()->camera = re.DefaultFrameBuffer()->GetViewport()->camera;

		queries_[i] = re.MakeConditionalRender();
	}

	front_blender_fbo_ = re.MakeFrameBuffer();
	front_color_blender_ = re.MakeTexture2D(width, height, 0, EF_ABGR32F, 1, EAH_GPU_Read | EAH_GPU_Write);
	RenderViewPtr front_depth_rv = re.Make2DDepthStencilRenderView(*(front_depth_tex_[1]), 0);
	RenderViewPtr front_color_blender_rv = re.Make2DRenderView(*front_color_blender_, 0);
	front_blender_fbo_->Attach(ATT_DepthStencil, front_depth_rv);
	front_blender_fbo_->Attach(ATT_Color0, front_color_blender_rv);
	front_blender_fbo_->GetViewport()->camera = re.DefaultFrameBuffer()->GetViewport()->camera;

	if(btestSimple)
	{
		peel_blend_pp_ = LoadPostProcess("oit_pp_simple.xml", "FrontPeelingBlendPP");
		peel_final_pp_ = LoadPostProcess("oit_pp_simple.xml", "PeelingFinalPP");
	}
	else
	{
		peel_blend_pp_ = LoadPostProcess("oit_pp.xml", "FrontPeelingBlendPP");
		peel_final_pp_ = LoadPostProcess("oit_pp.xml", "PeelingFinalPP");
	}

}

void OIT::InitWeightedBlended()
{
	RenderEngine &re = Context::Instance().RenderEngineInstance();
	const WindowPtr &window = re.GetWindow();
	int32_t width = window->Width(), height = window->Height();

	accum_tex_[0] = re.MakeTexture2D(width, height, 0, EF_ABGR16F, 1, EAH_GPU_Read | EAH_GPU_Write);
	accum_tex_[1] = re.MakeTexture2D(width, height, 0, EF_R8, 1, EAH_CPU_Read | EAH_GPU_Write);

	RenderViewPtr view0 = re.Make2DRenderView(*accum_tex_[0], 0);
	RenderViewPtr view1 = re.Make2DRenderView(*accum_tex_[1], 0);

	accum_fbo_ = re.MakeFrameBuffer();
	accum_fbo_->Attach(ATT_Color0, view0);
	accum_fbo_->Attach(ATT_Color1, view1);
	accum_fbo_->GetViewport()->camera = re.DefaultFrameBuffer()->GetViewport()->camera;

	if(btestSimple)
	{
		weighted_blended_final_pp_ = LoadPostProcess("oit_pp_simple.xml", "WeightedBlendedFinalPP");
	}
	else
	{
		weighted_blended_final_pp_ = LoadPostProcess("oit_pp.xml", "WeightedBlendedFinalPP");
	}
	
}

uint32_t OIT::DoUpdateDepthPeeling(uint32_t render_index)
{
	RenderEngine &re = Context::Instance().RenderEngineInstance();
	Color clear_color(0.2f, 0.4f, 0.6f, 1.0f);
	Color black_color(0, 0, 0, 0);

	if (render_index == 0)
	{
		if(bOrdinary)
		{
			re.CurrentFrameBuffer()->Clear(CBM_Color | CBM_Depth, clear_color, 1.0f, 0);
			checked_pointer_cast<RenderTriangle>(polygon_[0]->GetRenderable())->SetOitStatus(OITStatus::Ordinary);
			checked_pointer_cast<RenderTriangle>(polygon_[1]->GetRenderable())->SetOitStatus(OITStatus::Ordinary);
			return UR_NeedFlush | UR_Finished;
		}

		//{
		//	re.DefaultFrameBuffer()->Clear(CBM_Color | CBM_Depth, clear_color, 1.0f, 0);
		//	front_blender_fbo_->Clear(CBM_Color | CBM_Depth, black_color, 1.0f, 0);
		//	//for (int32_t i = 0; i < 2; ++i)
		//	//{
		//	//	front_fbo_[i]->Clear(CBM_Color | CBM_Depth, black_color, 1.0f, 0);
		//	//}
		//	//checked_pointer_cast<RenderTriangle>(polygon_[0]->GetRenderable())->SetOitStatus(OITStatus::Ordinary);
		//	//checked_pointer_cast<RenderTriangle>(polygon_[1]->GetRenderable())->SetOitStatus(OITStatus::Ordinary);
		//	checked_pointer_cast<RenderTriangle>(polygon_[0]->GetRenderable())->SetOitStatus(OITStatus::PeelingInit);
		//	checked_pointer_cast<RenderTriangle>(polygon_[1]->GetRenderable())->SetOitStatus(OITStatus::PeelingInit);
		//	re.BindFrameBuffer(front_blender_fbo_);
		//	return UR_NeedFlush;
		//}

		num_layers_ = 0;
		re.DefaultFrameBuffer()->Clear(CBM_Color | CBM_Depth, black_color, 1.0f, 0);
		//front_blender_fbo_->Clear(CBM_Color | CBM_Depth, Color(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0);
		front_blender_fbo_->Clear(CBM_Color | CBM_Depth, black_color, 1.0f, 0);
		for (int32_t i = 0; i < 2; ++i)
		{
			front_fbo_[i]->Clear(CBM_Color | CBM_Depth, black_color, 1.0f, 0);
		}
		re.BindFrameBuffer(front_blender_fbo_);
		return UR_OpaqueOnly;
	}
	else if (render_index == 1)
	{
		re.BindFrameBuffer(front_blender_fbo_);
		if(btestSimple)
		{
			checked_pointer_cast<RenderTriangle>(polygon_[0]->GetRenderable())->SetOitStatus(OITStatus::PeelingInit);
			checked_pointer_cast<RenderTriangle>(polygon_[1]->GetRenderable())->SetOitStatus(OITStatus::PeelingInit);
		}
		else
		{
			dragon_->ForEachMeshes([](const MeshPtr& mesh)
			{
				checked_pointer_cast<RenderPolygon>(mesh)->SetOitStatus(OITStatus::PeelingInit);
			});
		}

		return UR_TransparencyOnly | UR_NeedFlush;
	}
	else
	{
		uint32_t currId = (render_index - 2) / 2 % 2;
		uint32_t prevId = 1 - currId;
		const bool peel_pass = (render_index - 2) % 2 == 0;

		if (peel_pass)
		{
			bool finished = false;
			if (render_index > 3)
			{
				if (queries_[prevId]->AnySamplesPassed())
					++num_layers_;
				else
					finished = true;
			}
			if (finished)
			{
				peel_final_pp_->InputTexture(0, front_color_blender_);
				peel_final_pp_->OutputTexture(0, TexturePtr());
				peel_final_pp_->SetParam("bgColor", glm::vec3(0.2f, 0.4f, 0.6f));
				peel_final_pp_->Render();
				return UR_Finished;
			}

			re.BindFrameBuffer(front_fbo_[currId]);
			re.CurrentFrameBuffer()->Clear(CBM_Color | CBM_Depth, black_color,1.0f, 0);
			queries_[currId]->Begin();

			if(btestSimple)
			{
				checked_pointer_cast<RenderTriangle>(polygon_[0]->GetRenderable())->SetOitStatus(OITStatus::PeelingPeel);
				checked_pointer_cast<RenderTriangle>(polygon_[0]->GetRenderable())->SetDepthTex(front_depth_tex_[prevId]);
				checked_pointer_cast<RenderTriangle>(polygon_[1]->GetRenderable())->SetOitStatus(OITStatus::PeelingPeel);
				checked_pointer_cast<RenderTriangle>(polygon_[1]->GetRenderable())->SetDepthTex(front_depth_tex_[prevId]);
			}
			else
			{
				dragon_->ForEachMeshes([this, prevId](const MeshPtr& mesh) 
				{
					auto polygon = checked_pointer_cast<RenderPolygon>(mesh);
					polygon->SetOitStatus(OITStatus::PeelingPeel);
					polygon->SetDepthTex(front_depth_tex_[prevId]);
				});
			}

			return UR_TransparencyOnly | UR_NeedFlush;
		}
		else // blend pass
		{
			queries_[currId]->End();
			peel_blend_pp_->InputTexture(0, front_color_tex_[currId]);
			peel_blend_pp_->OutputTexture(0, front_color_blender_);
			peel_blend_pp_->Render();

			return 0;
		}
	}
}

uint32_t OIT::DoUpdateWeightedBlended(uint32_t render_index)
{
	RenderEngine &re = Context::Instance().RenderEngineInstance();

	switch (render_index)
	{
	case 0:
	{
		re.DefaultFrameBuffer()->Clear(CBM_Color | CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1.0f), 1.0f, 0);
		accum_fbo_->ClearColor(ATT_Color0, Color(0, 0, 0, 0));
		accum_fbo_->ClearColor(ATT_Color1, Color(1.0f, 1.0f, 1.0f, 1.0f));
		return UR_OpaqueOnly /*| UR_NeedFlush*/;
	}
	case 1:
	{
		re.BindFrameBuffer(accum_fbo_);
		if(btestSimple)
		{
			checked_pointer_cast<RenderTriangle>(polygon_[0]->GetRenderable())->SetOitStatus(OITStatus::WeightedBlendedBlend);
			checked_pointer_cast<RenderTriangle>(polygon_[1]->GetRenderable())->SetOitStatus(OITStatus::WeightedBlendedBlend);
		}
		else
		{
			dragon_->ForEachMeshes([](const MeshPtr& mesh)
			{
				checked_pointer_cast<RenderPolygon>(mesh)->SetOitStatus(OITStatus::WeightedBlendedBlend);
			});
		}
		return UR_TransparencyOnly | UR_NeedFlush;
	}
	case 2:
	{
		re.BindFrameBuffer(FrameBufferPtr());
		weighted_blended_final_pp_->InputTexture(0, accum_tex_[0]);
		weighted_blended_final_pp_->InputTexture(1, accum_tex_[1]);
		weighted_blended_final_pp_->SetParam("bgColor", glm::vec3(0.2f, 0.4f, 0.6f));
		weighted_blended_final_pp_->Render();
		return UR_Finished;
	}

	default:
	{
		CHECK_INFO(false, "Wrong path!");
		return UR_Finished;
	}
	}
}

int main()
{
	OIT app;
	app.Create();
	app.Run();
}
