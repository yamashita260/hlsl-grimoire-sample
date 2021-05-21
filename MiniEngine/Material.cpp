#include "stdafx.h"
#include "Material.h"

//���[�g�V�O�l�`���ƃp�C�v���C���X�e�[�g����̓J���J���J��
enum {
	enDescriptorHeap_CB,
	enDescriptorHeap_SRV,
	enNumDescriptorHeap
};
	
void Material::InitTexture(const TkmFile::SMaterial& tkmMat)
{

	const auto& nullTextureMaps = g_graphicsEngine->GetNullTextureMaps();
	if (tkmMat.albedoMap != nullptr) {
		m_albedoMap.InitFromMemory(tkmMat.albedoMap.get(), tkmMat.albedoMapSize);
	}
	else {
		m_albedoMap.InitFromMemory(
			nullTextureMaps.GetAlbedoMap().get(), 
			nullTextureMaps.GetAlbedoMapSize());
	}
	if (tkmMat.normalMap != nullptr) {
		m_normalMap.InitFromMemory(tkmMat.normalMap.get(), tkmMat.normalMapSize);
	}
	else {
		m_normalMap.InitFromMemory(
			nullTextureMaps.GetNormalMap().get(), 
			nullTextureMaps.GetNormalMapSize());
	}
	if (tkmMat.specularMap != nullptr) {
		m_specularMap.InitFromMemory(tkmMat.specularMap.get(), tkmMat.specularMapSize);
	}
	else {
		m_specularMap.InitFromMemory(
			nullTextureMaps.GetSpecularMap().get(),
			nullTextureMaps.GetSpecularMapSize());
	}

	if (tkmMat.reflectionMap != nullptr) {
		m_reflectionMap.InitFromMemory(tkmMat.reflectionMap.get(), tkmMat.reflectionMapSize);
	}
	else {
		m_reflectionMap.InitFromMemory(
			nullTextureMaps.GetReflectionMap().get(),
			nullTextureMaps.GetReflectionMapSize());
	}

	if (tkmMat.refractionMap != nullptr) {
		m_refractionMap.InitFromMemory(tkmMat.refractionMap.get(), tkmMat.refractionMapSize);
	}
	else {
		m_refractionMap.InitFromMemory(
			nullTextureMaps.GetRefractionMap().get(),
			nullTextureMaps.GetRefractionMapSize());
	}
}
void Material::InitFromTkmMaterila(
	const TkmFile::SMaterial& tkmMat,
	const char* fxFilePath,
	const char* vsEntryPointFunc,
	const char* vsSkinEntryPointFunc,
	const char* psEntryPointFunc,
	const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat
)
{
	//�e�N�X�`�������[�h�B
	InitTexture(tkmMat);
	
	//�萔�o�b�t�@���쐬�B
	SMaterialParam matParam;
	matParam.hasNormalMap = m_normalMap.IsValid() ? 1 : 0;
	matParam.hasSpecMap = m_specularMap.IsValid() ? 1 : 0;
	m_constantBuffer.Init(sizeof(SMaterialParam), &matParam);

	//���[�g�V�O�l�`�����������B
	D3D12_STATIC_SAMPLER_DESC samplerDescArray[2];
	//�f�t�H���g�̃T���v��
	samplerDescArray[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescArray[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescArray[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescArray[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescArray[0].MipLODBias = 0;
	samplerDescArray[0].MaxAnisotropy = 0;
	samplerDescArray[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDescArray[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDescArray[0].MinLOD = 0.0f;
	samplerDescArray[0].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDescArray[0].ShaderRegister = 0;
	samplerDescArray[0].RegisterSpace = 0;
	samplerDescArray[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	//�V���h�E�}�b�v�p�̃T���v���B
	samplerDescArray[1] = samplerDescArray[0];
	//��r�Ώۂ̒l����������΂O�A�傫����΂P��Ԃ���r�֐���ݒ肷��B
	samplerDescArray[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDescArray[1].ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER;
	samplerDescArray[1].MaxAnisotropy = 1;
	samplerDescArray[1].ShaderRegister = 1;

	m_rootSignature.Init(
		samplerDescArray,
		2
	);

	if (fxFilePath != nullptr && strlen(fxFilePath) > 0) {
		//�V�F�[�_�[���������B
		InitShaders(fxFilePath, vsEntryPointFunc, vsSkinEntryPointFunc, psEntryPointFunc);
		//�p�C�v���C���X�e�[�g���������B
		InitPipelineState(colorBufferFormat);
	}
}
void Material::InitPipelineState(const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat)
{
	// ���_���C�A�E�g���`����B
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	//�p�C�v���C���X�e�[�g���쐬�B
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsSkinModel->GetCompiledBlob());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_psModel->GetCompiledBlob());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	int numRenderTarget = 0;
	for (auto& format : colorBufferFormat) {
		if (format == DXGI_FORMAT_UNKNOWN) {
			//�t�H�[�}�b�g���w�肳��Ă��Ȃ��ꏊ��������I���B
			break;
		}
		psoDesc.RTVFormats[numRenderTarget] = colorBufferFormat[numRenderTarget];
		numRenderTarget++;
	}
	psoDesc.NumRenderTargets = numRenderTarget;

	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	m_skinModelPipelineState.Init(psoDesc);

	//�����ăX�L���Ȃ����f���p���쐬�B
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsNonSkinModel->GetCompiledBlob());
	m_nonSkinModelPipelineState.Init(psoDesc);

	//�����Ĕ������}�e���A���p�B
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsSkinModel->GetCompiledBlob());
	psoDesc.BlendState.IndependentBlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	
	m_transSkinModelPipelineState.Init(psoDesc);

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsNonSkinModel->GetCompiledBlob());
	m_transNonSkinModelPipelineState.Init(psoDesc);

}
void Material::InitShaders(
	const char* fxFilePath,
	const char* vsEntryPointFunc,
	const char* vsSkinEntriyPointFunc,
	const char* psEntryPointFunc
)
{
	//�X�L���Ȃ����f���p�̃V�F�[�_�[�����[�h����B
	m_vsNonSkinModel = g_engine->GetShaderFromBank(fxFilePath, vsEntryPointFunc);
	if (m_vsNonSkinModel == nullptr) {
		m_vsNonSkinModel = new Shader;
		m_vsNonSkinModel->LoadVS(fxFilePath, vsEntryPointFunc);
		g_engine->RegistShaderToBank(fxFilePath, vsEntryPointFunc, m_vsNonSkinModel);
	}
	//�X�L�����胂�f���p�̃V�F�[�_�[�����[�h����B
	m_vsSkinModel = g_engine->GetShaderFromBank(fxFilePath, vsSkinEntriyPointFunc);
	if (m_vsSkinModel == nullptr) {
		m_vsSkinModel = new Shader;
		m_vsSkinModel->LoadVS(fxFilePath, vsSkinEntriyPointFunc);
		g_engine->RegistShaderToBank(fxFilePath, vsSkinEntriyPointFunc, m_vsSkinModel);
	}

	m_psModel = g_engine->GetShaderFromBank(fxFilePath, psEntryPointFunc);
	if (m_psModel == nullptr) {
		m_psModel = new Shader;
		m_psModel->LoadPS(fxFilePath, psEntryPointFunc);
		g_engine->RegistShaderToBank(fxFilePath, psEntryPointFunc, m_psModel);
	}
}
void Material::BeginRender(RenderContext& rc, int hasSkin)
{
	rc.SetRootSignature(m_rootSignature);
	
	if (hasSkin) {
	//	rc.SetPipelineState(m_skinModelPipelineState);
		rc.SetPipelineState(m_transSkinModelPipelineState);
	}
	else {
	//	rc.SetPipelineState(m_nonSkinModelPipelineState);
		rc.SetPipelineState(m_transNonSkinModelPipelineState);
	}
}