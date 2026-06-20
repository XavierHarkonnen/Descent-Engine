#include "input.h"

#include <GLFW/glfw3.h>
#include <string.h>
#include <stdlib.h>

#include <descent/atomic.h>
#include <descent/rengine.h>
#include <descent/types/core.h>

#include "context.h"


struct input_table {
	input_button buttons[INPUT_COUNT];
	input_cursor cursor;
	input_scroll scroll;
	u64 _reserved;
	void *userdata;
} __attribute__((aligned(64)));

_Static_assert(sizeof(struct input_table) == 1024, "Input table must be 1024 bytes");


#define INPUT_LOOKUP_TABLE_LAST (GLFW_KEY_LAST)
#define INPUT_LOOKUP_TABLE_SIZE (INPUT_LOOKUP_TABLE_LAST + 1)

// GLFW's mouse and key codes do not overlap, so we can place them in the same table
static u8 input_lookup[INPUT_LOOKUP_TABLE_SIZE];


static void initialize_input_lookup(void) {
	memset(input_lookup, INPUT_UNKNOWN, sizeof(input_lookup));
	input_lookup[GLFW_MOUSE_BUTTON_1]    = INPUT_MOUSE_1;
	input_lookup[GLFW_MOUSE_BUTTON_2]    = INPUT_MOUSE_2;
	input_lookup[GLFW_MOUSE_BUTTON_3]    = INPUT_MOUSE_3;
	input_lookup[GLFW_MOUSE_BUTTON_4]    = INPUT_MOUSE_4;
	input_lookup[GLFW_MOUSE_BUTTON_5]    = INPUT_MOUSE_5;
	input_lookup[GLFW_MOUSE_BUTTON_6]    = INPUT_MOUSE_6;
	input_lookup[GLFW_MOUSE_BUTTON_7]    = INPUT_MOUSE_7;
	input_lookup[GLFW_MOUSE_BUTTON_8]    = INPUT_MOUSE_8;
	input_lookup[GLFW_KEY_SPACE]         = INPUT_KEY_SPACE;
	input_lookup[GLFW_KEY_APOSTROPHE]    = INPUT_KEY_APOSTROPHE;
	input_lookup[GLFW_KEY_COMMA]         = INPUT_KEY_COMMA;
	input_lookup[GLFW_KEY_MINUS]         = INPUT_KEY_MINUS;
	input_lookup[GLFW_KEY_PERIOD]        = INPUT_KEY_PERIOD;
	input_lookup[GLFW_KEY_SLASH]         = INPUT_KEY_SLASH;
	input_lookup[GLFW_KEY_0]             = INPUT_KEY_0;
	input_lookup[GLFW_KEY_1]             = INPUT_KEY_1;
	input_lookup[GLFW_KEY_2]             = INPUT_KEY_2;
	input_lookup[GLFW_KEY_3]             = INPUT_KEY_3;
	input_lookup[GLFW_KEY_4]             = INPUT_KEY_4;
	input_lookup[GLFW_KEY_5]             = INPUT_KEY_5;
	input_lookup[GLFW_KEY_6]             = INPUT_KEY_6;
	input_lookup[GLFW_KEY_7]             = INPUT_KEY_7;
	input_lookup[GLFW_KEY_8]             = INPUT_KEY_8;
	input_lookup[GLFW_KEY_9]             = INPUT_KEY_9;
	input_lookup[GLFW_KEY_SEMICOLON]     = INPUT_KEY_SEMICOLON;
	input_lookup[GLFW_KEY_EQUAL]         = INPUT_KEY_EQUAL;
	input_lookup[GLFW_KEY_A]             = INPUT_KEY_A;
	input_lookup[GLFW_KEY_B]             = INPUT_KEY_B;
	input_lookup[GLFW_KEY_C]             = INPUT_KEY_C;
	input_lookup[GLFW_KEY_D]             = INPUT_KEY_D;
	input_lookup[GLFW_KEY_E]             = INPUT_KEY_E;
	input_lookup[GLFW_KEY_F]             = INPUT_KEY_F;
	input_lookup[GLFW_KEY_G]             = INPUT_KEY_G;
	input_lookup[GLFW_KEY_H]             = INPUT_KEY_H;
	input_lookup[GLFW_KEY_I]             = INPUT_KEY_I;
	input_lookup[GLFW_KEY_J]             = INPUT_KEY_J;
	input_lookup[GLFW_KEY_K]             = INPUT_KEY_K;
	input_lookup[GLFW_KEY_L]             = INPUT_KEY_L;
	input_lookup[GLFW_KEY_M]             = INPUT_KEY_M;
	input_lookup[GLFW_KEY_N]             = INPUT_KEY_N;
	input_lookup[GLFW_KEY_O]             = INPUT_KEY_O;
	input_lookup[GLFW_KEY_P]             = INPUT_KEY_P;
	input_lookup[GLFW_KEY_Q]             = INPUT_KEY_Q;
	input_lookup[GLFW_KEY_R]             = INPUT_KEY_R;
	input_lookup[GLFW_KEY_S]             = INPUT_KEY_S;
	input_lookup[GLFW_KEY_T]             = INPUT_KEY_T;
	input_lookup[GLFW_KEY_U]             = INPUT_KEY_U;
	input_lookup[GLFW_KEY_V]             = INPUT_KEY_V;
	input_lookup[GLFW_KEY_W]             = INPUT_KEY_W;
	input_lookup[GLFW_KEY_X]             = INPUT_KEY_X;
	input_lookup[GLFW_KEY_Y]             = INPUT_KEY_Y;
	input_lookup[GLFW_KEY_Z]             = INPUT_KEY_Z;
	input_lookup[GLFW_KEY_LEFT_BRACKET]  = INPUT_KEY_LEFT_BRACKET;
	input_lookup[GLFW_KEY_BACKSLASH]     = INPUT_KEY_BACKSLASH;
	input_lookup[GLFW_KEY_RIGHT_BRACKET] = INPUT_KEY_RIGHT_BRACKET;
	input_lookup[GLFW_KEY_GRAVE_ACCENT]  = INPUT_KEY_GRAVE_ACCENT;
	input_lookup[GLFW_KEY_ESCAPE]        = INPUT_KEY_ESCAPE;
	input_lookup[GLFW_KEY_ENTER]         = INPUT_KEY_ENTER;
	input_lookup[GLFW_KEY_TAB]           = INPUT_KEY_TAB;
	input_lookup[GLFW_KEY_BACKSPACE]     = INPUT_KEY_BACKSPACE;
	input_lookup[GLFW_KEY_INSERT]        = INPUT_KEY_INSERT;
	input_lookup[GLFW_KEY_DELETE]        = INPUT_KEY_DELETE;
	input_lookup[GLFW_KEY_RIGHT]         = INPUT_KEY_RIGHT;
	input_lookup[GLFW_KEY_LEFT]          = INPUT_KEY_LEFT;
	input_lookup[GLFW_KEY_DOWN]          = INPUT_KEY_DOWN;
	input_lookup[GLFW_KEY_UP]            = INPUT_KEY_UP;
	input_lookup[GLFW_KEY_PAGE_UP]       = INPUT_KEY_PAGE_UP;
	input_lookup[GLFW_KEY_PAGE_DOWN]     = INPUT_KEY_PAGE_DOWN;
	input_lookup[GLFW_KEY_HOME]          = INPUT_KEY_HOME;
	input_lookup[GLFW_KEY_END]           = INPUT_KEY_END;
	input_lookup[GLFW_KEY_CAPS_LOCK]     = INPUT_KEY_CAPS_LOCK;
	input_lookup[GLFW_KEY_SCROLL_LOCK]   = INPUT_KEY_SCROLL_LOCK;
	input_lookup[GLFW_KEY_NUM_LOCK]      = INPUT_KEY_NUM_LOCK;
	input_lookup[GLFW_KEY_PRINT_SCREEN]  = INPUT_KEY_PRINT_SCREEN;
	input_lookup[GLFW_KEY_PAUSE]         = INPUT_KEY_PAUSE;
	input_lookup[GLFW_KEY_F1]            = INPUT_KEY_F1;
	input_lookup[GLFW_KEY_F2]            = INPUT_KEY_F2;
	input_lookup[GLFW_KEY_F3]            = INPUT_KEY_F3;
	input_lookup[GLFW_KEY_F4]            = INPUT_KEY_F4;
	input_lookup[GLFW_KEY_F5]            = INPUT_KEY_F5;
	input_lookup[GLFW_KEY_F6]            = INPUT_KEY_F6;
	input_lookup[GLFW_KEY_F7]            = INPUT_KEY_F7;
	input_lookup[GLFW_KEY_F8]            = INPUT_KEY_F8;
	input_lookup[GLFW_KEY_F9]            = INPUT_KEY_F9;
	input_lookup[GLFW_KEY_F10]           = INPUT_KEY_F10;
	input_lookup[GLFW_KEY_F11]           = INPUT_KEY_F11;
	input_lookup[GLFW_KEY_F12]           = INPUT_KEY_F12;
	input_lookup[GLFW_KEY_F13]           = INPUT_KEY_F13;
	input_lookup[GLFW_KEY_F14]           = INPUT_KEY_F14;
	input_lookup[GLFW_KEY_F15]           = INPUT_KEY_F15;
	input_lookup[GLFW_KEY_F16]           = INPUT_KEY_F16;
	input_lookup[GLFW_KEY_F17]           = INPUT_KEY_F17;
	input_lookup[GLFW_KEY_F18]           = INPUT_KEY_F18;
	input_lookup[GLFW_KEY_F19]           = INPUT_KEY_F19;
	input_lookup[GLFW_KEY_F20]           = INPUT_KEY_F20;
	input_lookup[GLFW_KEY_F21]           = INPUT_KEY_F21;
	input_lookup[GLFW_KEY_F22]           = INPUT_KEY_F22;
	input_lookup[GLFW_KEY_F23]           = INPUT_KEY_F23;
	input_lookup[GLFW_KEY_F24]           = INPUT_KEY_F24;
	input_lookup[GLFW_KEY_F25]           = INPUT_KEY_F25;
	input_lookup[GLFW_KEY_KP_0]          = INPUT_KEY_KP_0;
	input_lookup[GLFW_KEY_KP_1]          = INPUT_KEY_KP_1;
	input_lookup[GLFW_KEY_KP_2]          = INPUT_KEY_KP_2;
	input_lookup[GLFW_KEY_KP_3]          = INPUT_KEY_KP_3;
	input_lookup[GLFW_KEY_KP_4]          = INPUT_KEY_KP_4;
	input_lookup[GLFW_KEY_KP_5]          = INPUT_KEY_KP_5;
	input_lookup[GLFW_KEY_KP_6]          = INPUT_KEY_KP_6;
	input_lookup[GLFW_KEY_KP_7]          = INPUT_KEY_KP_7;
	input_lookup[GLFW_KEY_KP_8]          = INPUT_KEY_KP_8;
	input_lookup[GLFW_KEY_KP_9]          = INPUT_KEY_KP_9;
	input_lookup[GLFW_KEY_KP_DECIMAL]    = INPUT_KEY_KP_DECIMAL;
	input_lookup[GLFW_KEY_KP_DIVIDE]     = INPUT_KEY_KP_DIVIDE;
	input_lookup[GLFW_KEY_KP_MULTIPLY]   = INPUT_KEY_KP_MULTIPLY;
	input_lookup[GLFW_KEY_KP_SUBTRACT]   = INPUT_KEY_KP_SUBTRACT;
	input_lookup[GLFW_KEY_KP_ADD]        = INPUT_KEY_KP_ADD;
	input_lookup[GLFW_KEY_KP_ENTER]      = INPUT_KEY_KP_ENTER;
	input_lookup[GLFW_KEY_KP_EQUAL]      = INPUT_KEY_KP_EQUAL;
	input_lookup[GLFW_KEY_LEFT_SHIFT]    = INPUT_KEY_LEFT_SHIFT;
	input_lookup[GLFW_KEY_LEFT_CONTROL]  = INPUT_KEY_LEFT_CONTROL;
	input_lookup[GLFW_KEY_LEFT_ALT]      = INPUT_KEY_LEFT_ALT;
	input_lookup[GLFW_KEY_RIGHT_SHIFT]   = INPUT_KEY_RIGHT_SHIFT;
	input_lookup[GLFW_KEY_RIGHT_CONTROL] = INPUT_KEY_RIGHT_CONTROL;
	input_lookup[GLFW_KEY_RIGHT_ALT]     = INPUT_KEY_RIGHT_ALT;
	input_lookup[GLFW_KEY_MENU]          = INPUT_KEY_MENU;
}


