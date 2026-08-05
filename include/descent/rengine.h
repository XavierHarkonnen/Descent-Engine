/* Copyright 2025 XavierHarkonnen9 and Enlarium
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DESCENT_RENGINE_H
#define DESCENT_RENGINE_H

/*
Render Graph Engine (rengine)
*/

/*
What passes exist
What resources each pass reads/writes
Optional scheduling hints
Code to execute the pass
*/

/* API
Singleton rendering backend

Promise capabilities up front instead of failing at invocation
Initialization can fail for:
- unsupported features
- no device
- invalid config
Compile can fail for:
- graph cycle
- illegal resource usage
- exceeding declared caps
Execute can fail only for:
- runtime environment loss

Everything must be thread-safe

Note: we could even decide to make the graph static, in a system that runs the graph setup only once,
and rebuild it only if explicitly requested, if we are sure that our system will almost always need
all the declared passes.
*/

#include <descent/type/core.h>
#include <descent/type/uuid.h>

enum {
	RENGINE_SUCCESS = 0,
	RENGINE_ERROR_INVALID,
	RENGINE_ERROR_NULL,
	RENGINE_ERROR_INIT,
	RENGINE_ERROR_MEMORY,
	RENGINE_ERROR_THREAD,
	RENGINE_ERROR_BACKEND,
	RENGINE_ERROR_UNSUPPORTED
};

#define INPUT_BUTTON_NONE ((input_button) NULL)
#define INPUT_CURSOR_NONE ((input_cursor) NULL)
#define INPUT_SCROLL_NONE ((input_scroll) NULL)
#define INPUT_TABLE_NONE U8_MAX

enum {
	INPUT_ACTION_RELEASE = 0,
	INPUT_ACTION_PRESS,
	INPUT_ACTION_REPEAT
};

enum {
	INPUT_MOUSE_1,
	INPUT_MOUSE_LEFT = INPUT_MOUSE_1,
	INPUT_MOUSE_2,
	INPUT_MOUSE_RIGHT = INPUT_MOUSE_2,
	INPUT_MOUSE_3,
	INPUT_MOUSE_MIDDLE = INPUT_MOUSE_3,
	INPUT_MOUSE_4,
	INPUT_MOUSE_5,
	INPUT_MOUSE_6,
	INPUT_MOUSE_7,
	INPUT_MOUSE_8,
	INPUT_KEY_SPACE,
	INPUT_KEY_APOSTROPHE,
	INPUT_KEY_COMMA,
	INPUT_KEY_MINUS,
	INPUT_KEY_PERIOD,
	INPUT_KEY_SLASH,
	INPUT_KEY_0,
	INPUT_KEY_1,
	INPUT_KEY_2,
	INPUT_KEY_3,
	INPUT_KEY_4,
	INPUT_KEY_5,
	INPUT_KEY_6,
	INPUT_KEY_7,
	INPUT_KEY_8,
	INPUT_KEY_9,
	INPUT_KEY_SEMICOLON,
	INPUT_KEY_EQUAL,
	INPUT_KEY_A,
	INPUT_KEY_B,
	INPUT_KEY_C,
	INPUT_KEY_D,
	INPUT_KEY_E,
	INPUT_KEY_F,
	INPUT_KEY_G,
	INPUT_KEY_H,
	INPUT_KEY_I,
	INPUT_KEY_J,
	INPUT_KEY_K,
	INPUT_KEY_L,
	INPUT_KEY_M,
	INPUT_KEY_N,
	INPUT_KEY_O,
	INPUT_KEY_P,
	INPUT_KEY_Q,
	INPUT_KEY_R,
	INPUT_KEY_S,
	INPUT_KEY_T,
	INPUT_KEY_U,
	INPUT_KEY_V,
	INPUT_KEY_W,
	INPUT_KEY_X,
	INPUT_KEY_Y,
	INPUT_KEY_Z,
	INPUT_KEY_LEFT_BRACKET,
	INPUT_KEY_BACKSLASH,
	INPUT_KEY_RIGHT_BRACKET,
	INPUT_KEY_GRAVE_ACCENT,
	INPUT_KEY_ESCAPE,
	INPUT_KEY_ENTER,
	INPUT_KEY_TAB,
	INPUT_KEY_BACKSPACE,
	INPUT_KEY_INSERT,
	INPUT_KEY_DELETE,
	INPUT_KEY_RIGHT,
	INPUT_KEY_LEFT,
	INPUT_KEY_DOWN,
	INPUT_KEY_UP,
	INPUT_KEY_PAGE_UP,
	INPUT_KEY_PAGE_DOWN,
	INPUT_KEY_HOME,
	INPUT_KEY_END,
	INPUT_KEY_CAPS_LOCK,
	INPUT_KEY_SCROLL_LOCK,
	INPUT_KEY_NUM_LOCK,
	INPUT_KEY_PRINT_SCREEN,
	INPUT_KEY_PAUSE,
	INPUT_KEY_F1,
	INPUT_KEY_F2,
	INPUT_KEY_F3,
	INPUT_KEY_F4,
	INPUT_KEY_F5,
	INPUT_KEY_F6,
	INPUT_KEY_F7,
	INPUT_KEY_F8,
	INPUT_KEY_F9,
	INPUT_KEY_F10,
	INPUT_KEY_F11,
	INPUT_KEY_F12,
	INPUT_KEY_F13,
	INPUT_KEY_F14,
	INPUT_KEY_F15,
	INPUT_KEY_F16,
	INPUT_KEY_F17,
	INPUT_KEY_F18,
	INPUT_KEY_F19,
	INPUT_KEY_F20,
	INPUT_KEY_F21,
	INPUT_KEY_F22,
	INPUT_KEY_F23,
	INPUT_KEY_F24,
	INPUT_KEY_F25,
	INPUT_KEY_KP_0,
	INPUT_KEY_KP_1,
	INPUT_KEY_KP_2,
	INPUT_KEY_KP_3,
	INPUT_KEY_KP_4,
	INPUT_KEY_KP_5,
	INPUT_KEY_KP_6,
	INPUT_KEY_KP_7,
	INPUT_KEY_KP_8,
	INPUT_KEY_KP_9,
	INPUT_KEY_KP_DECIMAL,
	INPUT_KEY_KP_DIVIDE,
	INPUT_KEY_KP_MULTIPLY,
	INPUT_KEY_KP_SUBTRACT,
	INPUT_KEY_KP_ADD,
	INPUT_KEY_KP_ENTER,
	INPUT_KEY_KP_EQUAL,
	INPUT_KEY_LEFT_SHIFT,
	INPUT_KEY_LEFT_CONTROL,
	INPUT_KEY_LEFT_ALT,
	INPUT_KEY_RIGHT_SHIFT,
	INPUT_KEY_RIGHT_CONTROL,
	INPUT_KEY_RIGHT_ALT,
	INPUT_KEY_MENU,
	INPUT_COUNT,
	INPUT_UNKNOWN = INPUT_COUNT,
};

