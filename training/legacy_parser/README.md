# Educational Legacy Parser (Training Fixture)

**This code is intentionally vulnerable and must not be used in production.**

Enable with:

```bash
cmake -DTINYCFG_ENABLE_LEGACY_TRAINING_FIXTURE=ON
```

The fixture provides a small binary record parser with a planted out-of-bounds read (`parse_records_vulnerable`) and a fixed counterpart (`parse_records_fixed`).

See `CASE_STUDY.md` for the full triage write-up and `data/minimized_heap_buffer_overflow.bin` for the minimized reproducer.
