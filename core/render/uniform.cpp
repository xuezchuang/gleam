#include "uniform.h"
#include <glm/gtc/type_ptr.hpp>
#include "texture.h"
#include "render_state.h"
#include "graphics_buffer.h"
#include <base/context.h>
#include "render_engine.h"
#include "ogl_util.h"
namespace gleam {
	void OGLUniform::StoreUniformLocation(GLuint program)
	{
		program_ = program;
		location_ = glGetUniformLocation(program, name_.c_str());
		CHECK_INFO(location_ != -1, "Uniform varible " << name_ << " not found...");
	}
	Uniform & OGLUniformBool::operator=(const bool & value)
	{
		GLboolean v = static_cast<GLboolean>(value);
		if (v != data_)
		{
			dirty_ = true;
			data_ = v;
		}
		return *this;
	}
	void OGLUniformBool::Load()
	{
		if (dirty_) {
			glProgramUniform1i(program_, location_, static_cast<GLint>(data_));
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformBool::CopyResource() const
	{
		std::shared_ptr<OGLUniformBool> u = std::make_shared<OGLUniformBool>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		return u;
	}
	Uniform & OGLUniformFloat::operator=(const float & value)
	{
		if (value != data_)
		{
			dirty_ = true;
			data_ = value;
		}
		return *this;
	}
	Uniform & OGLUniformFloat::operator=(const uint32_t & value)
	{
		GLfloat v = static_cast<GLfloat>(value);
		if (v != data_)
		{
			dirty_ = true;
			data_ = v;
		}
		return *this;
	}
	Uniform & OGLUniformFloat::operator=(const int32_t & value)
	{
		GLfloat v = static_cast<GLfloat>(value);
		if (v != data_)
		{
			dirty_ = true;
			data_ = v;
		}
		return *this;
	}
	void OGLUniformFloat::Load()
	{
		if (dirty_) {
			glProgramUniform1f(program_, location_, data_);
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformFloat::CopyResource() const
	{
		std::shared_ptr<OGLUniformFloat> u = std::make_shared<OGLUniformFloat>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		return u;
	}
	Uniform & OGLUniformSampler::operator=(const SamplerStateObjectPtr & value)
	{
		if(data_.sampler_state != value)
		{
			dirty_ = true;
			data_.sampler_state = value;
		}
		return *this;
	}
	Uniform & OGLUniformSampler::operator=(const TexturePtr & value)
	{
		// should bind texture before every draw call,
		// it's always dirty.
		if (data_.texture != value)
		{
			data_.texture = value;
		}
		return *this;
	}
	Uniform & OGLUniformSampler::operator=(const uint32_t & value)
	{
		GLuint unit = static_cast<GLuint>(value);
		if (data_.texture_unit != unit)
		{
			dirty_ = true;
			data_.texture_unit = unit;
		}
		return *this;
	}
	Uniform & OGLUniformSampler::operator=(const int32_t & value)
	{
		GLuint unit = static_cast<GLuint>(value);
		if (data_.texture_unit != unit)
		{
			dirty_ = true;
			data_.texture_unit = unit;
		}
		return *this;
	}
	void OGLUniformSampler::Load()
	{
		GLuint gl_tex = 0;
		OGLSamplerStateObject &gl_sampler_state = *checked_pointer_cast<OGLSamplerStateObject>(data_.sampler_state);
		if (data_.texture)
		{
			OGLTexture &gl_texture = *checked_pointer_cast<OGLTexture>(data_.texture);
			gl_tex = gl_texture.GLTexture();
		}
		glBindTextureUnit(data_.texture_unit, gl_tex);
		glBindSampler(data_.texture_unit, gl_sampler_state.GLSampler());
		if (dirty_)
		{
			glProgramUniform1i(program_, location_, data_.texture_unit);
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformSampler::CopyResource() const
	{
		std::shared_ptr<OGLUniformSampler> u = std::make_shared<OGLUniformSampler>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		return u;
	}
	Uniform & OGLUniformVec2::operator=(const glm::vec2 & value)
	{
		if (value != data_)
		{
			dirty_ = true;
			data_ = value;
		}
		return *this;
	}
	void OGLUniformVec2::Load()
	{
		if (dirty_)
		{
			glProgramUniform2f(program_, location_, data_.x, data_.y);
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformVec2::CopyResource() const
	{
		std::shared_ptr<OGLUniformVec2> u = std::make_shared<OGLUniformVec2>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		return u;
	}
	Uniform & OGLUniformVec3::operator=(const glm::vec3 & value)
	{
		if (value != data_)
		{
			dirty_ = true;
			data_ = value;
		}
		return *this;
	}
	void OGLUniformVec3::Load()
	{
		if (dirty_)
		{
			glProgramUniform3f(program_, location_, data_.x, data_.y, data_.z);
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformVec3::CopyResource() const
	{
		std::shared_ptr<OGLUniformVec3> u = std::make_shared<OGLUniformVec3>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		return u;
	}
	Uniform & OGLUniformVec4::operator=(const glm::vec4 & value)
	{
		if (value != data_)
		{
			dirty_ = true;
			data_ = value;
		}
		return *this;
	}
	void OGLUniformVec4::Load()
	{
		if (dirty_)
		{
			glProgramUniform4f(program_, location_, data_.x, data_.y, data_.z, data_.w);
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformVec4::CopyResource() const
	{
		std::shared_ptr<OGLUniformVec4> u = std::make_shared<OGLUniformVec4>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		return u;
	}
	Uniform & OGLUniformMatrix4::operator=(const glm::mat4 & value)
	{
		if (value != data_)
		{
			dirty_ = true;
			data_ = value;
		}
		return *this;
	}
	void OGLUniformMatrix4::Load()
	{
		if (dirty_)
		{
			glProgramUniformMatrix4fv(program_, location_, 1, false, glm::value_ptr(data_));
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformMatrix4::CopyResource() const
	{
		std::shared_ptr<OGLUniformMatrix4> u = std::make_shared<OGLUniformMatrix4>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		return u;
	}
	void OGLUniformBuffer::StoreUniformBlockIndex(GLuint program)
	{
		index_ = glGetUniformBlockIndex(program, name_.c_str());
		program_ = program;
		CHECK_INFO(GL_INVALID_INDEX != index_, "Counldn't find uniform blocak : " << name_);
	}
	void OGLUniformBuffer::BindPoint(GLuint bind_point)
	{
		if (bind_point != bind_point_)
		{
			dirty_ = true;
			bind_point_ = bind_point;
		}
	}
	void OGLUniformBuffer::Load()
	{
		if (dirty_)
		{
			glUniformBlockBinding(program_, index_, bind_point_);
			glBindBufferBase(GL_UNIFORM_BUFFER, bind_point_,
				checked_pointer_cast<OGLGraphicsBuffer>(data_)->GLvbo());
			dirty_ = false;
		}
	}
	UniformBufferPtr OGLUniformBuffer::CopyResource() const
	{
		std::shared_ptr<OGLUniformBuffer> u = std::make_shared<OGLUniformBuffer>();
		u->name_ = this->name_;
		if (this->data_)
		{
			u->data_ = Context::Instance().RenderEngineInstance().MakeConstantBuffer(this->data_->Usage(), this->data_->AccessHint(), this->data_->Size(), nullptr);
			this->data_->CopyToBuffer(*u->data_);
		}
		u->dirty_ = true;
		return u;
	}
	void UniformTypeFromString(UniformType & type, const std::string & name, uint32_t array_size)
	{
		if (name == "bool")
			type = UT_Bool;
		else if (name == "float")
			type = UT_Float;
		else if (name == "vec2")
			type = UT_Vector2f;
		else if (name == "vec3")
			type = UT_Vector3f;
		else if (name == "vec4")
			type = UT_Vector4f;
		else if (name == "sampler" || name == "sampler1d" || name == "sampler2d" ||
			name == "sampler3d")
			type = UT_Sampler;
		else if (name == "mat4")
			type = UT_Matrix4f;
		else if (name == "image" || name == "image2d")
			type = UT_Image;
		else
			CHECK_INFO(false, "invalid uniform type : " << name);

		if (array_size > 0)
		{
			switch (type)
			{
			case UT_Float:
				type = UT_FloatArray;
				break;
			case UT_Vector2f:
				type = UT_Vector2fArray;
				break;
			case UT_Vector3f:
				type = UT_Vector3fArray;
				break;
			case UT_Vector4f:
				type = UT_Vector4fArray;
				break;
			default:
				CHECK_INFO(false, "invalid uniform type & array_size : " << name << ", " << array_size);
			}
		}
	}
	void OGLAttrib::StoreAttribLocation(GLuint program)
	{
		location_ = glGetAttribLocation(program, name_.c_str());
		program_ = program;
		CHECK_INFO(-1 != location_, "Counldn't find attrib : " << name_);
	}
	AttribPtr OGLAttrib::CopyResource() const
	{
		std::shared_ptr<OGLAttrib> u = std::make_shared<OGLAttrib>();
		u->name_ = this->name_;
		u->element_ = this->element_;
		return u;
	}
	UniformBuffer & UniformBuffer::operator=(const GraphicsBufferPtr & buffer)
	{
		if (buffer != data_)
		{
			dirty_ = true;
			data_ = buffer;
		}
		return *this;
	}
	Uniform & OGLUniformImage::operator=(const TexturePtr & value)
	{
		if (data_.texture != value)
		{
			data_.texture = value;
		}
		return *this;
	}
	Uniform & OGLUniformImage::operator=(const uint32_t & value)
	{
		GLuint unit = static_cast<GLuint>(value);
		if (data_.texture_unit != unit)
		{
			dirty_ = true;
			data_.texture_unit = unit;
		}
		return *this;
	}
	Uniform & OGLUniformImage::operator=(const int32_t & value)
	{
		GLuint unit = static_cast<GLuint>(value);
		if (data_.texture_unit != unit)
		{
			dirty_ = true;
			data_.texture_unit = unit;
		}
		return *this;
	}
	void OGLUniformImage::Load()
	{
		GLuint gl_tex = 0;
		if (data_.texture)
		{
			OGLTexture &gl_texture = *checked_pointer_cast<OGLTexture>(data_.texture);
			gl_tex = gl_texture.GLTexture();
		}
		glBindImageTextures(data_.texture_unit, 1, &gl_tex);
		if (dirty_)
		{
			glProgramUniform1i(program_, location_, data_.texture_unit);
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformImage::CopyResource() const
	{
		std::shared_ptr<OGLUniformImage> u = std::make_shared<OGLUniformImage>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		return u;
	}
	Uniform & OGLUniformFloatArray::operator=(const std::vector<float>& value)
	{
		WARNING(static_cast<GLsizei>(value.size()) == size_, "Array size is inconsistent..." << value.size() << " : " << size_);

		if (size_ > 16 || (size_ <= 16 && data_ != value))
		{
			dirty_ = true;
			data_ = value;
		}
		return *this;
	}
	void OGLUniformFloatArray::Load()
	{
		if (dirty_)
		{
			glProgramUniform1fv(program_, location_, size_, data_.data());
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformFloatArray::CopyResource() const
	{
		std::shared_ptr<OGLUniformFloatArray> u = std::make_shared<OGLUniformFloatArray>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		u->size_ = this->size_;
		return u;
	}
	Uniform & OGLUniformVec2Array::operator=(const std::vector<glm::vec2>& value)
	{
		WARNING(static_cast<GLsizei>(value.size()) == size_, "Array size is inconsistent..." << value.size() << " : " << size_);

		if (size_ > 8 || (size_ <= 8 && data_ != value))
		{
			dirty_ = true;
			data_ = value;
		}
		return *this;
	}
	void OGLUniformVec2Array::Load()
	{
		if (dirty_)
		{
			glProgramUniform2fv(program_, location_, size_,
				static_cast<GLfloat*>(static_cast<void*>(data_.data())));
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformVec2Array::CopyResource() const
	{
		std::shared_ptr<OGLUniformVec2Array> u = std::make_shared<OGLUniformVec2Array>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		u->size_ = this->size_;
		return u;
	}
	Uniform & OGLUniformVec3Array::operator=(const std::vector<glm::vec3>& value)
	{
		WARNING(static_cast<GLsizei>(value.size()) == size_, "Array size is inconsistent..." << value.size() << " : " << size_);

		if (size_ > 6 || (size_ <= 6 && data_ != value))
		{
			dirty_ = true;
			data_ = value;
		}
		return *this;
	}
	void OGLUniformVec3Array::Load()
	{
		if (dirty_)
		{
			glProgramUniform3fv(program_, location_, size_,
				static_cast<GLfloat*>(static_cast<void*>(data_.data())));
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformVec3Array::CopyResource() const
	{
		std::shared_ptr<OGLUniformVec3Array> u = std::make_shared<OGLUniformVec3Array>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		u->size_ = this->size_;
		return u;
	}
	Uniform & OGLUniformVec4Array::operator=(const std::vector<glm::vec4>& value)
	{
		WARNING(static_cast<GLsizei>(value.size()) == size_, "Array size is inconsistent..." << value.size() << " : " << size_);

		if (size_ > 4 || (size_ <= 4 && data_ != value))
		{
			dirty_ = true;
			data_ = value;
		}
		return *this;
	}
	void OGLUniformVec4Array::Load()
	{
		if (dirty_)
		{
			glProgramUniform4fv(program_, location_, size_,
				static_cast<GLfloat*>(static_cast<void*>(data_.data())));
			dirty_ = false;
		}
	}
	UniformPtr OGLUniformVec4Array::CopyResource() const
	{
		std::shared_ptr<OGLUniformVec4Array> u = std::make_shared<OGLUniformVec4Array>();
		u->name_ = this->name_;
		u->data_ = this->data_;
		u->dirty_ = true;
		u->size_ = this->size_;
		return u;
	}
}
