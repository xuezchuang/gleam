#include "input_engine.h"
#include <base/window.h>
#include <render/camera_controller.h>
#include <base/context.h>
#include <input/input_record.h>
namespace gleam {
	void InputEngine::Update()
	{
		for (size_t i = 0; i < input_handlers_.size(); ++i) {
			input_handlers_[i]();
		}
		elapsed_time_ = timer_.Elapsed();
		timer_.Restart();
	}
	void InputEngine::Register(const std::function<void()>& action_handler)
	{
		input_handlers_.push_back(action_handler);
	}
}
