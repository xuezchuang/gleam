/************************************************************************ 
 * @description :  
 * @author		:  $username$
 * @creat 		:  $time$
************************************************************************ 
 * Copyright @ OscarShen 2017. All rights reserved. 
************************************************************************/  
#pragma once
#ifndef GLEAM_BASE_FRAMEWORK_H_
#define GLEAM_BASE_FRAMEWORK_H_
#include <gleam.h>
#include <util/timer.h>
#include <boost/noncopyable.hpp>
#include <render/render_engine.h>
namespace gleam
{
	enum UpdateResult
	{
		UR_NeedFlush = 1UL << 0,
		UR_Finished = 1UL << 1,
		UR_OpaqueOnly = 1UL << 2,
		UR_TransparencyOnly = 1UL << 3,
	};

	class Framework3D : boost::noncopyable
	{
		friend class SceneManager;

	public:
		explicit Framework3D(const std::string &name);
		virtual ~Framework3D();

		float AppTime() const { return app_time_; }
		float FrameTime() const { return frame_time_; }
		float FPS() const { return fps_; }

		void Run();

		const Camera &ActiveCamera() const;
		Camera &ActiveCamera();

		void Create();
		void Destroy();

		uint32_t RegisterAfterFrameFunc(std::function<int(float, float)> &&func);
		void	 UnregisterAfterFrameFunc(uint32_t index);
		void	 RunAfterFrame();

	protected:
		uint32_t Update(uint32_t render_index);
		void LookAt(const glm::vec3 &eye, const glm::vec3 &lookat);
		void LookAt(const glm::vec3 &eye, const glm::vec3 &lookat, const glm::vec3 &up);
		void Proj(float nearPlane, float farPlane);

	private:
		virtual void OnCreate() { }
		virtual void OnDestroy() { }
		virtual uint32_t DoUpdate(uint32_t render_index) = 0;

	protected:
		std::string name_;

		uint32_t total_num_frames_;
		float fps_;
		float accumulate_time_;
		uint32_t num_frames_;

		Timer timer_;
		float app_time_;
		float frame_time_;

		RenderSettings settings;
		std::vector<std::function<int(float, float)>> run_after_frame_;
	};
}

#endif // !GLEAM_BASE_FRAMEWORK_H_