static void input_button_none(void *context, u8 action)          { (void) context; (void) action; }
static void input_cursor_none(void *context, double x, double y) { (void) context; (void) x; (void) y; }
static void input_scroll_none(void *context, double x, double y) { (void) context; (void) x; (void) y; }


static inline bool is_valid_GLFW_input(int input) {
	return input >= 0 && input <= GLFW_KEY_LAST;
}


static inline u8 index_from_GLFW_input(int input) {
	if (is_valid_GLFW_input(input)) return input_lookup[input];
	return INPUT_UNKNOWN;
}


static inline bool is_valid_GLFW_action(int action) {
	return action == GLFW_RELEASE || action == GLFW_PRESS || action == GLFW_REPEAT;
}


static inline u8 action_from_GLFW_action(int action) {
	return (u8) action;
}


static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	(void) window;
	(void) mods;
	(void) scancode;

	u8 index = index_from_GLFW_input(key);
	if (index == INPUT_UNKNOWN) return;

	struct input_table *input_table = atomic_load(&rengine_context()->input_tables.active);
	input_button function = atomic_load(&input_table->buttons[index]);
	void *userdata = atomic_load(&input_table->userdata);

	function(userdata, action_from_GLFW_action(action));
}


static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	(void) window;
	(void) mods;

	u8 index = index_from_GLFW_input(button);
	if (index == INPUT_UNKNOWN) return;

	struct input_table *input_table = atomic_load(&rengine_context()->input_tables.active);
	input_button function = atomic_load(&input_table->buttons[index]);
	void *userdata = atomic_load(&input_table->userdata);

	function(userdata, action_from_GLFW_action(action));
}


