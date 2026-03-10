# Policy Matrix

## Strict Default
- reject `obs-fold`
- reject bare LF
- reject `TE + CL`
- reject unencoded spaces in URI

## Consumer Profiles

### iohttp
- strict by default
- explicit leniency only where interoperability requires it

### ringwall
- fail-closed
- smaller limits
- no legacy compatibility assumptions
