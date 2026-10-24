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

// Public API

// Implementation

namespace descent {



File::File(const Directory &parent, const char *name) {
	if (!parent || !validate_name(name))
		return;

	fd = openat(parent.fd, name, O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW);
}

}

// Implementation

[[gnu::constructor]]
static void roots_construct_elements(void) {
	const char *home_path = getenv("HOME");
	if (!home_path)
		sys_fatal("Could not discover home path");

	if (home_path[0] != '/')
		sys_fatal("Home path cannot be relative path");

	i32 home = open(home_path, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (home < 0)
		sys_fatal("Could not open home directory");

	roots.config = roots_get_element("granbarrow", "XDG_CONFIG_HOME", home, ".config");
	if (roots.config < 0)
		sys_fatal("Could not open config directory");

	roots.data = roots_get_element("granbarrow", "XDG_DATA_HOME", home, ".local/share");
	if (roots.data < 0)
		sys_fatal("Could not open data directory");

	roots.state = roots_get_element("granbarrow", "XDG_STATE_HOME", home, ".local/state");
	if (roots.state < 0)
		sys_fatal("Could not open state directory");

	roots.cache = roots_get_element("granbarrow", "XDG_CACHE_HOME", home, ".cache");
	if (roots.cache < 0)
		sys_fatal("Could not open cache directory");

	close(home);
}


[[gnu::destructor]]
static void roots_destroy(void) {
	close(roots.base);
	close(roots.config);
	close(roots.data);
	close(roots.state);
	close(roots.cache);
}
