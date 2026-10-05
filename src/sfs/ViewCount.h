#pragma once
#include <cstdint>
#include <limits>

namespace kharvox::sfs {
// SFS multiview layout: views 0/1 are the eyes, view 2 is the optical scope.
constexpr uint32_t kEyeViews=2;
constexpr uint32_t kViews=3;
constexpr uint32_t kScopeView=2;
// Only the eyes are spatially correlated; the scope view is not.
constexpr uint32_t kEyeCorrelationMask=(1u<<kEyeViews)-1;
inline constexpr uint32_t viewMask(uint32_t views){return views>=32?~0u:(1u<<views)-1;}
// Runtime view count for a device: the scope view needs maxMultiviewViewCount>=3 and three array layers.
inline constexpr uint32_t supportedViews(uint32_t maxViews,uint32_t maxLayers){return maxViews>=kViews&&maxLayers>=kViews?kViews:kEyeViews;}
inline constexpr bool validViews(uint32_t views){return views==kEyeViews||views==kViews;}
// Physical query slots for a logical index: multiview writes one consecutive slot per view.
inline bool querySlot(uint32_t logical,uint32_t views,uint32_t& result){
    if(!validViews(views)||logical>std::numeric_limits<uint32_t>::max()/views)return false;
    result=logical*views;return true;
}
// Vulkan may distribute a multiview occlusion count across its per-view slots arbitrarily, so every view must be summed.
struct OcclusionSum {uint64_t samples{},available{};};
inline OcclusionSum occlusionSum(const uint64_t* raw,uint32_t views){
    OcclusionSum result{0,1};
    for(uint32_t view=0;view<views;++view){result.samples+=raw[2*view];if(!raw[2*view+1])result.available=0;}
    return result;
}
}
