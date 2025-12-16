# Changelog

## 2025-12-16
- Replace deprecated `websockets` legacy imports with top-level `websockets` API (`serve`, `connect`, `ServerProtocol`, `ConnectionClosed`), fixing deprecation warnings. ✅
- Add GitHub Actions workflow to run tests and fail on deprecation warnings (treat DeprecationWarning as errors). ✅
- Add automated E2E test simulating lobby flows. ✅
