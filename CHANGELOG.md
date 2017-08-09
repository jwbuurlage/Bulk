# Changelog

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
