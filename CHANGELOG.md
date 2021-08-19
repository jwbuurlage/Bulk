# Changelog

All notable changes to Bulk will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic
Versioning](https://semver.org/spec/v2.0.0.html).

## [3.0.0] - 2021-08-19

### Breaking changes

- Bulk now requires C++20.
- `bulk::index` now represents indices as `size_t`

### Added

- Add `clear_queues` argument to `world::sync`, to optionally retain all messages
  from the previous superstep.
- Expose `clear` method of `queue`.
- Add DGEMM example (Cannon's algorithm).

### Changed

- Default barrier for thread backend is now `std::barrier`.
- Added `Barrier` concept, and constrain the associated `environment` and
  `world` parameters.
- Span based coarray puts are now based on `std::span`.
  
### Removed

- `bulk::span` was removed in favor of `std::span`.

### Fixed

- Prevent `bulk::sum` and `bulk::product` from silently truncating values (@TimoMaarse, #11)
- Fix const correctness of `bulk::product(bulk::world&, T)`.
- Fix data race in shared state creation for `world::split` in the thread backend.
- Explicitly cast to `T*` in the implementation of `bulk::coarray:data`.

## [2.0.0] - 2020-03-27

### Breaking changes

- Use `size_t` for indices in arrays and slices
- `coarray::data` now returns a `T*` instead of `void*`
- The iterator overload for `coarray::put` now requires an explicit offset
- Index type for D = 1 now wraps an `int`, instead of a singleton array

### Added

- Add `foldl` and `foldl_each` for coarrays.
- Add aliases for common folds: `sum`, `product`, `min`, and `max`, for values, variables and coarrays. _Example_: `bulk::max(world, s + 1) == p`.
- Add `bulk::span` for coarrays, allowing contiguous sequences as slice data
  sources in addition to `std::vector`.
- Add `coarray::data` to access the underlying (sequential) storage of coarrays.
- Allow externally managed data buffers for `coarray`.
- Add support for splitting a world into multiple parts, allowing subset syncs and communication. See also `bulk::world::split`.
- Add `get_change` to timer class (@SdeBerg, #10)

### Changed

- Use `size_t` for indices in arrays and slices
- `coarray::data` now returns a `T*` instead of `void*`
- The iterator overload for `coarray::put` now requires an explicit offset
- Index type for D = 1 now wraps an `int`, instead of a singleton array

### Fixed

- Fixed 'element owner' and 'local <-> global' computations for block
  partitionings with sizes that are not perfect divisors
- Fixed virtual qualifications on a number of internal functions

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

[Unreleased]: https://github.com/jwbuurlage/bulk/compare/v3.0.0...develop
[3.0.0]: https://github.com/jwbuurlage/bulk/compare/v2.0.0...v3.0.0
[2.0.0]: https://github.com/jwbuurlage/bulk/compare/v1.2.0...v2.0.0
[1.2.0]: https://github.com/jwbuurlage/bulk/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/jwbuurlage/bulk/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/jwbuurlage/bulk/compare/v0.2.0...v1.0.0
[0.2.0]: https://github.com/jwbuurlage/bulk/compare/v0.1.0...v0.2.0