static void cursor_callback(GLFWwindow *window, double x, double y) {
	(void) window;

	struct input_table *input_table = atomic_load(&rengine_context()->input_tables.active);
	input_cursor function = atomic_load(&input_table->cursor);
	void *userdata = atomic_load(&input_table->userdata);

	function(userdata, x, y);
}


static void scroll_callback(GLFWwindow *window, double x, double y) {
	(void) window;

	struct input_table *input_table = atomic_load(&rengine_context()->input_tables.active);
	input_scroll function = atomic_load(&input_table->scroll);
	void *userdata = atomic_load(&input_table->userdata);

	function(userdata, x, y);
}


input_button rengine_input_table_bind_button(u8 index, u8 input, input_button function) {
	struct rengine_context *context = rengine_context();

	if (!atomic_load(&context->initialized))
		return INPUT_BUTTON_NONE;

	if (input >= INPUT_COUNT || index >= context->input_tables.count)
		return INPUT_BUTTON_NONE;

	if (!function)
		function = input_button_none;

	function = atomic_exchange(&context->input_tables.tables[index].buttons[input], function);

	if (function == input_button_none)
		function = NULL;

	return function;
}


input_cursor rengine_input_table_bind_cursor(u8 index, input_cursor function) {
	struct rengine_context *context = rengine_context();

	if (!atomic_load(&context->initialized))
		return INPUT_CURSOR_NONE;

	if (index >= context->input_tables.count)
		return INPUT_CURSOR_NONE;

	if (!function)
		function = input_cursor_none;

	function = atomic_exchange(&context->input_tables.tables[index].cursor, function);

	if (function == input_cursor_none)
		function = NULL;

	return function;
}


