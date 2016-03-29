# Change Log
All notable changes to this project will be documented in this file.

This project adheres to [Semantic Versioning](http://semver.org/).

This project follows the [Gitflow Workflow model](https://www.atlassian.com/git/tutorials/comparing-workflows/gitflow-workflow).

## [Unreleased]
The Unreleased section will be empty for tagged releases. Unreleased functionality appears in the develop branch.

## [2.1.2] - 2016-03-29

### Fixed
- Windows termination bugs.
  - FNCS clients experienced hangs during exit.
  - fncs_broker would experience a WSAStartup assertion.
  - fncs_broker would exit even if realtime mode was not attempted.

## [2.1.1] - 2016-03-08

### Changed
- Python interface publish(key,value) now stringifies its arguments.

### Fixed
- Python interface bugs in
  - get_keys()
  - get_values(key)
  - get_events()

### Closed Issues
- fncs.py fails in get_keys(), get_events(), get_values(key) [\#3]
- python fncs.publish should stringify arguments [\#2]

## [2.1.0] - 2016-02-17

### Added
- (fncs.hpp) fncs::get_version(), (fncs.h) fncs_get_version().
- libtool versioning, starting with libfncs.so.1.0.0 to distinguish it from any previous effort.
- Added msvc 2013 build.

## [2.0] - 2016-02-16

This is the first production-ready release of the new FNCS library and
broker.  This is a complete rewrite of the previous FNCS implementation
and thus it gets the backwards-incompatible version 2.0 designation.

[Unreleased]: https://github.com/FNCS/fncs/compare/v2.1.2...develop
[2.1.2]: https://github.com/FNCS/fncs/compare/v2.1.1...v2.1.2
[2.1.1]: https://github.com/FNCS/fncs/compare/v2.1.0...v2.1.1
[2.1.0]: https://github.com/FNCS/fncs/compare/v2.0...v2.1.0
[2.0]: https://github.com/FNCS/fncs/releases/tag/v2.0

