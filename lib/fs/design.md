# Open-First vs Always-Mkdir: Directory Fast Path

## Syscall counts

Open first is the better fast path. With p the probability the directory already exists:

| Strategy | Exists | Missing | Expected syscalls |
| --- | --- | --- | --- |
| Open → (mkdir → open) | 1 | 3 | 3 − 2p |
| Always mkdir → open | 2 | 2 | 2 |

By count alone, open-first wins once p > 0.5. Directories are usually created once and opened many times, so p is typically close to 1: about half the cost.

## Why the syscalls don't cost the same

The count understates the gap, because open-first relies on the cheap calls.

- **O\_PATH open of an existing directory:** about the cheapest way to resolve a path. The lookup runs in RCU mode without locks, and O\_PATH skips the filesystem's ->open and the file-open LSM hook.
- **Failed open (ENOENT):** after the first miss, this hits a cached negative dentry and is also cheap.
- **mkdir on an existing entry:** takes the mount's write barrier (sb\_start\_write), then locks the parent directory's inode exclusively for the lookup, only to return EEXIST. Concurrent "always mkdir" callers in the same parent serialize on that lock; open-first does only shared lookups and scales.
- **Remote filesystems:** for an exclusive create, NFS and FUSE often skip the local lookup and send mkdir to the server. "Always mkdir" can then cost a network round trip every call, even when the directory exists.

With c\_om the cost of a failed open and c\_me the cost of an EEXIST mkdir, open-first wins when:

```latex
\frac{p}{1 - p} > \frac{c_{om}}{c_{me}}
```

Since c\_om < c\_me, the break-even point is below 50% existence. Open-first wins even if most directories are new, and by a wide margin in typical use.

## Races

Both races are harmless:

- **Another thread creates the directory between the failed open and your mkdir.** You get EEXIST, which counts as success, and the second open succeeds.
- **The directory is removed between mkdir and the second open.** You get ENOENT. Either fail, or retry the sequence a bounded number of times.

## Multi-component paths

The same reasoning applies to make\_dir. One openat2 call with the RESOLVE\_\* flags for the whole path is the fast path: 1 syscall when everything exists. On ENOENT, fall back to the per-component walk.

For a path of depth N with k missing trailing components:

| Strategy | All exist | k missing |
| --- | --- | --- |
| openat2 first, then walk | 1 | about 1 + (N − k) + 2k |
| Always mkdir each level, then open | N + 1 | N + 1 or more |

# Directory Rework

Oct 30, 2026

## Problems in the current directory.cpp

- **make\_dir does not hold the directories it walks.** It cuts the path at each `/` and calls mkdir on every prefix from the start. Each call resolves the whole prefix again, so lookups grow with the square of the depth, and a prefix can be replaced by a symlink between steps.
- **make\_open\_dir creates first and opens second**, the order the analysis above argues against.
- **One set of flags for every directory.** O\_NOFOLLOW applies to roots too, so a root that is a symlink (for example a dotfile-managed `~/.config`) fails.
- **Errors are lost.** The make functions return nothing, so the reason the final open failed is unknown.
- **Leftovers:** the `descent` namespace instead of `descent::fs`, the removed `fd` member, and a hand-written move assignment and destructor that Handle now provides.

## Structure

| File | Contents |
| --- | --- |
| `sys.hpp` | Internal. System calls retried on EINTR: openat, mkdirat, openat2. Shared with file.cpp. |
| `walk.hpp`, `walk.cpp` | Internal. `open_or_make` for one component, `open_or_make_path` for several. |
| `roots.cpp` | HOME, `base()`, the XDG roots, the program directory and `set_program_name`. Takes over state.hpp. |
| `directory.cpp` | `Directory(parent, name)` and the static accessors only. |
| `file.cpp` | Unchanged apart from using sys.hpp. |

Inside the library, operations return a descriptor, or -1 with errno set. The public API exposes only a valid or invalid Directory.

## open\_or\_make

The one primitive everything else is built on.

```cpp
enum class Links { FOLLOW, REFUSE };

// Opens parent/name as a directory (O_PATH), creating it with mode 0700 only if it is missing.
// Only ENOENT leads to mkdirat; EEXIST from mkdirat counts as success. The final open is the only
// proof that the directory exists and is of the right type. Returns a descriptor, or -1 with errno set.
i32 open_or_make(i32 parent, const char *name, Links links);
```

