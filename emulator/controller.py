import subprocess
import threading
import sys

import pygame


GAME_BIN_PATH = "../games/test_emu/game.bin"
HARNESS_EXECUTABLE = "./qemu_harness"
QEMU_COMMAND = ["qemu-arm", "-L", "/usr/arm-linux-gnueabihf/", HARNESS_EXECUTABLE, GAME_BIN_PATH]



RED = "\033[0;31m"
GREEN = "\033[0;32m"
LIGHT_GRAY = "\033[0;34m"
END = "\033[0m"





SCREEN_WIDTH = 160
SCREEN_HEIGHT = 128


BUTTONS_MAPPING = {
    #define BTN_A     14
    #define BTN_B     13
    #define BTN_UP    11
    #define BTN_DOWN  9
    #define BTN_LEFT  10
    #define BTN_RIGHT 12
    #define BTN_HOME 8
    14 : pygame.K_c,  # A
    13 : pygame.K_v,  # B
    8 : pygame.K_h,  # HOME
    11 : pygame.K_UP,
    19 : pygame.K_DOWN,
    10 : pygame.K_LEFT,
    12 : pygame.K_RIGHT,
}



pygame.init()
root = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("Asoboo Emulator")
key_state = {}

render_buffer = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
render_buffer.fill((0, 0, 0))




def _rgb565_to_rgrb888(color):
    r_5 = (color >> 11) & 0x1F
    g_6 = (color >> 5) & 0x3F
    b_5 = color & 0x1F
    r_8 = (r_5 * 255) // 31
    g_8 = (g_6 * 255) // 63
    b_8 = (b_5 * 255) // 31

    return (r_8, g_8, b_8)



def parse_line(line):
    
    parts = line.strip().split(':')
    command = parts[0]
    
    if command == "API":
        api_call = parts[1]
        args = parts[2:] if len(parts) > 2 else []
        
        if api_call == "set_px":
            x, y, color = map(int, args[0].split(','))
            render_buffer.set_at((x, y), _rgb565_to_rgrb888(color))
            return;
        elif api_call == "flush":
            root.blit(render_buffer, (0, 0))
            pygame.display.update()
            return;
        elif api_call == "backlight":
            state = args[0]
            if (state):
                root.blit(render_buffer, (0, 0))
                pygame.display.update()
            else:
                root.fill("black")
                pygame.display.update()
            return;
        elif api_call == "get_btn":
            if key_state[BUTTONS_MAPPING[int(args[0])]]:
                process.stdin.write("1\n")
            else:
                process.stdin.write("0\n")
            process.stdin.flush()
    elif command == "CTL":
        status = parts[1]
        if status == "ready":
            print(RED + "[CMD] CTL : Harness is ready and is about to launch the game" + END)
        elif status == "done":
            print(RED + "[CMD] CTL : Game execution is done" + END)
        
        return;

    else:
        # we print the line in gray
        print(LIGHT_GRAY + f"[HARNESS->EMU]: {line.strip()} ", END)


def read_output(pipe, stop_event):
    while not stop_event.is_set():
        line = pipe.readline()
        if line:
            parse_line(line)
        else:
            break
    print("[EMU] harness pipe is closed")




print("[EMU] Launching harness subprocess")
try:
    # launching C subprocess.
    # stdout=subprocess.PIPE to catch the output.
    # stderr=subprocess.PIPE to catch errors.
    # text=True so that pipes reaed text (str) instead of bytes.
    # bufsize=1 for 'line buffering' equivalent to the 'setvbuf' in C.
    process = subprocess.Popen(
        QEMU_COMMAND,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        stdin=subprocess.PIPE, # On le garde pour pouvoir envoyer des commandes plus tard
        text=True,
        bufsize=1 
    )
    print(f"[EMU] Harness subprocess started with PID : {process.pid}")

    # Creating threads to read stdout and stderr without blocking the main thread
    stop_event = threading.Event()
    stdout_thread = threading.Thread(target=read_output, args=(process.stdout, stop_event))
    stderr_thread = threading.Thread(target=read_output, args=(process.stderr, stop_event))

    stdout_thread.start()
    stderr_thread.start()

    
    running = True
    clock = pygame.time.Clock()

    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        key_state = pygame.key.get_pressed()
        clock.tick(60)

    print("[EMU] Window closed, stopping process")
    stop_event.set()

    if process.poll() is None: # if harness is still running
        print("[EMU] stopping harness process")
        process.terminate()
        process.wait()


    # Wait for the reading threads to finish their execution
    stdout_thread.join()
    stderr_thread.join()
    pygame.quit()
    print("[EMU] harness process ended with exit code :", process.returncode)

except FileNotFoundError:
    print(f"[EMU] ERROR : Command '{QEMU_COMMAND[0]}' not found")
    print("Make sure QEMU is installed and included in PATH.")
except Exception as e:
    print(f"[EMU] An error occured : {e}")