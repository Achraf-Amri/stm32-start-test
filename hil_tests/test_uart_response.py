import serial
import time
import sys

def test_uart_response(port='/dev/ttyACM0', baudrate=115200, timeout=5):
    ser = serial.Serial(port, baudrate, timeout=timeout)
    
    # Forcer un reset matériel de la carte via DTR
    ser.setDTR(False)
    time.sleep(0.2)
    ser.setDTR(True)
    
    time.sleep(0.1)
    ser.reset_input_buffer()  # vider le bruit electrique du reset lui-meme, AVANT le vrai message
    
    time.sleep(2.5)  # laisser le temps au firmware de redemarrer et transmettre
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
