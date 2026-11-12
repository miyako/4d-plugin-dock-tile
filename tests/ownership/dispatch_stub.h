// Force-included into the plugin sources by tests/ownership/run.sh.
// Replaces dispatch_async with a recorder that never runs the block, so the
// harness never touches NSApp, the Dock or the main queue.
#pragma once
#include <dispatch/dispatch.h>
#ifdef __cplusplus
extern "C"
#endif
void ownership_harness_dispatch_async(dispatch_queue_t queue, dispatch_block_t block);
#define dispatch_async ownership_harness_dispatch_async