typedef void (*input_button)(void *context, u8 action);
typedef void (*input_cursor)(void *context, double x, double y);
typedef void (*input_scroll)(void *context, double x, double y);

// Binding NULL will unset a binding
// Returns the previous button binding
// Binding an index that doesn't exist returns NULL
input_button rengine_input_table_bind_button(u8 index, u8 input, input_button function);

// Binding NULL will unset a binding
// Returns the previous cursor binding
// Binding an index that doesn't exist returns NULL
input_cursor rengine_input_table_bind_cursor(u8 index, input_cursor function);

// Binding NULL will unset a binding
// Returns the previous scroll binding
// Binding an index that doesn't exist returns NULL
input_scroll rengine_input_table_bind_scroll(u8 index, input_scroll function);

void *rengine_input_table_set_userdata(u8 index, void *userdata);

// Returns the previous selected table
// Selecting an index that doesn't exist (preferably INPUT_TABLE_NONE) unsets existing input handling
u8 rengine_select_input_table(u8 index);

// TODO: Set cursor

// TODO: Set cursor mode













typedef u32 rimage;
typedef u32 rbuffer;
typedef u32 rpass;

// u64 debug; // Bitmask of debug features
#define DEBUG_VALIDATION (1 << 0)

struct rengine_create_info {
	u64 debug;

	const char* name;
	u32 version;

	i32 height;
	i32 width;

	u8 input_table_count;
	
	const uuid* preferred_device;
};

struct rengine_capabilities {
	bool async_compute;
	bool raytracing;
	bool mesh_shaders;
	bool bindless;
	bool timeline_sync;

	u32 max_color_targets;
	u64 max_buffer_size;
};

// Converts a rengine error code to a string
const char *rengine_result_string(u64 result);

// Initializes GLFW and Vulkan
// Selects the best physical device
// Creates windows
// Reentrancy-safe
// Main thread only
u64 rengine_initialize(const struct rengine_create_info *create_info);
// Reentrancy-safe
// Main thread only
u64 rengine_sys_terminate(void);
struct rengine_capabilities *rengine_capabilities(void);

/* 
// rengine_import_external_*(...)
rimage rengine_import_swapchain(window_id);
rbuffer rengine_import_external_buffer(native_handle);

// rengine_create_transient_*(...)
rimage rengine_create_transient_image(desc);
rbuffer rengine_create_transient_buffer(desc);

rpass rengine_add_pass(void);

void rengine_compile(void);
void rengine_execute(void);

void rengine_present(void);

// TODO: Rengine needs to take care of input handling, too.

*/

#endif