input_scroll rengine_input_table_bind_scroll(u8 index, input_scroll function) {
	struct rengine_context *context = rengine_context();

	if (!atomic_load(&context->initialized))
		return INPUT_SCROLL_NONE;

	if (index >= context->input_tables.count)
		return INPUT_SCROLL_NONE;

	if (!function)
		function = input_scroll_none;

	function = atomic_exchange(&context->input_tables.tables[index].scroll, function);

	if (function == input_scroll_none)
		function = NULL;

	return function;
}


void *rengine_input_table_set_userdata(u8 index, void *userdata) {
	struct rengine_context *context = rengine_context();

	if (!atomic_load(&context->initialized))
		return NULL;

	if (index >= context->input_tables.count)
		return NULL;

	userdata = atomic_exchange(&context->input_tables.tables[index].userdata, userdata);

	return userdata;
}


u8 rengine_select_input_table(u8 index) {
	struct rengine_context *context = rengine_context();

	if (!atomic_load(&context->initialized))
		return INPUT_TABLE_NONE;

	// Use hidden table as for unbound input
	if (index > context->input_tables.count)
		index = context->input_tables.count;
	
	struct input_table *input_table = &context->input_tables.tables[index];

	input_table = atomic_exchange(&context->input_tables.active, input_table);
	if (input_table == &context->input_tables.tables[context->input_tables.count])
		return INPUT_TABLE_NONE;

	return (u8) (input_table - context->input_tables.tables);
}


