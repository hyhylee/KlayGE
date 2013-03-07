// RenderableHelper.cpp
// KlayGE 一些常用的可渲染对象 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2005-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// RenderableSkyBox和RenderableHDRSkyBox增加了Technique() (2010.1.4)
//
// 3.9.0
// 增加了RenderableHDRSkyBox (2009.5.4)
//
// 2.7.1
// 增加了RenderableHelper基类 (2005.7.10)
//
// 2.6.0
// 增加了RenderableSkyBox (2005.5.26)
//
// 2.5.0
// 增加了RenderablePoint，RenderableLine和RenderableTriangle (2005.4.13)
//
// 2.4.0
// 初次建立 (2005.3.22)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Camera.hpp>

#include <cstring>

#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	RenderableHelper::RenderableHelper(std::wstring const & name)
		: name_(name)
	{
	}

	RenderTechniquePtr const & RenderableHelper::GetRenderTechnique() const
	{
		return technique_;
	}

	RenderLayoutPtr const & RenderableHelper::GetRenderLayout() const
	{
		return rl_;
	}

	AABBox const & RenderableHelper::PosBound() const
	{
		return pos_aabb_;
	}

	AABBox const & RenderableHelper::TexcoordBound() const
	{
		return tc_aabb_;
	}

	std::wstring const & RenderableHelper::Name() const
	{
		return name_;
	}


	RenderablePoint::RenderablePoint()
		: RenderableHelper(L"Point")
	{
		this->Init();
	}

	RenderablePoint::RenderablePoint(float3 const & v, Color const & clr)
		: RenderableHelper(L"Point")
	{
		this->Init();

		this->SetPoint(v);
		this->SetColor(clr);
	}

	void RenderablePoint::SetPoint(float3 const & v)
	{
		pos_aabb_.Min() = pos_aabb_.Max() = v;
		*v0_ep_ = v;
	}

	void RenderablePoint::SetColor(Color const & clr)
	{
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());
	}

	void RenderablePoint::OnRenderBegin()
	{
		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
		*mvp_ep_ = model_mat_ * camera.ViewProjMatrix();
	}

	void RenderablePoint::Init()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderEffectPtr effect = rf.LoadEffect("RenderableHelper.fxml");
		if (deferred_effect_)
		{
			this->BindDeferredEffect(effect);
			depth_tech_ = effect->TechniqueByName("PointDepthTech");
			gbuffer_rt0_tech_ = effect->TechniqueByName("PointRT0Tech");
			gbuffer_rt1_tech_ = effect->TechniqueByName("PointRT1Tech");
			gbuffer_mrt_tech_ = effect->TechniqueByName("PointMRTTech");
		}
		else
		{
			technique_ = effect->TechniqueByName("PointTec");
		}
		v0_ep_ = effect->ParameterByName("v0");
		color_ep_ = effect->ParameterByName("color");
		mvp_ep_ = effect->ParameterByName("mvp");

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_PointList);

		float v = 0;
		ElementInitData init_data;
		init_data.row_pitch = sizeof(v);
		init_data.slice_pitch = 0;
		init_data.data = &v;

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, make_tuple(vertex_element(VEU_Position, 0, EF_R32F)));

		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

		*(effect->ParameterByName("pos_center")) = float3(0, 0, 0);
		*(effect->ParameterByName("pos_extent")) = float3(1, 1, 1);
	}


	RenderableLine::RenderableLine()
		: RenderableHelper(L"Line")
	{
		this->Init();
	}
	
	RenderableLine::RenderableLine(float3 const & v0, float3 const & v1, Color const & clr)
		: RenderableHelper(L"Line")
	{
		this->Init();

		this->SetLine(v0, v1);
		this->SetColor(clr);
	}

	void RenderableLine::SetLine(float3 const & v0, float3 const & v1)
	{
		float3 vs[] = 
		{
			v0, v1
		};
		pos_aabb_ = MathLib::compute_aabbox(&vs[0], &vs[0] + sizeof(vs) / sizeof(vs[0]));
		
		*v0_ep_ = v0;
		*v1_ep_ = v1;
	}

	void RenderableLine::SetColor(Color const & clr)
	{
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());
	}

	void RenderableLine::OnRenderBegin()
	{
		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
		*mvp_ep_ = model_mat_ * camera.ViewProjMatrix();
	}

	void RenderableLine::Init()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderEffectPtr effect = rf.LoadEffect("RenderableHelper.fxml");
		if (deferred_effect_)
		{
			this->BindDeferredEffect(effect);
			depth_tech_ = effect->TechniqueByName("LineDepthTech");
			gbuffer_rt0_tech_ = effect->TechniqueByName("LineRT0Tech");
			gbuffer_rt1_tech_ = effect->TechniqueByName("LineRT1Tech");
			gbuffer_mrt_tech_ = effect->TechniqueByName("LineMRTTech");
		}
		else
		{
			technique_ = effect->TechniqueByName("LineTec");
		}
		v0_ep_ = effect->ParameterByName("v0");
		v1_ep_ = effect->ParameterByName("v1");
		color_ep_ = effect->ParameterByName("color");
		mvp_ep_ = effect->ParameterByName("mvp");

		float vertices[] =
		{
			0, 1
		};

		ElementInitData init_data;
		init_data.row_pitch = sizeof(vertices);
		init_data.slice_pitch = 0;
		init_data.data = vertices;

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_LineList);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, make_tuple(vertex_element(VEU_Position, 0, EF_R32F)));

		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

		*(effect->ParameterByName("pos_center")) = float3(0, 0, 0);
		*(effect->ParameterByName("pos_extent")) = float3(1, 1, 1);
	}


	RenderableTriangle::RenderableTriangle()
		: RenderableHelper(L"Triangle")
	{
		this->Init();
	}

	RenderableTriangle::RenderableTriangle(float3 const & v0, float3 const & v1, float3 const & v2, Color const & clr)
		: RenderableHelper(L"Triangle")
	{
		this->Init();

		this->SetTriangle(v0, v1, v2);
		this->SetColor(clr);
	}

	void RenderableTriangle::SetTriangle(float3 const & v0, float3 const & v1, float3 const & v2)
	{
		float3 vs[] = 
		{
			v0, v1, v2
		};
		pos_aabb_ = MathLib::compute_aabbox(&vs[0], &vs[0] + sizeof(vs) / sizeof(vs[0]));
		
		*v0_ep_ = v0;
		*v1_ep_ = v1;
		*v2_ep_ = v2;
	}

	void RenderableTriangle::SetColor(Color const & clr)
	{
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());
	}

	void RenderableTriangle::OnRenderBegin()
	{
		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
		*mvp_ep_ = model_mat_ * camera.ViewProjMatrix();
	}

	void RenderableTriangle::Init()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderEffectPtr effect = rf.LoadEffect("RenderableHelper.fxml");
		if (deferred_effect_)
		{
			this->BindDeferredEffect(effect);
			depth_tech_ = effect->TechniqueByName("LineDepthTech");
			gbuffer_rt0_tech_ = effect->TechniqueByName("LineRT0Tech");
			gbuffer_rt1_tech_ = effect->TechniqueByName("LineRT1Tech");
			gbuffer_mrt_tech_ = effect->TechniqueByName("LineMRTTech");
		}
		else
		{
			technique_ = effect->TechniqueByName("LineTec");
		}
		v0_ep_ = effect->ParameterByName("v0");
		v1_ep_ = effect->ParameterByName("v1");
		v2_ep_ = effect->ParameterByName("v2");
		color_ep_ = effect->ParameterByName("color");
		mvp_ep_ = effect->ParameterByName("mvp");

		float vertices[] =
		{
			0, 1, 2
		};

		ElementInitData init_data;
		init_data.row_pitch = sizeof(vertices);
		init_data.slice_pitch = 0;
		init_data.data = vertices;

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, make_tuple(vertex_element(VEU_Position, 0, EF_R32F)));

		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

		*(effect->ParameterByName("pos_center")) = float3(0, 0, 0);
		*(effect->ParameterByName("pos_extent")) = float3(1, 1, 1);
	}


	RenderableTriBox::RenderableTriBox()
		: RenderableHelper(L"TriBox")
	{
		this->Init();
	}

	RenderableTriBox::RenderableTriBox(OBBox const & obb, Color const & clr)
		: RenderableHelper(L"TriBox")
	{
		this->Init();

		this->SetBox(obb);
		this->SetColor(clr);
	}

	void RenderableTriBox::SetBox(OBBox const & obb)
	{
		pos_aabb_ = MathLib::convert_to_aabbox(obb);

		*v0_ep_ = obb.Corner(0);
		*v1_ep_ = obb.Corner(1);
		*v2_ep_ = obb.Corner(2);
		*v3_ep_ = obb.Corner(3);
		*v4_ep_ = obb.Corner(4);
		*v5_ep_ = obb.Corner(5);
		*v6_ep_ = obb.Corner(6);
		*v7_ep_ = obb.Corner(7);
	}

	void RenderableTriBox::SetColor(Color const & clr)
	{
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());
	}

	void RenderableTriBox::OnRenderBegin()
	{
		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
		*mvp_ep_ = model_mat_ * camera.ViewProjMatrix();
	}

	void RenderableTriBox::Init()
	{		
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderEffectPtr effect = rf.LoadEffect("RenderableHelper.fxml");
		if (deferred_effect_)
		{
			this->BindDeferredEffect(effect);
			depth_tech_ = effect->TechniqueByName("LineDepthTech");
			gbuffer_rt0_tech_ = effect->TechniqueByName("LineRT0Tech");
			gbuffer_rt1_tech_ = effect->TechniqueByName("LineRT1Tech");
			gbuffer_mrt_tech_ = effect->TechniqueByName("LineMRTTech");
		}
		else
		{
			technique_ = effect->TechniqueByName("LineTec");
		}
		v0_ep_ = effect->ParameterByName("v0");
		v1_ep_ = effect->ParameterByName("v1");
		v2_ep_ = effect->ParameterByName("v2");
		v3_ep_ = effect->ParameterByName("v3");
		v4_ep_ = effect->ParameterByName("v4");
		v5_ep_ = effect->ParameterByName("v5");
		v6_ep_ = effect->ParameterByName("v6");
		v7_ep_ = effect->ParameterByName("v7");
		color_ep_ = effect->ParameterByName("color");
		mvp_ep_ = effect->ParameterByName("mvp");

		float vertices[] =
		{
			0, 1, 2, 3, 4, 5, 6, 7
		};

		uint16_t indices[] =
		{
			0, 2, 3, 3, 1, 0,
			5, 7, 6, 6, 4, 5,
			4, 0, 1, 1, 5, 4,
			4, 6, 2, 2, 0, 4,
			2, 6, 7, 7, 3, 2,
			1, 3, 7, 7, 5, 1
		};

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);

		ElementInitData init_data;
		init_data.row_pitch = sizeof(vertices);
		init_data.slice_pitch = 0;
		init_data.data = vertices;

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, make_tuple(vertex_element(VEU_Position, 0, EF_R32F)));

		init_data.row_pitch = sizeof(indices);
		init_data.slice_pitch = 0;
		init_data.data = indices;

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindIndexStream(ib, EF_R16UI);

		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

		*(effect->ParameterByName("pos_center")) = float3(0, 0, 0);
		*(effect->ParameterByName("pos_extent")) = float3(1, 1, 1);
	}


	RenderableLineBox::RenderableLineBox()
		: RenderableHelper(L"LineBox")
	{
		this->Init();
	}
	
	RenderableLineBox::RenderableLineBox(OBBox const & obb, Color const & clr)
		: RenderableHelper(L"LineBox")
	{
		this->Init();

		this->SetBox(obb);
		this->SetColor(clr);
	}

	void RenderableLineBox::SetBox(OBBox const & obb)
	{
		pos_aabb_ = MathLib::convert_to_aabbox(obb);

		*v0_ep_ = obb.Corner(0);
		*v1_ep_ = obb.Corner(1);
		*v2_ep_ = obb.Corner(2);
		*v3_ep_ = obb.Corner(3);
		*v4_ep_ = obb.Corner(4);
		*v5_ep_ = obb.Corner(5);
		*v6_ep_ = obb.Corner(6);
		*v7_ep_ = obb.Corner(7);
	}

	void RenderableLineBox::SetColor(Color const & clr)
	{
		*color_ep_ = float4(clr.r(), clr.g(), clr.b(), clr.a());
	}

	void RenderableLineBox::OnRenderBegin()
	{
		Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
		*mvp_ep_ = model_mat_ * camera.ViewProjMatrix();
	}

	void RenderableLineBox::Init()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderEffectPtr effect = rf.LoadEffect("RenderableHelper.fxml");
		if (deferred_effect_)
		{
			this->BindDeferredEffect(effect);
			depth_tech_ = effect->TechniqueByName("LineDepthTech");
			gbuffer_rt0_tech_ = effect->TechniqueByName("LineRT0Tech");
			gbuffer_rt1_tech_ = effect->TechniqueByName("LineRT1Tech");
			gbuffer_mrt_tech_ = effect->TechniqueByName("LineMRTTech");
		}
		else
		{
			technique_ = effect->TechniqueByName("LineTec");
		}
		v0_ep_ = effect->ParameterByName("v0");
		v1_ep_ = effect->ParameterByName("v1");
		v2_ep_ = effect->ParameterByName("v2");
		v3_ep_ = effect->ParameterByName("v3");
		v4_ep_ = effect->ParameterByName("v4");
		v5_ep_ = effect->ParameterByName("v5");
		v6_ep_ = effect->ParameterByName("v6");
		v7_ep_ = effect->ParameterByName("v7");
		color_ep_ = effect->ParameterByName("color");
		mvp_ep_ = effect->ParameterByName("mvp");

		float vertices[] =
		{
			0, 1, 2, 3, 4, 5, 6, 7
		};

		uint16_t indices[] =
		{
			0, 1, 1, 3, 3, 2, 2, 0,
			4, 5, 5, 7, 7, 6, 6, 4,
			0, 4, 1, 5, 2, 6, 3, 7
		};

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_LineList);

		ElementInitData init_data;
		init_data.row_pitch = sizeof(vertices);
		init_data.slice_pitch = 0;
		init_data.data = vertices;

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, make_tuple(vertex_element(VEU_Position, 0, EF_R32F)));

		init_data.row_pitch = sizeof(indices);
		init_data.slice_pitch = 0;
		init_data.data = indices;

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindIndexStream(ib, EF_R16UI);

		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

		*(effect->ParameterByName("pos_center")) = float3(0, 0, 0);
		*(effect->ParameterByName("pos_extent")) = float3(1, 1, 1);
	}


	RenderablePlane::RenderablePlane(float length, float width,
				int length_segs, int width_segs, bool has_tex_coord)
			: RenderableHelper(L"RenderablePlane")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);

		std::vector<float3> pos;
		for (int y = 0; y < width_segs + 1; ++ y)
		{
			for (int x = 0; x < length_segs + 1; ++ x)
			{
				pos.push_back(float3(x * (length / length_segs) - length / 2,
					-y * (width / width_segs) + width / 2, 0.0f));
			}
		}

		ElementInitData init_data;
		init_data.row_pitch = static_cast<uint32_t>(pos.size() * sizeof(pos[0]));
		init_data.slice_pitch = 0;
		init_data.data = &pos[0];

		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(pos_vb, make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		if (has_tex_coord)
		{
			std::vector<float2> tex;
			for (int y = 0; y < width_segs + 1; ++ y)
			{
				for (int x = 0; x < length_segs + 1; ++ x)
				{
					tex.push_back(float2(static_cast<float>(x) / length_segs,
						static_cast<float>(y) / width_segs));
				}
			}

			init_data.row_pitch = static_cast<uint32_t>(tex.size() * sizeof(tex[0]));
			init_data.slice_pitch = 0;
			init_data.data = &tex[0];

			GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
			rl_->BindVertexStream(tex_vb, make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));
		}

		std::vector<uint16_t> index;
		for (int y = 0; y < width_segs; ++ y)
		{
			for (int x = 0; x < length_segs; ++ x)
			{
				index.push_back(static_cast<uint16_t>((y + 0) * (length_segs + 1) + (x + 0)));
				index.push_back(static_cast<uint16_t>((y + 0) * (length_segs + 1) + (x + 1)));
				index.push_back(static_cast<uint16_t>((y + 1) * (length_segs + 1) + (x + 1)));

				index.push_back(static_cast<uint16_t>((y + 1) * (length_segs + 1) + (x + 1)));
				index.push_back(static_cast<uint16_t>((y + 1) * (length_segs + 1) + (x + 0)));
				index.push_back(static_cast<uint16_t>((y + 0) * (length_segs + 1) + (x + 0)));
			}
		}

		init_data.row_pitch = static_cast<uint32_t>(index.size() * sizeof(index[0]));
		init_data.slice_pitch = 0;
		init_data.data = &index[0];

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindIndexStream(ib, EF_R16UI);

		pos_aabb_ = MathLib::compute_aabbox(pos.begin(), pos.end());
		tc_aabb_ = AABBox(float3(0, 0, 0), float3(1, 1, 0));
	}


	RenderDecal::RenderDecal(TexturePtr const & normal_tex, TexturePtr const & diffuse_tex, float3 const & diffuse_clr,
			TexturePtr const & specular_tex, float3 const & specular_level, float shininess)
		: RenderableHelper(L"Decal")
	{
		BOOST_ASSERT(deferred_effect_);

		gbuffer_alpha_test_rt0_tech_ = deferred_effect_->TechniqueByName("DecalGBufferAlphaTestRT0Tech");
		gbuffer_alpha_test_rt1_tech_ = deferred_effect_->TechniqueByName("DecalGBufferAlphaTestRT1Tech");
		gbuffer_alpha_test_mrt_tech_ = deferred_effect_->TechniqueByName("DecalGBufferAlphaTestMRTTech");
		technique_ = gbuffer_alpha_test_rt0_tech_;

		pos_aabb_ = AABBox(float3(-1, -1, -1), float3(1, 1, 1));
		tc_aabb_ = AABBox(float3(0, 0, 0), float3(1, 1, 0));

		float3 xyzs[] =
		{
			pos_aabb_.Corner(0), pos_aabb_.Corner(1), pos_aabb_.Corner(2), pos_aabb_.Corner(3),
			pos_aabb_.Corner(4), pos_aabb_.Corner(5), pos_aabb_.Corner(6), pos_aabb_.Corner(7)
		};

		uint16_t indices[] =
		{
			0, 2, 3, 3, 1, 0,
			5, 7, 6, 6, 4, 5,
			4, 0, 1, 1, 5, 4,
			4, 6, 2, 2, 0, 4,
			2, 6, 7, 7, 3, 2,
			1, 3, 7, 7, 5, 1
		};

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		rl_ = rf.MakeRenderLayout();
		rl_->TopologyType(RenderLayout::TT_TriangleList);

		ElementInitData init_data;
		init_data.row_pitch = sizeof(xyzs);
		init_data.slice_pitch = 0;
		init_data.data = xyzs;

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindVertexStream(vb, make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		init_data.row_pitch = sizeof(indices);
		init_data.slice_pitch = 0;
		init_data.data = indices;

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		rl_->BindIndexStream(ib, EF_R16UI);

		model_mat_ = float4x4::Identity();
		effect_attrs_ |= EA_AlphaTest;

		inv_mv_ep_ = technique_->Effect().ParameterByName("inv_mv");

		normal_tex_ = normal_tex;
		diffuse_tex_ = diffuse_tex;
		diffuse_clr_ = diffuse_clr;
		specular_tex_ = specular_tex;
		specular_level_ = specular_level.x();
		shininess_ = shininess;
	}

	void RenderDecal::OnRenderBegin()
	{
		RenderableHelper::OnRenderBegin();

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		Camera const & camera = *re.CurFrameBuffer()->GetViewport()->camera;

		float4x4 const & view_to_decal = MathLib::inverse(model_mat_ * camera.ViewMatrix());
					
		switch (type_)
		{
		case PT_OpaqueGBufferRT0:
		case PT_TransparencyBackGBufferRT0:
		case PT_TransparencyFrontGBufferRT0:
		case PT_OpaqueGBufferRT1:
		case PT_TransparencyBackGBufferRT1:
		case PT_TransparencyFrontGBufferRT1:
		case PT_OpaqueGBufferMRT:
		case PT_TransparencyBackGBufferMRT:
		case PT_TransparencyFrontGBufferMRT:
			*diffuse_clr_param_ = float4(diffuse_clr_.x(), diffuse_clr_.y(), diffuse_clr_.z(), static_cast<float>(!!diffuse_tex_));
			*specular_level_param_ = float4(specular_level_, 0, 0, static_cast<float>(!!specular_tex_));
			*shininess_param_ = MathLib::clamp(shininess_ / 256.0f, 1e-6f, 0.999f);
			*inv_mv_ep_ = view_to_decal;
			break;

		default:
			break;
		}
	}
}
