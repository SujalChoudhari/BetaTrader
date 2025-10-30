
# Vendor Libraries

This directory contains third-party dependencies used in the project.  
All libraries are vendored locally to ensure consistent builds and remove external dependency on package managers or remote repositories.

## Structure

```

vendor/
└── lib/
├── googletest/   # GoogleTest framework for unit testing
├── spdlog/       # Fast, header-only C++ logging library
├── SPSCQueue/    # Single-producer, single-consumer lock-free queue
└── SQLiteCpp/    # Lightweight C++ wrapper around SQLite3

```

## Libraries & Versions

| Library       | Badge                                                         |
|---------------|---------------------------------------------------------------|
| GoogleTest    | [![GoogleTest v1.17.0](https://img.shields.io/badge/GoogleTest-v1.17.0-blue)](https://github.com/google/googletest/releases) :contentReference[oaicite:0]{index=0} |
| spdlog        | [![spdlog v1.15.x](https://img.shields.io/badge/spdlog-v1.15.x-blue)](https://github.com/gabime/spdlog/releases) :contentReference[oaicite:1]{index=1} |
| SPSCQueue     | [![SPSCQueue](https://img.shields.io/badge/SPSCQueue-latest-blue)](https://github.com/rigtorp/SPSCQueue/releases) :contentReference[oaicite:2]{index=2} |
| SQLiteCpp     | [![SQLiteCpp v3.3.3](https://img.shields.io/badge/SQLiteCpp-v3.3.3-blue)](https://github.com/SRombauts/SQLiteCpp) :contentReference[oaicite:3]{index=3} |

\* The SPSCQueue project does not publish standard “versioned” releases; treat it as a “latest commit” dependency.

## Notes

- These libraries are included as submodules or manually added from their upstream sources.
- Check each library’s LICENSE file for individual licensing terms.
- Do **not** modify vendored code directly. Submit patches upstream or maintain local changes in a separate patch file or fork.
- Badges link to the upstream release pages for convenience.

## Updating Dependencies

1. Pull or fetch the latest upstream code or release tag for the desired library.
2. Verify API compatibility with your project before merging.
3. Run all unit tests (via GoogleTest) and ensure everything still passes.
4. Update the version table above and regenerate badges if needed.

---

**Last Updated:** _30-10-2025_