u64 rengine_create_input_tables(struct rengine_context *context) {
	context->input_tables.count = context->info->input_table_count;

	// Create hidden table as for unbound input
	u64 true_count = (u64) context->input_tables.count + 1;

	context->input_tables.tables = malloc(true_count * sizeof(struct input_table));
	if (!context->input_tables.tables)
		return RENGINE_ERROR_MEMORY;

	for (u64 i = 0; i < true_count; ++i) {
		for (u64 j = 0; j < INPUT_COUNT; ++j)
			atomic_store(&context->input_tables.tables[i].buttons[j], input_button_none);
	
		atomic_store(&context->input_tables.tables[i].cursor, input_cursor_none);
		atomic_store(&context->input_tables.tables[i].scroll, input_scroll_none);
		atomic_store(&context->input_tables.tables[i].userdata, NULL);
	}

	// Initialize to hidden unbound table
	atomic_store(&context->input_tables.active, &context->input_tables.tables[context->input_tables.count]);

	initialize_input_lookup();
	glfwSetKeyCallback(context->window.window, key_callback);
	glfwSetMouseButtonCallback(context->window.window, mouse_button_callback);
	glfwSetCursorPosCallback(context->window.window, cursor_callback);
	glfwSetScrollCallback(context->window.window, scroll_callback);

	return RENGINE_SUCCESS;
}


void rengine_destroy_input_tables(struct rengine_context *context) {
	if (context->window.window) {
		glfwSetScrollCallback(context->window.window, NULL);
		glfwSetCursorPosCallback(context->window.window, NULL);
		glfwSetMouseButtonCallback(context->window.window, NULL);
		glfwSetKeyCallback(context->window.window, NULL);
	}

	glfwPollEvents();

	context->input_tables.count = 0;
	atomic_store(&context->input_tables.active, NULL);
	free(context->input_tables.tables);
}
