#pragma once
#include <vulkan/vulkan.h>
#include "EyeProjection.h"
#include "../EternalDlssAbi.h"
#include <unordered_set>
namespace argent::sfs {
// Explicit Eternal classification; no inherited DOOM hashes or scale.
struct Configuration {std::unordered_set<uint64_t> projectionShaders,stereoComputeShaders,broadcastComputeShaders,screenUiShaders; bool imageComputeStereo{},projectDepthOnly{},eternalVolumes{},eternalLightGrids{},eternalVk3d{}; uint32_t views{kViews};};
struct FramePose {uint64_t serial{};XrTime displayTime{};std::array<XrView,2> views{};bool quadView{};XrPosef quadPose{};bool quadHeadLocked{};bool headPositionTracked{},recenterRequested{};XrPosef predictedHead{};bool stereoQuad{};};
struct EyeImage {VkImage image{};VkExtent2D extent{};VkFormat format{};VkImageLayout layout{};XrPosef pose{};XrFovf fov{};uint32_t layer{};uint64_t serial{};};
struct StereoFrame {FramePose pose;uint64_t generation{};std::array<EyeImage,2> eyes;};
bool sourceRingRequested();
bool configureSourceRing(VkDevice,PFN_vkGetDeviceProcAddr,const VkPhysicalDeviceMemoryProperties&,VkQueue,void(*)(),void(*)());
bool sourceRingActive(VkDevice);
bool sourceSwapchain(VkDevice,VkSwapchainKHR);
VkResult createSourceSwapchain(VkDevice,const VkSwapchainCreateInfoKHR&,VkSwapchainKHR*);
VkResult sourceImages(VkDevice,VkSwapchainKHR,uint32_t*,VkImage*);
VkResult acquireSource(VkDevice,VkSwapchainKHR,uint64_t,VkSemaphore,VkFence,uint32_t*);
VkResult presentSource(VkDevice,VkQueue,const VkPresentInfoKHR&,bool);
void destroySourceSwapchain(VkDevice,VkSwapchainKHR);
VkImageLayout sourceLayout(VkDevice,VkImage,VkImageLayout);
bool nativeProbeEnabled();
bool initialize(VkDevice,VkPhysicalDevice,PFN_vkGetDeviceProcAddr,const VkPhysicalDeviceMemoryProperties&,const Configuration&);
void enableGpuCheckpoints(VkDevice,PFN_vkGetDeviceProcAddr);
void reportGpuCheckpoints(VkDevice,VkQueue) noexcept; // Only after VK_ERROR_DEVICE_LOST.
void enableGpuFaultReport(VkDevice,PFN_vkGetDeviceProcAddr);
void enableWaterRobustness(VkDevice);
void markXrCheckpoint(VkDevice,VkCommandBuffer,const char*);
void configurePerformanceGpu(VkDevice,float,uint32_t,const VkQueueFamilyProperties*);
// Called only after a successful native submit, under the existing queue lock.
void performanceSubmitted(VkDevice,VkQueue,uint32_t,const VkCommandBuffer*);
void shutdown(VkDevice);
void waterCaptureSubmitted(VkDevice,VkQueue,uint32_t,const VkCommandBuffer*);
PFN_vkVoidFunction wrapProc(VkDevice d,const char*,PFN_vkVoidFunction);
void swapchainImages(VkDevice,VkSwapchainKHR,uint32_t,const VkImage*);
void swapchainDestroyed(VkDevice,VkSwapchainKHR);
bool vrEnabled();
void prepare(VkDevice,const FramePose&,const EyeUniforms&);
void copyCompleted(VkDevice);
// Caller holds the device queue lock through retirement and uniform upload.
VkResult beginFrame(VkDevice,VkSwapchainKHR=VK_NULL_HANDLE,uint32_t imageIndex=0);
bool pair(VkDevice,VkImage,VkExtent2D,VkFormat,StereoFrame&);
// view < viewCount: 0/1 are the eyes, kScopeView is the scope (only when viewCount()==kViews).
bool eyeAttachmentView(VkDevice,VkImageView,uint32_t,VkImageView&);
uint32_t viewCount(VkDevice);
// Fills one entry per view (viewCount); entries past viewCount are untouched.
bool dlssEyeResources(VkCommandBuffer,const std::array<const dlss::Resource*,30>&,std::array<dlss::EyeParameters,kViews>&,FramePose&);
uint32_t viewCount(VkCommandBuffer);
}
