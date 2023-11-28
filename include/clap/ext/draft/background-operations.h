#pragma once

#include "../../plugin.h"

static const char CLAP_EXT_BACKGROUND_OPERATIONS[] = "clap.background_operations.draft/0";

/*
This extension defines a set of specific operations to be executed on its background threads to alleviate its
main thread.

This extensions

For example, loading a state or activating a plugin can require a bit of processing.
When loading a large project, with a thousand plugin instances, it is desirable to be able
to load the plugin state and activate the plugin in a background thread. This can significantly
improve loading times.



Here is how this extension works:

1. Check which operation can be performed in background

        plugin_bgop->is_supported(plugin, CLAP_EXT_STATE);

2. Inform the plugin that the host is about to start a background operation

        const bool can_start = plugin_bgop->about_to_start(plugin);

   After about_to_start() returned true, no more call can be made on the main thread by the plugin and host.
   If the background thread couldn't be created then it is allowed to jump to the step 6.
   immediately.

3. Inform the plugin that we've setup the background thread and are about to start (on the
   background thread)

        plugin_bgop->started(plugin);

4. Perform the background operations, the host call any method from supported extensions which are
   marked as [main-thread] on the [bgop-thread].

        plugin_state->load(...)
        ...
        plugin->activate(...)

5. Inform the plugin that we're about to quit the background thread

        plugin_bgop->about_to_finish(plugin);

6. Inform the plugin that we're done and back onto the main thread

        plugin_bgop->finished(plugin);

*/

#ifdef __cplusplus
extern "C" {
#endif

enum clap_background_operation_type {
   CLAP_BACKGROUND_OPERATION_PREPARE_ACTIVATE = 1,
   CLAP_BACKGROUND_OPERATION_PRELOAD_STATE = 2,
   CLAP_BACKGROUND_OPERATION_SAVE_STATE = 3,
};
typedef uint32_t clap_background_operation_type_t;

typedef struct clap_background_operation_prepare_activate_data {
      double                    sample_rate;
      uint32_t                  min_frames_count;
      uint32_t                  max_frames_count;
} clap_background_operation_prepare_activate_data_t;

// Implementing this extension implies that plugin->activate() can be called from a [bgop-thread].
typedef struct clap_plugin_background_operations {
   // Returns true if an extension can be used in [bgop-thread].
   // [thread-safe]
   bool(CLAP_ABI *is_supported)(const clap_plugin_t              *plugin,
                                clap_background_operation_type_t  operation_type);

   // The host is about to start a background operation.
   // Return false to cancel the background operation.
   // If the plugin returns true then no further call can be made on the [main-thread] by the plugin
   // and host, except `finished()`.
   //
   // [thread-safe]
   bool(CLAP_ABI *perform)(const clap_plugin_t              *plugin,
                           clap_background_operation_type_t  operation_type,
                           const void                       *data);
} clap_plugin_background_operations_t;

#ifdef __cplusplus
}
#endif