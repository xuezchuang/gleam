/************************************************************************ 
 * @description :  
 * @author		:  $username$
 * @creat 		:  $time$
************************************************************************ 
 * Copyright @ OscarShen 2017. All rights reserved. 
************************************************************************/  
#pragma once
#ifndef GLEAM_CORE_RENDER_MESH_H_
#define GLEAM_CORE_RENDER_MESH_H_

#include "renderable.h"
namespace gleam
{
	class Mesh : public Renderable
	{
	public:
		Mesh(const std::string &name, const RenderModelPtr &model);

		void LoadMeshInfo();

		RenderLayout &GetRenderLayout() const override { return *layout_; }

		void NumVertices(uint32_t n) { layout_->NumVertices(n); }
		uint32_t NumVertices() const { return layout_->NumVertices(); }

		void NumIndices(uint32_t n) { layout_->NumIndices(n); }
		uint32_t NumIndices() const { return layout_->NumIndices(); }

		void AddVertexStream(const void *buffer, uint32_t size, const VertexElement &element, uint32_t access_hint);
		void AddVertexStream(const GraphicsBufferPtr &buffer, const VertexElement &element);

		void AddIndexStream(const void *buffer, uint32_t size, ElementFormat format, uint32_t access_hint);
		void AddIndexStream(const GraphicsBufferPtr &buffer, ElementFormat format);

		int32_t MaterialID() const { return mtl_id_; }
		void MaterialID(int32_t mtl_id) { mtl_id_ = mtl_id; }

		void StartVertexLocation(uint32_t location) { layout_->StartVertexLocation(location); }
		uint32_t StartVertexLocation() const { return layout_->StartVertexLocation(); }

		void StartIndexLocation(uint32_t location) { layout_->StartIndexLocation(location); }
		uint32_t StartIndexLocation() const { return layout_->StartIndexLocation(); }

	protected:
		virtual void DoLoadMeshInfo();

	protected:
		std::string name_;

		RenderLayoutPtr layout_;

		int32_t mtl_id_;

		std::weak_ptr<RenderModel> model_ptr_;
	};

	class RenderModel
	{
	public:
		RenderModel(const std::string &name);

		void LoadModelInfo();

		void NumMaterials(size_t i) { materials_.resize(i); }
		size_t NumMaterials() const { return materials_.size(); }
		MaterialPtr &GetMaterial(int32_t index) { return materials_[index]; }
		const MaterialPtr &GetMaterial(int32_t index) const { return materials_[index]; }

		void ModelMatrix(const glm::mat4 &model);

		void ForEachMeshes(const std::function<void(const MeshPtr &)> func);

		template <typename ForwardIterator>
		void AssignMeshes(ForwardIterator first, ForwardIterator last)
		{
			meshes_.assign(first, last);
		}

		void AssignMeshes(std::vector<MeshPtr>&& parm_mesh)
		{
			meshes_ = parm_mesh;
		}

		int32_t NumMeshes() const { return meshes_.size(); }
		MeshPtr &GetMesh(int32_t index) { return meshes_[index]; }

	protected:
		virtual void DoLoadModelInfo() { }

	protected:
		std::string name_;

		std::vector<MeshPtr> meshes_;
		std::vector<MaterialPtr> materials_;

		glm::mat4 model_matrix_;
	};

	class RenderableLightPolygon : public RenderableHelper
	{
	public:
		RenderableLightPolygon();

		void OnRenderBegin() override;

	private:
		UniformPtr v0_;
		UniformPtr v1_;
		UniformPtr v2_;
		UniformPtr v3_;
		UniformPtr v4_;
		UniformPtr v5_;
		UniformPtr v6_;
		UniformPtr v7_;
	};

	class BasicPolygon : public Mesh
	{
	public:
		BasicPolygon(const std::string &name, const RenderModelPtr &model);
		void SetColor(const glm::vec4 &color);
		void OnRenderBegin() override;
		void OnRepeatRenderBegin(uint32_t i) override;
	};

	template <typename T>
	struct CreateMeshFunc
	{
		MeshPtr operator()(const std::string & name, const RenderModelPtr & model)
		{
			return std::make_shared<T>(name, model);
		}
	};

	template <typename T>
	struct CreateModelFunc
	{
		RenderModelPtr operator()(const std::string & name)
		{
			return std::make_shared<T>(name);
		}
	};

	bool LoadModel(const std::string &name, std::vector<MaterialPtr> &mtls,
		std::vector<VertexElement>& merged_ves,
		std::vector<std::vector<uint8_t>>& merged_buff, std::vector<uint8_t>& merged_indices,
		std::vector<std::string>& mesh_names, std::vector<int32_t>& mtl_ids,
		std::vector<uint32_t>& mesh_num_vertices, std::vector<uint32_t> &mesh_start_vertices,
		std::vector<uint32_t>& mesh_num_indices, std::vector<uint32_t> &mesh_start_indices
		);
	RenderModelPtr LoadModel(const std::string &name, uint32_t access_hint,
		std::function<RenderModelPtr(const std::string &)> create_model_func,
		std::function<MeshPtr(const std::string &, const RenderModelPtr &)> create_mesh_func);
}

#endif // !GLEAM_CORE_RENDER_MESH_H_
