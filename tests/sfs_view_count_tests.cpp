#include "../src/sfs/StereoResources.h"
#include "../src/sfs/EyeProjection.h"
#include <iostream>
#include <stdexcept>
#include <string>
using namespace kharvox::sfs;
void check(bool b,const char* what){if(!b)throw std::runtime_error(what);}
int main(){try{
    check(kViews==3&&kScopeView==2&&kEyeViews==2,"view constants");
    check(viewMask(3)==7&&viewMask(2)==3&&kEyeCorrelationMask==3,"view masks");
    check(supportedViews(4,2048)==3&&supportedViews(3,3)==3,"scope supported");
    check(supportedViews(2,2048)==2&&supportedViews(16,2)==2,"scope fallback");
    check(validViews(2)&&validViews(3)&&!validViews(1)&&!validViews(4),"valid views");

    uint32_t depth=0;
    check(dispatchDepth(12,true,64,depth,3)&&depth==36,"three-view dispatch");
    check(dispatchDepth(12,true,64,depth,2)&&depth==24,"two-view dispatch");
    check(dispatchDepth(12,false,64,depth,3)&&depth==12,"mono dispatch");
    check(!dispatchDepth(22,true,64,depth,3)&&depth==12,"three-view limit");
    check(dispatchDepth(21,true,63,depth,3)&&depth==63,"three-view exact limit");
    check(!dispatchDepth(0x55555556u,true,UINT32_MAX,depth,3),"three-view overflow");
    check(!dispatchDepth(1,true,64,depth,4)&&depth==63,"invalid view count");

    uint32_t slot=0;
    check(querySlot(5,3,slot)&&slot==15,"three-view query slot");
    check(querySlot(5,2,slot)&&slot==10,"two-view query slot");
    check(!querySlot(0x55555556u,3,slot)&&slot==10,"query slot overflow");
    check(!querySlot(1,1,slot),"query slot invalid views");

    const uint64_t ready[6]{10,1,20,1,999,1};
    auto sum=occlusionSum(ready,3);check(sum.samples==1029&&sum.available==1,"occlusion sums every view");
    const uint64_t scopePending[6]{10,1,20,1,0,0};
    sum=occlusionSum(scopePending,3);check(sum.available==0,"occlusion waits for scope slot");
    sum=occlusionSum(ready,2);check(sum.samples==30&&sum.available==1,"two-view occlusion");

    VkSubpassDescription subpass{};VkRenderPassCreateInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};rp.subpassCount=1;rp.pSubpasses=&subpass;
    RenderPassPlan three(rp,true,3);auto mv=static_cast<const VkRenderPassMultiviewCreateInfo*>(three.info().pNext);
    check(three.valid()&&mv->pViewMasks[0]==7&&mv->pCorrelationMasks[0]==3,"three-view pass");
    RenderPassPlan two(rp,true,2);mv=static_cast<const VkRenderPassMultiviewCreateInfo*>(two.info().pNext);
    check(two.valid()&&mv->pViewMasks[0]==3&&mv->pCorrelationMasks[0]==3,"two-view pass");
    RenderPassPlan bad(rp,true,4);check(!bad.valid(),"invalid view pass");

    VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};info.imageType=VK_IMAGE_TYPE_2D;info.arrayLayers=1;info.usage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    check(imageInfo(info,2).arrayLayers==2&&imageInfo(info).arrayLayers==3,"image layers");
    Images images;images.setViews(2);check(images.views()==2,"two-view registry");
    images.setViews(7);check(images.views()==2,"invalid registry views fall back");
    images.setViews(3);auto image=reinterpret_cast<VkImage>(uintptr_t(7));images.track(image,3);
    VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};view.image=image;view.viewType=VK_IMAGE_VIEW_TYPE_2D;view.subresourceRange.layerCount=1;
    const auto arrayed=images.viewInfo(view);
    check(arrayed.viewType==VK_IMAGE_VIEW_TYPE_2D_ARRAY&&arrayed.subresourceRange.layerCount==3,"three-layer view");

    using namespace argent::sfs;
    check(sizeof(EyeUniforms)==448,"std140 uniform size");
    auto u=identityUniforms();check(u.clipFromCenter[kScopeView]==identity()&&u.screenClip[kScopeView]==identity(),"identity scope view");
    u.clipFromCenter[0][12]=0.25f;u.eyeTranslation[0][0]=-1.5f;u.screenClip[0][13]=0.5f;u.clipFromCenter[1][12]=-0.25f;
    mirrorScopeView(u);
    check(u.clipFromCenter[kScopeView]==u.clipFromCenter[0]&&u.eyeTranslation[kScopeView]==u.eyeTranslation[0]&&u.screenClip[kScopeView]==u.screenClip[0],"scope mirrors left eye");
    check(u.clipFromCenter[1][12]==-0.25f,"right eye untouched");
    std::cout<<"SFS view count, dispatch depth, query slot and scope uniform contracts passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
