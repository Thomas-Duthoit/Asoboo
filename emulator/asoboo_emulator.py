import sys
import struct
import numpy as np
import matplotlib.pyplot as plt
import pygame

# --- We only import the absolute necessities that exist in all versions ---
from unicorn import Uc, UcError, UC_HOOK_CODE, UC_ARCH_ARM, UC_MODE_THUMB
from unicorn.arm_const import UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC, UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2

# --- Emulator Configuration ---
CODE_ADDR = 0x20030000  # Match bootloader.h
STACK_ADDR = 0x2003FFFC  # End of game code region, as in bootloader
API_TABLE_ADDR = 0x20020000  # Place API table below game code
SCREEN_WIDTH = 160
SCREEN_HEIGHT = 128

class PicoEmulator:
    def print_c_api_ptrs(self):
        # Print the 7 function pointer values and the api pointer written by C code
        try:
            ptrs = struct.unpack('<IIIIIII', self.uc.mem_read(0x20022000, 28))
            api_ptr = struct.unpack('<I', self.uc.mem_read(0x20022020, 4))[0]
            print("[EMU] C code API ptrs at 0x20022000:")
            for i, ptr in enumerate(ptrs):
                print(f"  [{i}] = 0x{ptr:08X}")
            print(f"[EMU] C code api pointer value: 0x{api_ptr:08X}")
        except Exception as e:
            print(f"[EMU] Could not read C code API ptrs: {e}")
    def print_api_table(self):
        # Print the 7 function pointer values from the API table
        api_table = self.uc.mem_read(API_TABLE_ADDR, 28)
        ptrs = struct.unpack('<IIIIIII', api_table)
        print("[EMU] API table at 0x%08X:" % API_TABLE_ADDR)
        for i, ptr in enumerate(ptrs):
            print(f"  [{i}] = 0x{ptr:08X}")
    def __init__(self, code_binary_path):
        self.code_binary_path = code_binary_path
        # 1D framebuffer in RGB565, to match C code
        self.framebuffer = np.zeros(SCREEN_HEIGHT * SCREEN_WIDTH, dtype=np.uint16)
        # Pygame setup
        pygame.init()
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        pygame.display.set_caption("Pico Emulator Display")
        self.clock = pygame.time.Clock()
        self.uc = Uc(UC_ARCH_ARM, UC_MODE_THUMB)
        # Map 256KB SRAM (covers API table and game code)
        self.uc.mem_map(0x20000000, 256 * 1024)
        # Map 4KB at 0xFF000000 for API hooks (executable)
        self.uc.mem_map(0xFF000000, 0x1000)
        self.uc.reg_write(UC_ARM_REG_SP, STACK_ADDR)
        self._load_code()
        self._setup_api_table()

    def _load_code(self):
        try:
            with open(self.code_binary_path, 'rb') as f:
                code = f.read()
            self.uc.mem_write(CODE_ADDR, code)
            print(f"Loaded {len(code)} bytes of code at 0x{CODE_ADDR:08X}")
        except FileNotFoundError:
            print(f"Error: Could not find '{self.code_binary_path}'.")
            print("Please compile the C code first to create 'game.bin'.")
            sys.exit(1)

    def _setup_api_table(self):
        self.api_hooks = {
            0xFF000000: self._api_backlight_state,
            0xFF000004: self._api_flush_render_buffer,
            0xFF000008: self._api_set_px,
            0xFF00000C: self._api_get_px,
            0xFF000010: self._api_put_sprite,
            0xFF000014: self._api_get_btn,
            0xFF000018: self._api_get_rand_32,
        }
        api_struct_data = struct.pack('<IIIIIII', *self.api_hooks.keys())
        self.uc.mem_write(API_TABLE_ADDR, api_struct_data)
        print(f"API function table written to 0x{API_TABLE_ADDR:08X}")
        # Write a Thumb 'bx lr' (0x4770) at each API address to prevent UC_ERR_READ_UNMAPPED
        bx_lr_thumb = b'\x70\x47'  # bx lr in Thumb
        for addr in self.api_hooks.keys():
            self.uc.mem_write(addr, bx_lr_thumb)
        self.uc.hook_add(UC_HOOK_CODE, self._hook_code)

    def _hook_code(self, uc, address, size, user_data):
        if address in self.api_hooks:
            self.api_hooks[address](uc)
            uc.reg_write(UC_ARM_REG_PC, uc.reg_read(UC_ARM_REG_LR))

    def _api_print_serial(self, uc):
        ptr = uc.reg_read(UC_ARM_REG_R0)
        string_data = b''
        while True:
            char_byte = uc.mem_read(ptr, 1)
            if char_byte == b'\0': break
            string_data += char_byte
            ptr += 1
        print(f"[SERIAL] {string_data.decode('utf-8', errors='ignore')}", end='')

    def _api_backlight_state(self, uc):
        print(f"[HW] Backlight state set to: {'ON' if uc.reg_read(UC_ARM_REG_R0) else 'OFF'}")

    def _rgb565_to_rgb888(self, c):
        r = (c >> 11) & 0x1F
        g = (c >> 5) & 0x3F
        b = c & 0x1F
        return ((r * 255) // 31, (g * 255) // 63, (b * 255) // 31)

    def _api_set_px(self, uc):
        x, y, c = uc.reg_read(UC_ARM_REG_R0), uc.reg_read(UC_ARM_REG_R1), uc.reg_read(UC_ARM_REG_R2)
        if 0 <= x < SCREEN_WIDTH and 0 <= y < SCREEN_HEIGHT:
            idx = SCREEN_WIDTH * y + x
            self.framebuffer[idx] = c & 0xFFFF

    def _api_flush_render_buffer(self, uc): pass
    def _api_get_px(self, uc):
        x, y = uc.reg_read(UC_ARM_REG_R0), uc.reg_read(UC_ARM_REG_R1)
        if 0 <= x < SCREEN_WIDTH and 0 <= y < SCREEN_HEIGHT:
            idx = SCREEN_WIDTH * y + x
            uc.reg_write(UC_ARM_REG_R0, int(self.framebuffer[idx]))
        else:
            uc.reg_write(UC_ARM_REG_R0, 0)
    def _api_put_sprite(self, uc): pass

    BTN_MAP = {
        14: pygame.K_z,   # BTN_A
        13: pygame.K_x,   # BTN_B
        11: pygame.K_UP,  # BTN_UP
        9:  pygame.K_DOWN,# BTN_DOWN
        10: pygame.K_LEFT,# BTN_LEFT
        12: pygame.K_RIGHT# BTN_RIGHT
    }
    def _api_get_btn(self, uc):
        btn = uc.reg_read(UC_ARM_REG_R0)
        key = self.BTN_MAP.get(btn, None)
        pressed = pygame.key.get_pressed()
        uc.reg_write(UC_ARM_REG_R0, 1 if key and pressed[key] else 0)

    def _api_get_rand_32(self, uc):
        import random
        uc.reg_write(UC_ARM_REG_R0, random.getrandbits(32))

    def run(self, entry_offset=0):
        self.print_api_table()
        # Print C code's view of API ptrs before running main loop
        self.print_c_api_ptrs()

        # Print first 32 bytes of loaded code for debug
        try:
            with open(self.code_binary_path, 'rb') as f:
                code = f.read()
            print(f"First 32 bytes of code: {code[:32].hex()}")
        except Exception:
            code = b''
            pass

        # Warn if entry_offset is not 0
        if entry_offset != 0:
            print(f"[EMU][WARN] game_main starts at offset 0x{entry_offset:X} in .text section. Starting emulation there.")

        # Print bytes at entry offset
        if code and entry_offset < len(code):
            print(f"Bytes at entry offset: {code[entry_offset:entry_offset+16].hex()}")
        else:
            print("[EMU][DEBUG] Entry offset out of code bounds or code not loaded.")

        # Disassembly (optional, can be removed for speed)
        try:
            from capstone import Cs, CS_ARCH_ARM, CS_MODE_THUMB
            md = Cs(CS_ARCH_ARM, CS_MODE_THUMB)
            entry_bytes = code[entry_offset:entry_offset+16]
            print("Disassembly at entry point:")
            for i, insn in enumerate(md.disasm(entry_bytes, CODE_ADDR + entry_offset | 1)):
                print(f"  0x{insn.address:08X}: {insn.mnemonic} {insn.op_str}")
                if i >= 7:
                    break
        except ImportError:
            pass
        except Exception as e:
            print(f"[EMU][DEBUG] Disassembly error: {e}")

        # Set R0 to API table address just before starting emulation
        self.uc.reg_write(UC_ARM_REG_R0, API_TABLE_ADDR)

        # Set LR to 0 (simulate reset vector return)
        self.uc.reg_write(UC_ARM_REG_LR, 0)

        # Set PC to entry point (Thumb mode)
        entry_point = (CODE_ADDR + entry_offset) | 1
        self.uc.reg_write(UC_ARM_REG_PC, entry_point)
        print(f"\n--- Starting Emulation at 0x{entry_point:X} ---")
        running = True
        while running:
            # Emulate a small step (e.g., 5000 instructions)
            try:
                self.uc.emu_start(entry_point, CODE_ADDR + (2 * 1024 * 1024) - 1, count=5000)
            except UcError as e:
                UC_ERR_TIMEOUT_VALUE = 11
                if e.errno == UC_ERR_TIMEOUT_VALUE:
                    print("\n--- Emulation finished (timeout) ---")
                    running = False
                else:
                    print(f"!!! Emulation Error: {e} !!!")
                    self.dump_context()
                    running = False
            # Handle Pygame events
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
            self.show_display()
            self.clock.tick(60)
        pygame.quit()

    def show_display(self):
        # Convert framebuffer to RGB888 surface for Pygame
        surf = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
        for y in range(SCREEN_HEIGHT):
            for x in range(SCREEN_WIDTH):
                idx = SCREEN_WIDTH * y + x
                surf.set_at((x, y), self._rgb565_to_rgb888(self.framebuffer[idx]))
        self.screen.blit(surf, (0, 0))
        pygame.display.flip()

    def dump_context(self):
        print("--- Register Dump ---")
        for i in range(13): print(f"R{i:<2} = 0x{self.uc.reg_read(UC_ARM_REG_R0 + i):08X}")
        print(f"SP = 0x{self.uc.reg_read(UC_ARM_REG_SP):08X}, LR = 0x{self.uc.reg_read(UC_ARM_REG_LR):08X}, PC = 0x{self.uc.reg_read(UC_ARM_REG_PC):08X}")

import subprocess
def find_game_main_offset(elf_path):
    try:
        # Use nm to find the address of game_main
        result = subprocess.run([
            "arm-none-eabi-nm", elf_path
        ], capture_output=True, text=True)
        for line in result.stdout.splitlines():
            if "game_main" in line:
                addr_str = line.split()[0]
                return int(addr_str, 16)
    except Exception as e:
        print(f"[EMU] Could not find game_main offset: {e}")
    return 0

if __name__ == "__main__":
    import os
    import subprocess
    elf_path = "games/test_emu/game.elf"
    # Use objcopy to extract the .text section to a temp binary file
    bin_path = "games/test_emu/game.bin"
    try:
        subprocess.run([
            "arm-none-eabi-objcopy",
            "-O", "binary",
            "-j", ".text",
            elf_path,
            bin_path
        ], check=True)
        print(f"[EMU] Extracted .text section from ELF to {bin_path}")
    except Exception as e:
        print(f"[EMU] Failed to extract .text section: {e}")
        sys.exit(1)

    # Always start at offset 0 (start of .text section)
    emulator = PicoEmulator(bin_path)
    emulator.run(entry_offset=0)