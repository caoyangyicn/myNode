# Permissions

Permissions can be used to control what system resources the
Node.js process has access to or what actions the process can take
with those resources.

* [Process-based permissions](#process-based-permissions) control the Node.js
  process's access to resources.
  The resource can be entirely allowed or denied, or actions related to it can
  be controlled. For example, file system reads can be allowed while denying
  writes.
  This feature does not protect against malicious code. According to the Node.js
  [Security Policy][], Node.js trusts any code it is asked to run.

The permission model implements a "seat belt" approach, which prevents trusted
code from unintentionally changing files or using resources that access has
not explicitly been granted to. It does not provide security guarantees in the
presence of malicious code. Malicious code can bypass the permission model and
execute arbitrary code without the restrictions imposed by the permission
model.

If you find a potential security vulnerability, please refer to our
[Security Policy][].

## Process-based permissions

### Permission Model

<!-- type=misc -->

> Stability: 1.1 - Active development

<!-- name=permission-model -->

The Node.js Permission Model is a mechanism for restricting access to specific
resources during execution.
The API exists behind a flag [`--experimental-permission`][] which when enabled,
will restrict access to all available permissions.

The available permissions are documented by the [`--experimental-permission`][]
flag.

When starting Node.js with `--experimental-permission`,
the ability to access the file system through the `fs` module, spawn processes,
use `node:worker_threads`, use native addons, use WASI, and enable the runtime inspector
will be restricted.

```console
$ node --experimental-permission index.js
node:internal/modules/cjs/loader:171
  const result = internalModuleStat(filename);
                 ^

Error: Access to this API has been restricted
    at stat (node:internal/modules/cjs/loader:171:18)
    at Module._findPath (node:internal/modules/cjs/loader:627:16)
    at resolveMainPath (node:internal/modules/run_main:19:25)
    at Function.executeUserEntryPoint [as runMain] (node:internal/modules/run_main:76:24)
    at node:internal/main/run_main_module:23:47 {
  code: 'ERR_ACCESS_DENIED',
  permission: 'FileSystemRead',
  resource: '/home/user/index.js'
}
```

Allowing access to spawning a process and creating worker threads can be done
using the [`--allow-child-process`][] and [`--allow-worker`][] respectively.

To allow native addons when using permission model, use the [`--allow-addons`][]
flag. For WASI, use the [`--allow-wasi`][] flag.

#### Runtime API

When enabling the Permission Model through the [`--experimental-permission`][]
flag a new property `permission` is added to the `process` object.
This property contains one function:

##### `permission.has(scope[, reference])`

API call to check permissions at runtime ([`permission.has()`][])

```js
process.permission.has('fs.write'); // true
process.permission.has('fs.write', '/home/rafaelgss/protected-folder'); // true

process.permission.has('fs.read'); // true
process.permission.has('fs.read', '/home/rafaelgss/protected-folder'); // false
```

#### File System Permissions

The Permission Model, by default, restricts access to the file system through the `node:fs` module.
It does not guarantee that users will not be able to access the file system through other means,
such as through the `node:sqlite` module.

To allow access to the file system, use the [`--allow-fs-read`][] and
[`--allow-fs-write`][] flags:

```console
$ node --experimental-permission --allow-fs-read=* --allow-fs-write=* index.js
Hello world!
(node:19836) ExperimentalWarning: Permission is an experimental feature
(Use `node --trace-warnings ...` to show where the warning was created)
```

The valid arguments for both flags are:

* `*` - To allow all `FileSystemRead` or `FileSystemWrite` operations,
  respectively.
* Paths delimited by comma (`,`) to allow only matching `FileSystemRead` or
  `FileSystemWrite` operations, respectively.

Example:

* `--allow-fs-read=*` - It will allow all `FileSystemRead` operations.
* `--allow-fs-write=*` - It will allow all `FileSystemWrite` operations.
* `--allow-fs-write=/tmp/` - It will allow `FileSystemWrite` access to the `/tmp/`
  folder.
* `--allow-fs-read=/tmp/ --allow-fs-read=/home/.gitignore` - It allows `FileSystemRead` access
  to the `/tmp/` folder **and** the `/home/.gitignore` path.

Wildcards are supported too:

* `--allow-fs-read=/home/test*` will allow read access to everything
  that matches the wildcard. e.g: `/home/test/file1` or `/home/test2`

After passing a wildcard character (`*`) all subsequent characters will
be ignored. For example: `/home/*.js` will work similar to `/home/*`.

When the permission model is initialized, it will automatically add a wildcard
(\*) if the specified directory exists. For example, if `/home/test/files`
exists, it will be treated as `/home/test/files/*`. However, if the directory
does not exist, the wildcard will not be added, and access will be limited to
`/home/test/files`. If you want to allow access to a folder that does not exist
yet, make sure to explicitly include the wildcard:
`/my-path/folder-do-not-exist/*`.

#### Permission Model constraints

There are constraints you need to know before using this system:

* The model does not inherit to a child node process or a worker thread.
* When using the Permission Model the following features will be restricted:
  * Native modules
  * Child process
  * Worker Threads
  * Inspector protocol
  * File system access
  * WASI
* The Permission Model is initialized after the Node.js environment is set up.
  However, certain flags such as `--env-file` or `--openssl-config` are designed
  to read files before environment initialization. As a result, such flags are
  not subject to the rules of the Permission Model. The same applies for V8
  flags that can be set via runtime through `v8.setFlagsFromString`.
* OpenSSL engines cannot be requested at runtime when the Permission
  Model is enabled, affecting the built-in crypto, https, and tls modules.
* Using existing file descriptors via the `node:fs` module bypasses the
  Permission Model.

#### Limitations and Known Issues

* Symbolic links will be followed even to locations outside of the set of paths
  that access has been granted to. Relative symbolic links may allow access to
  arbitrary files and directories. When starting applications with the
  permission model enabled, you must ensure that no paths to which access has
  been granted contain relative symbolic links.

[Security Policy]: https://github.com/nodejs/node/blob/main/SECURITY.md
[`--allow-addons`]: cli.md#--allow-addons
[`--allow-child-process`]: cli.md#--allow-child-process
[`--allow-fs-read`]: cli.md#--allow-fs-read
[`--allow-fs-write`]: cli.md#--allow-fs-write
[`--allow-wasi`]: cli.md#--allow-wasi
[`--allow-worker`]: cli.md#--allow-worker
[`--experimental-permission`]: cli.md#--experimental-permission
[`permission.has()`]: process.md#processpermissionhasscope-reference