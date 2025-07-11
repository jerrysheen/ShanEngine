#include "hzpch.h"
#include "D3D12CommandList.h"
#include "Platform/D3D12/D3D12RenderAPIManager.h"
#include "Platform/D3D12/D3D12GraphicsPipeline.h"
#include "Hazel.h"
#include "Platform/D3D12/d3dx12.h"
#include "Platform/D3D12/d3dUtil.h"
#include "Runtime/Graphics/RHI/Interface/IGfxViewManager.h"
#include "Runtime/Graphics/RHI/Interface/DescriptorTypes.h"
#include "Runtime/Core/Log/Log.h"
#include "D3D12Utils.h"

namespace Hazel 
{
	// 使用公共的D3D12Utils命名空间
	using namespace D3D12Utils;
	D3D12CommandList::D3D12CommandList(CommandListType type) 
		: CommandList() {
		m_type = type;
		
		// 如果没有通过管理器设置，就创建默认的对象
		if (!m_CommandAllocator && !m_CommandList) {
			InitializeD3D12Resources();
		}
	}


	D3D12CommandList::~D3D12CommandList()
	{
		//COMPtr在析构时会自动调用Release();
	}


	void D3D12CommandList::Reset()
	{
		// 调用带PSO参数的Reset，传入当前管线或nullptr
		Reset(m_currentPipeline);
	}

	void D3D12CommandList::Reset(Ref<IGraphicsPipeline> pipeline)
	{
		if (!m_CommandAllocator || !m_CommandList) {
			HZ_CORE_ERROR("[D3D12CommandList] Cannot reset: D3D12 objects not initialized");
			return;
		}

		// Reset the command allocator.
		HRESULT hr = m_CommandAllocator->Reset();
		if (FAILED(hr)) {
			HZ_CORE_ERROR("[D3D12CommandList] Failed to reset command allocator: {}",HRESULTToString(hr));
			return;
		}

		// 提取D3D12 PSO
		ID3D12PipelineState* pso = ExtractD3D12PSO(pipeline);
		
		// Reset the command list with PSO
		hr = m_CommandList->Reset(m_CommandAllocator.Get(), pso);
		if (FAILED(hr)) {
			HZ_CORE_ERROR("[D3D12CommandList] Failed to reset command list: {}",HRESULTToString(hr));
			return;
		}

		// 更新当前管线
		m_currentPipeline = pipeline;
		m_state = ExecutionState::Recording;
	}

	void D3D12CommandList::SetPipelineState(Ref<IGraphicsPipeline> pipeline)
	{
		if (!m_CommandList) {
			HZ_CORE_ERROR("[D3D12CommandList] Cannot set pipeline state: CommandList not initialized");
			return;
		}

		if (!pipeline) {
			HZ_CORE_WARN("[D3D12CommandList] Setting null pipeline state");
			m_currentPipeline = nullptr;
			return;
		}

		// 提取D3D12 PSO并设置
		ID3D12PipelineState* pso = ExtractD3D12PSO(pipeline);
		if (pso) {
			m_CommandList->SetPipelineState(pso);
			m_currentPipeline = pipeline;
			
			// 如果有根签名，也设置根签名
			if (auto d3d12Pipeline = std::dynamic_pointer_cast<D3D12GraphicsPipeline>(pipeline)) {
				ID3D12RootSignature* rootSignature = d3d12Pipeline->GetD3D12RootSignature();
				if (rootSignature) {
					m_CommandList->SetGraphicsRootSignature(rootSignature);
				}
			}
		} else {
			HZ_CORE_ERROR("[D3D12CommandList] Failed to extract D3D12 PSO from pipeline");
		}
	}

	Ref<IGraphicsPipeline> D3D12CommandList::GetCurrentPipeline() const
	{
		return m_currentPipeline;
	}

	ID3D12PipelineState* D3D12CommandList::ExtractD3D12PSO(Ref<IGraphicsPipeline> pipeline) const
	{
		if (!pipeline) {
			return nullptr;
		}

		// 尝试将IGraphicsPipeline转换为D3D12GraphicsPipeline
		if (auto d3d12Pipeline = std::dynamic_pointer_cast<D3D12GraphicsPipeline>(pipeline)) {
			return d3d12Pipeline->GetD3D12PipelineState();
		}

		// 如果转换失败，尝试通过GetNativeHandle获取
		void* nativeHandle = pipeline->GetNativeHandle();
		if (nativeHandle) {
			return static_cast<ID3D12PipelineState*>(nativeHandle);
		}

		HZ_CORE_WARN("[D3D12CommandList] Could not extract D3D12 PSO from pipeline");
		return nullptr;
	}

	void D3D12CommandList::ClearRenderTargetView(const Ref<TextureBuffer>& textureBuffer, const glm::vec4& color)
	{
		if (!m_CommandList) {
			HZ_CORE_ERROR("[D3D12CommandList] Cannot clear RTV: CommandList not initialized");
			return;
		}

		auto uuid = textureBuffer->GetUUID();
		IGfxViewManager& viewManager = IGfxViewManager::Get();
		auto descAllocation = viewManager.GetCachedView(uuid, DescriptorType::RTV);
		// 直接构造 D3D12_CPU_DESCRIPTOR_HANDLE
    	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{ descAllocation.baseHandle.cpuHandle };
		m_CommandList->ClearRenderTargetView(rtvHandle, &color[0], 0, nullptr);

		m_commandCount++;
	}