- Flags are `O_PATH | O_DIRECTORY | O_CLOEXEC`, plus `O_NOFOLLOW` for `Links::REFUSE`.
- Exists: 1 syscall. Missing: 3 (open, mkdirat, open).
- Any error other than ENOENT from the first open (EACCES, ENOTDIR, ELOOP, ...) fails immediately, without a mkdirat. Under `REFUSE` a symlink fails with ENOTDIR, which is what `O_PATH | O_DIRECTORY | O_NOFOLLOW` gives for a link.
- mkdirat errors other than EEXIST fail.
- ENOENT from the second open means the directory was removed in between. Retry the sequence a small, fixed number of times, then fail.

## open\_or\_make\_path

Only for roots: an absolute path from an XDG variable, or a fallback such as `.local/share` under HOME.

```cpp
// Fast path: one openat of the whole path. Only on ENOENT, walk the path one component at a time,
// holding the parent's descriptor at each step (open_or_make, then close the parent). Nothing is
// resolved twice, and empty components from "//" are skipped.
i32 open_or_make_path(i32 start, char *path, Links links);
```

- The path is copied once into a stack buffer and cut in place, at `/`. This runs once per root, so the copy does not matter.
- An absolute path starts the walk from an open of `/`.
- Cost, for depth N with k missing trailing components: 1 syscall when everything exists; otherwise the failed fast path, N − k opens, and 3 syscalls per missing component, plus a close per step.

## Roots and symlinks

Symlinks are followed up to and including the roots, and refused beneath them.

| Root | Source | Created | Symlinks |
| --- | --- | --- | --- |
| home | `$HOME`, which must be absolute | never | followed |
| `base()` | directory of `/proc/self/exe` | never | followed |
| XDG base | `$XDG_*_HOME` if absolute, else home + fallback | yes, 0700 | followed |
| program directory | XDG base / program name | yes, 0700 | followed |
| `Directory(parent, name)` | beneath any of the above | yes, 0700 | refused |

- The program directory is a root, so a user can move an application's directory elsewhere with a symlink. Everything beneath it is still protected.
- Relative XDG paths are ignored, as the Base Directory specification requires.
- `config()`, `data()`, `state()` and `cache()` stay invalid until `set_program_name` has succeeded.

## Ownership check

When the program directory is opened, `fstat` it and refuse it unless `st_uid == geteuid()` and it is not group- or world-writable. This catches a directory planted by another user on a shared system, for one extra syscall per root. Directories beneath it need no check: they are reached without following symlinks from a directory that passed.

## Caching

Cache only success. A root that failed is tried again on the next call, so a transient EACCES or a full disk at startup does not leave `config()` invalid for the rest of the process. This needs a small helper, "once, if it succeeded", instead of a function-local static. Once a root is cached, it never changes.

## Directory names

`Directory(parent, name)` keeps `validate_name`: one component, no `/`, not `.` or `..`. With one component, O\_NOFOLLOW is enough and directories never need openat2. Nesting is `Directory(Directory(parent, "a"), "b")`.

If names with several components are allowed later, use one openat2 call with `RESOLVE_NO_SYMLINKS | RESOLVE_BENEATH` as the fast path, and on ENOENT fall back to `open_or_make_path` with `Links::REFUSE`.

## Directory class

- Move construction and move assignment are `= default`; Handle owns and closes the descriptor.
- No user-declared destructor.

## Errors

Not yet more than valid or invalid at the API. Add an error enum when callers need to tell "missing" from "refused".

## Testing

- `open_or_make`: existing, missing, a file in the way (ENOTDIR), a symlink under `REFUSE` (refused) and `FOLLOW` (opened), no permission on the parent.
- `open_or_make_path`: all present, trailing components missing, `//` in the path, a symlinked component.
- Race: remove the directory between mkdirat and the second open, and check the bounded retry.
- Roots: a symlinked XDG base and program directory are followed; a relative `$XDG_*_HOME` is ignored; a program directory owned by another user, or group-writable, is refused.
- Caching: a root that fails once succeeds on a later call after the cause is removed.
