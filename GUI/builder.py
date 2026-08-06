import subprocess

def flash_firmware(framesize: int, port: str = "/dev/ttyUSB0"):
    subprocess.run(
        ["make", "flash", f"UART_FRAMESIZE={framesize}"],
        check=True,
        cwd="firmware/"
    )

subprocess.run(["make", "clean"], cwd="firmware/")
subprocess.run(["make", f"UART_FRAMESIZE={framesize}", "flash"], cwd="firmware/", check=True)