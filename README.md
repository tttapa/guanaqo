# guanaqo

Utilities for scientific software.

Used by the following projects:
 - [alpaqa](https://github.com/kul-optec/alpaqa)
 - [alpaqa4scale](https://gitlab.esat.kuleuven.be/tipoflow/alpaqa4scale)
 - [cyqlone](https://github.com/kul-optec/cyqlone)
 - [hyhound](https://github.com/kul-optec/hyhound)
 - [batmat](https://github.com/tttapa/batmat)

## Features

- Non-owning views for dense and sparse matrices.
- User-friendly wrappers for BLAS/LAPACK routines.
- Interoperability with Eigen.
- Conversions between sparse matrix formats (CSC, COO, dense).
- I/O & printing utilities for printing and reading matrices (Python, MATLAB, CSV formats).
- Flexible type erasure utilities (owning and non-owning), tag-invoke style customization points.
- Utilities for string parsing, compile-time lookup tables, dynamic loading, preprocessor macros.
- Timing and benchmarking helpers, tracing support, integration with Intel ITT.
