# minemu Bootloader

This is a firmware-maintainer guide. Students using the supplied Boot ROM should
follow the [template quickstart](../README.md) instead.

This directory contains the fixed A32 firmware mapped into the platform's
64 KiB Boot ROM. It is the first guest code executed after reset.

The reset assembly establishes supervisor state and a temporary physical stack.
The C loader then reads the packed image from system ROM, copies kernel segments
into RAM, clears their BSS extents, writes the boot-info record, and transfers
control to the kernel's physical bootstrap entry. The kernel bootstrap is
responsible for constructing page tables and entering higher-half code.

`bootloader.bin` is checked in so normal template use does not depend on a local
compiler version. Build and compare the source-derived ROM with:

```sh
make -C bootloader
make -C bootloader check
```

After an intentional firmware change, replace the canonical binary and verify
it again with:

```sh
make -C bootloader update
make -C bootloader check
```

All generated ELF, map, object, dependency, and binary files remain under
`bootloader/build/`.
