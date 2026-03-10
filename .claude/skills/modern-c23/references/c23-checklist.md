# C23 Checklist

- Use `nullptr` instead of `NULL`.
- Mark public error-returning APIs with `[[nodiscard]]`.
- Use `_Static_assert` for size and enum invariants.
- Use `<stdckdint.h>` for input-derived size arithmetic.
- Keep scalar fallback for every SIMD-specific optimization.
- Avoid locale-aware parsing helpers in protocol code.
