#ifndef __VULKAN_COMMAND_BUFFER_H
#define __VULKAN_COMMAND_BUFFER_H

#include <vulkan/vulkan.h>
#include <memory>
#include <mutex>

#include "Device.h"
#include "RenderPass.h"

namespace Vulkan
{
  class CommandBuffer_impl
  {
  public:
    CommandBuffer_impl() = delete;
    CommandBuffer_impl(const CommandBuffer_impl &obj) = delete;
    CommandBuffer_impl(CommandBuffer_impl &&obj) = delete;
    CommandBuffer_impl &operator=(const CommandBuffer_impl &obj) = delete;
    CommandBuffer_impl &operator=(CommandBuffer_impl &&obj) = delete;
    ~CommandBuffer_impl() noexcept;
  private:
    friend class CommandBuffer;
    CommandBuffer_impl(const std::shared_ptr<Device> dev, const VkCommandPool pool, const VkCommandBufferLevel level) noexcept;
    void SetMemoryBarrier(const std::vector<VkBufferMemoryBarrier> buffer_barriers,
                          const std::vector<VkMemoryBarrier> memory_barriers,
                          const std::vector<VkImageMemoryBarrier> image_bariers,
                          const VkPipelineStageFlags src_tage_flags, 
                          const VkPipelineStageFlags dst_tage_flags) noexcept;

    bool IsError() noexcept { return state == BufferState::Error; }
    bool IsReady() noexcept { return state == BufferState::Ready; }
    bool IsReset() noexcept { return state == BufferState::NotReady; }

    void BeginCommandBuffer() noexcept;
    void EndCommandBuffer() noexcept;
    void ResetCommandBuffer() noexcept;
    VkResult ExecuteBuffer(const uint32_t family_queue_index) noexcept;
    VkResult WaitForExecute(const uint64_t timeout) noexcept;

    void BeginRenderPass(const std::shared_ptr<Vulkan::RenderPass> render_pass, const uint32_t frame_buffer_index, const VkOffset2D offset = {0, 0}) noexcept;
    void EndRenderPass() noexcept;
    void NextSubpass() noexcept;

    void DrawIndexed(const uint32_t index_count, const uint32_t first_index, const uint32_t vertex_offset = 0, const uint32_t instance_count = 1, const uint32_t first_instance = 0) noexcept;
    void Draw(const uint32_t vertex_count, const uint32_t first_vertex = 0, const uint32_t instance_count = 1, const uint32_t first_instance = 0) noexcept;
    void Dispatch(const uint32_t x, const uint32_t y, const uint32_t z) noexcept;

    void BindPipeline(const VkPipeline pipeline, const VkPipelineBindPoint bind_point) noexcept;
    void BindDescriptorSets(const VkPipelineLayout pipeline_layout, const VkPipelineBindPoint bind_point, const std::vector<VkDescriptorSet> sets, const uint32_t first_set, const std::vector<uint32_t> dynamic_offeset) noexcept;
    void BindVertexBuffers(const std::vector<VkBuffer> buffers, const std::vector<VkDeviceSize> offsets, const uint32_t first_binding, const uint32_t binding_count) noexcept;
    void BindIndexBuffer(const VkBuffer buffer, const VkIndexType index_type, const VkDeviceSize offset = 0) noexcept;

    void SetViewport(const std::vector<VkViewport> &viewports) noexcept;
    void SetDepthBias(const float depth_bias_constant_factor, const float depth_bias_clamp, const float depth_bias_slope_factor) noexcept;

    void ImageLayoutTransition(ImageArray &image, const size_t image_index, const VkImageLayout new_layout, const uint32_t mip_level = 0, const bool transit_all_mip_levels = true) noexcept;

    void CopyBufferToImage(const VkBuffer src, ImageArray &image, const size_t image_index, const std::vector<VkBufferImageCopy> regions) noexcept;

    std::shared_ptr<Device> device;
    VkCommandBuffer buffer = VK_NULL_HANDLE;
    VkFence exec_fence = VK_NULL_HANDLE;
    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    enum class BufferState
    {
      NotReady,
      Ready,
      Error,
      OnWrite
    } state = BufferState::NotReady;
  };
  
