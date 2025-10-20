# Génère .bin et .hex à partir de l'ELF après chaque build PlatformIO
# Compatible Windows (chemins avec espaces) grâce aux guillemets.

from SCons.Script import Import
Import("env")

# On référence directement les chemins avec guillemets pour éviter les soucis d'espaces
elf     = '"$BUILD_DIR/${PROGNAME}.elf"'
bin_out = '"$BUILD_DIR/${PROGNAME}.bin"'
hex_out = '"$BUILD_DIR/${PROGNAME}.hex"'

# .bin
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(
        f"$OBJCOPY -O binary {elf} {bin_out}",
        "Generating " + bin_out
    )
)

# .hex
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(
        f"$OBJCOPY -O ihex {elf} {hex_out}",
        "Generating " + hex_out
    )
)