	void D3D12CommandList::ChangeResourceState(const Ref<TextureBuffer>& texture, const TextureRenderUsage& fromFormat, const TextureRenderUsage& toFormat)
	{

		auto bufferResource = texture->getResource<Microsoft::WRL::ComPtr<ID3D12Resource>>();
		D3D12_RESOURCE_STATES fromState = D3D12_RESOURCE_STATE_COMMON;
		D3D12_RESOURCE_STATES toState = D3D12_RESOURCE_STATE_COMMON;

		switch (fromFormat)
		{
		case TextureRenderUsage::RENDER_TARGET:
			fromState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			break;
		case TextureRenderUsage::RENDER_TEXTURE:
			fromState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			break;
		default:
			break;
		}

		switch (toFormat)
		{
		case TextureRenderUsage::RENDER_TARGET:
			texture->SetTextureRenderUsage(TextureRenderUsage::RENDER_TARGET);
			toState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			break;
		case TextureRenderUsage::RENDER_TEXTURE:
			texture->SetTextureRenderUsage(TextureRenderUsage::RENDER_TEXTURE);
			toState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			break;
		default:
			break;
		}
		CD3DX12_RESOURCE_BARRIER barrierToSRV = CD3DX12_RESOURCE_BARRIER::Transition(
			bufferResource.Get(),
			fromState, // 渲染目标时的 Render Target 状态
			toState // 切换到着色器资源状态
		);

		m_CommandList->ResourceBarrier(1, &barrierToSRV);

		// 这应该在需要的地方添加一个同步点
		m_commandCount++;

	}

	//void D3D12CommandList::BindCbvHeap(const Ref<GfxDescHeap>& cbvHeap)
	//{
	//	auto d3dCbvHeap = cbvHeap->getHeap<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>();
	//	ID3D12DescriptorHeap* descriptorHeaps[] = { d3dCbvHeap.Get()};
	//	m_CommandListLocal->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	//}

	void D3D12CommandList::Close()
	{
		if (!m_CommandList) {
			HZ_CORE_ERROR("[D3D12CommandList] Cannot close: CommandList not initialized");
			return;
		}

		HRESULT hr = m_CommandList->Close();
		if (FAILED(hr)) {
			HZ_CORE_ERROR("[D3D12CommandList] Failed to close command list: {}",HRESULTToString(hr));
			m_state = ExecutionState::Error;
		} else {
			m_state = ExecutionState::Closed;
		}
	}

	void D3D12CommandList::Execute()
	{
		if (m_state != ExecutionState::Closed) {
			HZ_CORE_ERROR("[D3D12CommandList] Cannot execute: CommandList not closed");
			return;
		}

		m_state = ExecutionState::Executing;
		// 实际的执行通常由CommandQueue处理
		// 这里我们只更新状态
		m_state = ExecutionState::Completed;
	}

	// D3D12特定方法实现
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3D12CommandList::GetD3D12CommandList() const {
		return m_CommandList;
	}

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> D3D12CommandList::GetD3D12Allocator() const {
		return m_CommandAllocator;
	}

	void D3D12CommandList::SetD3D12Objects(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator, 
	                                       Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList) {
		m_CommandAllocator = allocator;
		m_CommandList = commandList;
		UpdateNativeHandle();
	}

	void D3D12CommandList::InitializeD3D12Resources() {
		D3D12RenderAPIManager* renderAPIManager = dynamic_cast<D3D12RenderAPIManager*>(
			RenderAPIManager::getInstance()->GetManager().get());
		if (!renderAPIManager) {
			HZ_CORE_ERROR("[D3D12CommandList] Failed to get D3D12RenderAPIManager");
			return;
		}

		Microsoft::WRL::ComPtr<ID3D12Device> device = renderAPIManager->GetD3DDevice();
		if (!device) {
			HZ_CORE_ERROR("[D3D12CommandList] Failed to get D3D12 device");
			return;
		}

		D3D12_COMMAND_LIST_TYPE d3dType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		switch (m_type) {
			case CommandListType::Graphics:
				d3dType = D3D12_COMMAND_LIST_TYPE_DIRECT;
				break;
			case CommandListType::Compute:
				d3dType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
				break;
			case CommandListType::Copy:
				d3dType = D3D12_COMMAND_LIST_TYPE_COPY;
				break;
			default:
				break;
		}

		HRESULT hr = device->CreateCommandAllocator(d3dType, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf()));
		if (FAILED(hr)) {
			HZ_CORE_ERROR("[D3D12CommandList] Failed to create command allocator: {}",HRESULTToString(hr));
			return;
		}

		hr = device->CreateCommandList(0, d3dType, m_CommandAllocator.Get(), nullptr, 
		                              IID_PPV_ARGS(m_CommandList.GetAddressOf()));
		if (FAILED(hr)) {
			HZ_CORE_ERROR("[D3D12CommandList] Failed to create command list: {}",HRESULTToString(hr));
			return;
		}

		// CommandList创建时是开放状态，需要先关闭
		m_CommandList->Close();
		UpdateNativeHandle();
	}

	void D3D12CommandList::UpdateNativeHandle() {
		m_nativeHandle.commandList = m_CommandList.Get();
		m_nativeHandle.commandAllocator = m_CommandAllocator.Get();
		m_nativeHandle.isValid = (m_CommandList && m_CommandAllocator);
	}
}