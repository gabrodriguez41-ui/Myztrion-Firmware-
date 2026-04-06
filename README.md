# Myztrion project (organized)

## Structure
- `myztrion/python/` : Python host library (`Myztrion.py`) + USB backend helpers.
- `myztrion/firmware/` : C firmware sources for the RP2040/Pico.
- `examples/python/` : Original rp2daq examples + converted `*_myztrion.py` variants.

## Running an example
From repo root:

```bash
python3 examples/python/hello_world_myztrion.py
```

(Examples add `myztrion/python/` to `sys.path` automatically.)
