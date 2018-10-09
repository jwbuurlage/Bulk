# Changelog

## 1.2.0

Unreleased

## 1.1.0

2018-10-10

### Added

- Add a spinlock barrier `bulk::thread::spinning_barrier` to the thread
  backend, which is now used by default.
- Add citation instruction to README

## 1.0.0

2018-02-27

### Added

- Backend for the Epiphany coprocessor

### Fixed

- Let `coarray::image::put` take values by const reference
- Allow non-uniform local array sizes in `coarray` 
- Require `T` to satisfy `is_trivially_copyable` for `coarray<T>`
- Add more type safety checks to (de-)serialization 
- (De-)serialization in `var` and `future` now avoids redundant memory
  allocation and copying, by making the memory buffer objects non-owning

## 0.2.0

2017-08-09

### Added

- Add support for `std::string` variables and queues
- Add support for array components in messages, i.e. `queue<T[], U, V, ...>`
- Deprecate `processor_id` in favor of `rank`, renamed `{next,prev}_processor`
  to `{next, prev}_rank`

### Fixed

- Fixed bug in MPI backend where messages could get truncated

## 0.1.0

2017-06-01

- Initial release. A complete modern replacement for BSPlib.
