import subprocess
import threading
import sys


GAME_BIN_PATH = "../games/test_emu/game.bin"
HARNESS_EXECUTABLE = "./qemu_harness"
QEMU_COMMAND = ["qemu-arm", "-L", "/usr/arm-linux-gnueabihf/", HARNESS_EXECUTABLE, GAME_BIN_PATH]



RED = "\033[0;31m"
GREEN = "\033[0;32m"
LIGHT_GRAY = "\033[0;34m"
END = "\033[0m"



def parse_line(line):
    
    parts = line.strip().split(':')
    command = parts[0]
    
    if command == "API":
        api_call = parts[1]
        args = parts[2:] if len(parts) > 2 else []
        
        if api_call == "set_px":
            x, y, color = map(int, args[0].split(','))
            # TODO
            pass
        elif api_call == "flush":
            # TODO
            pass
        elif api_call == "backlight":
            # TODO
            pass
    elif command == "CTL":
        status = parts[1]
        if status == "ready":
            print(RED + "[CMD] CTL : Harness is ready and is about to launch the game" + END)
        elif status == "done":
            print(RED + "[CMD] CTL : Game execution is done" + END)

    else:
        # we print the line in gray
        print(LIGHT_GRAY + f"[HARNESS->EMU]: {line.strip()} ", END)


def read_output(pipe):
    # executed in a thread to read continuously the output of the harness
    # `for line in pipe` is a blocking loop
    for line in iter(pipe.readline, ''):
        parse_line(line)
    pipe.close()
    print("[EMU] harness stdout pipe is closed")




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
    stdout_thread = threading.Thread(target=read_output, args=(process.stdout,))
    stderr_thread = threading.Thread(target=read_output, args=(process.stderr,))

    stdout_thread.start()
    stderr_thread.start()

    
    # TODO: GUI


    print("[EMU] Waiting for the end of harness process ...")
    
    # Wait for the harness process to end
    process.wait()
    
    # Wait for the reading threads to finish their execution
    stdout_thread.join()
    stderr_thread.join()
    
    print("[EMU] harness process ended with exit code :", process.returncode)

except FileNotFoundError:
    print(f"[EMU] ERROR : Command '{QEMU_COMMAND[0]}' not found")
    print("Make sure QEMU is installed and included in PATH.")
except Exception as e:
    print(f"[EMU] An error occured : {e}")