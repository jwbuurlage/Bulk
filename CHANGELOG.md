# Changelog

All notable changes to Bulk will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic
Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.0] - 2018-11-02

### Added

- Add CMake targets `bulk` and `bulk_[backend]`
- Add partitionings: cyclic, block, tree, rectangular, Cartesian
- Add parallel scientific computing examples and documentation
- Add `partitioned_array`
- Add Travis CI support
- Add `world::log_once`

### Fixed

- Remove duplicate sync from `foldl`
- Fix certain unittests relying on shared-memory access to `success` counter
- Fix coarray unittest relying on `p > 2`
- Fix `-Werror=format-security` issue in `world::log`

### Changed

- Update default value for `start_value` of `foldl` to `{}` from `0` for
  non-numeric types
- Allow different type for accumulator of `foldl`

## [1.1.0] - 2018-10-10

### Added

- Add a spinlock barrier `bulk::thread::spinning_barrier` to the thread
  backend, which is now used by default
- Add citation instruction to README

## [1.0.0] - 2018-02-27

### Added

- Backend for the Epiphany coprocessor

### Fixed

- Let `coarray::image::put` take values by const reference
- Allow non-uniform local array sizes in `coarray` 
- Require `T` to satisfy `is_trivially_copyable` for `coarray<T>`
- Add more type safety checks to (de-)serialization 
- (De-)serialization in `var` and `future` now avoids redundant memory
  allocation and copying, by making the memory buffer objects non-owning

## [0.2.0] - 2017-08-09

### Added

- Add support for `std::string` variables and queues
- Add support for array components in messages, i.e. `queue<T[], U, V, ...>`
- Deprecate `processor_id` in favor of `rank`, renamed `{next,prev}_processor`
  to `{next, prev}_rank`

### Fixed

- Fixed bug in MPI backend where messages could get truncated

## 0.1.0 - 2017-06-01

- Initial release. A complete modern replacement for BSPlib.

[1.2.0]: https://github.com/jwbuurlage/bulk/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/jwbuurlage/bulk/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/jwbuurlage/bulk/compare/v0.2.0...v1.0.0
[0.2.0]: https://github.com/jwbuurlage/bulk/compare/v0.1.0...v0.2.0
