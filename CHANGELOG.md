# Changelog

All notable customer-visible changes to QnBot SDK examples and public package
delivery are documented in this file.

## Unreleased

- Replaced the hardware-free C++ `coroutine_next` example with the real-glove
  C++20 `async_runtime` workflow. Update build targets and run commands from
  `coroutine_next` to `async_runtime`; the replacement requires a connected
  glove and compatible retargeting algorithm package.
- Aligned the Python and C++ public example inventories at eleven workflows by
  adding C++ `debug_trace`, `multiple_outputs`, and `device_lifecycle` examples.
- Validated GitHub App-backed Draft proposals for reviewed public example
  updates in the `develop` channel.
- Hardened the reviewed publication workflow used to propose public example
  updates.
- Added reviewed snapshot proposals for public example changes merged through
  the `develop` validation channel.
- Added the public QnBot SDK repository structure.
- Added Python and C++ Glove SDK examples.
- Added independent stable and release-candidate package delivery for Core and
  Glove through GitHub Releases.
