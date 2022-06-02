# GB Emu

This is a Gameboy emulator.
Probably the third Gameboy emulator I write at this point... Except this time it's not as bad as the last two times so I decided to open source it in case anyone is looking for different approaches.

Current Status:
- Joypad supported, though there's current no key remapping
- Interrupts work
- Serial inteface is technically implemented but currently doesn't do anything
- Blargg's cpu_instr rom passes all tests.
- All CPU instructions should work now, tough some timings may be incorrect (I haven't tested everything yet)
- Most games are in a playable state (~80% of what I've tested), a couple still crash, and another couple have major graphics glitches
- MBC1/MBC3/standalone ROMs are supported, other Memory Bank Controllers have yet to be implemented
- Saves are supported (for supported Memory Bank Controllers)
- Not all DMG acid2 tests pass
- CGB/SGB features are missing
- Audio still needs some work (frequency/volume sweeps are technically implement but since there's no countdown, they don't work)

TODO:
- Fix remaining PPU glitches
- Add countdown in audio in order to support volume envelops/frequency sweeps
- Check instruction timings
- Implement remaining memory bank controllers
- Clean up backend code and add proper interfaces for it
- Add CGB/SGB support
