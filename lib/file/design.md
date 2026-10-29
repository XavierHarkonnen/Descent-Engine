# Open-First vs Always-Mkdir: Directory Fast Path

Oct 28, 2026 · @Xav

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