  class CommandBuffer
  {
  private:
    friend class CommandPool_impl;
    friend class CommandPool;
    std::unique_ptr<CommandBuffer_impl> impl;
    CommandBuffer() noexcept = default;
    VkResult ExecuteBuffer(const uint32_t family_queue_index) noexcept { if (impl.get()) return impl->ExecuteBuffer(family_queue_index); return VK_ERROR_UNKNOWN; }
    VkResult WaitForExecute(const uint64_t timeout = UINT64_MAX) noexcept { if (impl.get()) return impl->WaitForExecute(timeout); return VK_ERROR_UNKNOWN; }
    void ResetCommandBuffer() noexcept { if (impl.get()) impl->ResetCommandBuffer(); }
  public:
    ~CommandBuffer() noexcept = default;
    CommandBuffer(const CommandBuffer &obj) = delete;
    CommandBuffer(CommandBuffer &&obj) noexcept : impl(std::move(obj.impl)) {};
    CommandBuffer(const std::shared_ptr<Device> dev, const VkCommandPool pool, const VkCommandBufferLevel level) noexcept : 
      impl(std::unique_ptr<CommandBuffer_impl>(new CommandBuffer_impl(dev, pool, level))) {}
    CommandBuffer &operator=(const CommandBuffer &obj) = delete;
    CommandBuffer &operator=(CommandBuffer &&obj) noexcept;
    void swap(CommandBuffer &obj) noexcept;
    bool IsValid() noexcept { return impl != nullptr; }
    bool IsError() noexcept { return !impl.get() || impl->IsError(); }
    bool IsReady() noexcept { return impl.get() && impl->IsReady(); }
    bool IsReset() noexcept { return impl.get() && impl->IsReset(); }
    auto &BeginCommandBuffer() noexcept { if (impl.get()) impl->BeginCommandBuffer(); return *this; }
    auto &EndCommandBuffer() noexcept { if (impl.get()) impl->EndCommandBuffer(); return *this; }
    auto &BeginRenderPass(const std::shared_ptr<Vulkan::RenderPass> render_pass, const uint32_t frame_buffer_index, const VkOffset2D offset = {0, 0}) noexcept { if (impl.get()) impl->BeginRenderPass(render_pass, frame_buffer_index, offset); return *this; }
    auto &EndRenderPass() noexcept { if (impl.get()) impl->EndRenderPass(); return *this; }
    auto &NextSubpass() noexcept { if (impl.get()) impl->NextSubpass(); return *this; }
    auto &DrawIndexed(const uint32_t index_count, const uint32_t first_index, const uint32_t vertex_offset = 0, const uint32_t instance_count = 1, const uint32_t first_instance = 0) noexcept { if (impl.get()) impl->DrawIndexed(index_count, first_index, vertex_offset, instance_count, first_instance); return *this; }
    auto &Draw(const uint32_t vertex_count, const uint32_t first_vertex = 0, const uint32_t instance_count = 1, const uint32_t first_instance = 0) noexcept { if (impl.get()) impl->Draw(vertex_count, first_vertex, instance_count, first_instance); return *this; }
    auto &Dispatch(const uint32_t x, const uint32_t y, const uint32_t z) noexcept { if (impl.get()) impl->Dispatch(x, y, z); return *this; }
    auto &BindPipeline(const VkPipeline pipeline, const VkPipelineBindPoint bind_point) noexcept { if (impl.get()) impl->BindPipeline(pipeline, bind_point); return *this; }
    auto &BindDescriptorSets(const VkPipelineLayout pipeline_layout, const VkPipelineBindPoint bind_point, const std::vector<VkDescriptorSet> sets, const uint32_t first_set, const std::vector<uint32_t> dynamic_offeset) noexcept { if (impl.get()) impl->BindDescriptorSets(pipeline_layout, bind_point, sets, first_set, dynamic_offeset); return *this; }
    auto &BindVertexBuffers(const std::vector<VkBuffer> buffers, const std::vector<VkDeviceSize> offsets, const uint32_t first_binding, const uint32_t binding_count) noexcept { if (impl.get()) impl->BindVertexBuffers(buffers, offsets, first_binding, binding_count); return *this; }
    auto &BindIndexBuffer(const VkBuffer buffer, const VkIndexType index_type, const VkDeviceSize offset = 0) noexcept { if (impl.get()) impl->BindIndexBuffer(buffer, index_type, offset); return *this; }
    auto &SetViewport(const std::vector<VkViewport> &viewports) noexcept { if (impl.get()) impl->SetViewport(viewports); return *this; }
    auto &SetDepthBias(const float depth_bias_constant_factor, const float depth_bias_clamp, const float depth_bias_slope_factor) noexcept { if (impl.get()) impl->SetDepthBias(depth_bias_constant_factor, depth_bias_clamp, depth_bias_slope_factor); return *this; }
    auto &ImageLayoutTransition(ImageArray &image, const size_t image_index, const VkImageLayout new_layout, const uint32_t mip_level = 0, const bool transit_all_mip_levels = true) noexcept { if (impl.get()) impl->ImageLayoutTransition(image, image_index, new_layout, mip_level, transit_all_mip_levels); return *this; }
    auto &CopyBufferToImage(const VkBuffer src, ImageArray &image, const size_t image_index, const std::vector<VkBufferImageCopy> regions) noexcept { if (impl.get()) impl->CopyBufferToImage(src, image, image_index, regions); return *this; }
  };

  void swap(CommandBuffer &lhs, CommandBuffer &rhs) noexcept;
}

#endif