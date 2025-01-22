# THe purpose of this file is to send a file contents
# over serial communication to an arduino on LINUX
#
# OPens the serial /dev/ttyACM0 on the 9600 and sends
# the file contents specified in the command line arguments
# to the arduino so that it can interpret the code as a .pvb

import serial
import time
import sys

if sys.platform == "linux":
    port = "/dev/ttyACM0"
elif sys.platform == "win32":
    port = "COM3"
else:
    print("Unsupported platform")
    exit(1)

def send_file_over_serial():
    # Get file path passed from command line arguments
    if len(sys.argv) - 1 < 1 or len(sys.argv) - 1 > 1:
        print(f"Incorrect number of arguments passed. 1 required, {len(sys.argv)-1} passed.")
        exit(1)

    file = sys.argv[1]

    try:
        ser = open(serial.Serial("/dev/ttyACM0", 9600))
        print("Serial port opened.")
        with open(file, "r") as f:
            lines = f.readlines()
    except FileNotFoundError and serial.SerialException:
        print("Failed to open. File not found.")
        exit(1)

            
    for line in lines:
        line = line.strip()
        ser.write(line.encode() + b'\n')
        ser.flush()
        time.sleep(0.1)

    ser.write("EOF".encode());
    print("Successfully sent file")

if __name__ == "__main__":
    send_file_over_serial()
