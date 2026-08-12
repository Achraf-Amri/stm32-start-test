import serial
import subprocess
import time
import sys

def test_uart_response(port='/dev/ttyACM0', baudrate=115200, timeout=5,
                        firmware_path='build/Start_test.bin'):
    # Ouvrir le port AVANT le flash, pour capturer le boot immediat
    ser = serial.Serial(port, baudrate, timeout=timeout)
    ser.reset_input_buffer()

    # Le flash lui-meme relance l'execution ("Go to Thumb mode")
    subprocess.run(["st-flash", "write", firmware_path, "0x8000000"], check=True)

    time.sleep(1.5)
    response = ser.readline().decode('utf-8', errors='ignore').strip()
    ser.close()
    print(f"Reponse recue : '{response}'")
    return "BOOT_OK" in response

if __name__ == "__main__":
    result = test_uart_response()
    if result:
        print("Test HIL : PASS")
        sys.exit(0)
    else:
        print("Test HIL : FAIL")
        sys.exit(1)
