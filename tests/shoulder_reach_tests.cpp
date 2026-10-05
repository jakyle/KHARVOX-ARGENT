#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "../src/openxr/ShoulderChainsaw.h"
#include <cstdio>
using argent::input::ShoulderChainsaw;
static int failures=0;
static void check(bool ok,const char* what){if(!ok){std::printf("FAIL: %s\n",what);++failures;}}
int main(){
 const XrPosef head{{0,0,0,1},{0,1.7f,0}};
 const XrPosef front{{0,0,0,1},{.25f,1.4f,-.3f}};
 const XrPosef shoulder{{0,0,0,1},{.25f,1.5f,.2f}};
 {
  ShoulderChainsaw s;
  s.update(head,front,true,true,false,false,1000);
  s.update(head,front,true,true,true,false,1016);
  check(s.update(head,shoulder,true,true,true,false,1033),"squeeze started during the reach did not fire on arrival");
 }
 {
  ShoulderChainsaw s;
  s.update(head,front,true,true,false,false,1000);
  s.update(head,front,true,true,true,false,1016);
  check(!s.update(head,shoulder,true,true,true,false,1400),"grip held long before the reach fired on arrival");
 }
 {
  ShoulderChainsaw s;
  s.update(head,front,true,true,false,false,1000);
  s.update(head,front,true,true,true,false,1016);
  s.update(head,shoulder,true,true,true,false,1033);
  s.update(head,front,true,true,true,false,1150);
  check(!s.update(head,shoulder,true,true,true,false,1200),"re-entering with the same held grip fired twice");
 }
 {
  ShoulderChainsaw s;
  s.update(head,front,true,true,false,false,1000);
  s.update(head,front,true,false,true,false,1016);
  check(!s.update(head,shoulder,true,true,true,false,1033),"grip pressed while disabled fired after re-enable");
 }
 std::printf(failures?"%d failure(s)\n":"all shoulder reach checks passed\n",failures);
 return failures?1:0;
}
