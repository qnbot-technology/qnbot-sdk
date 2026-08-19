# qnbot-sdk

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Release](https://img.shields.io/github/v/release/qnbot-technology/qnbot-sdk?cacheSeconds=3600)](https://github.com/qnbot-technology/qnbot-sdk/releases)

Public examples and binary releases for QnBot SDK. The SDK implementation is
distributed as prebuilt packages and is not part of this repository.

## SDKs

| Language | Install | Documentation | Examples |
| --- | --- | --- | --- |
| Python | Download the matching Core and Glove wheels from [Releases](https://github.com/qnbot-technology/qnbot-sdk/releases) | [Python guide](examples/python/README.md) | [examples/python](examples/python/) |
| C++ | Download the matching Core and Glove archives from [Releases](https://github.com/qnbot-technology/qnbot-sdk/releases) | [C++ guide](examples/cpp/README.md) | [examples/cpp](examples/cpp/) |

Core and Glove are versioned independently with tags named
`qnbot-sdk-core-vX.Y.Z` and `qnbot-sdk-glove-vX.Y.Z`. Stable and release
candidate builds are published here; development builds remain internal.

## Repository Structure

```text
├── examples/
│   ├── python/            # Python environment, guide, and runnable examples
│   └── cpp/               # CMake project, guide, and runnable examples
├── CHANGELOG.md           # Customer-visible release history
├── LICENSE
└── README.md
```

## Getting Started

- Follow [the Python guide](examples/python/README.md) to install wheels and run
  the Python examples.
- Follow [the C++ guide](examples/cpp/README.md) to install the CMake packages
  and build the C++ examples.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for customer-visible changes.

## Support

For package access, device integration, or SDK support, contact your QnBot
representative.

## License

[MIT](LICENSE)
