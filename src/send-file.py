# THe purpose of this file is to send a file contents
# over serial communication to an arduino on LINUX
#
# OPens the serial /dev/ttyACM0 on the 9600 and sends
# the file contents specified in the command line arguments
# to the arduino so that it can interpret the code as a .pvb
#
# *Note: this is pretty much useless. Arduino doesn't have enough memory for lexer.

import serial
import time
import sys

if sys.platform == "linux":
    port = "/dev/ttyACM1"
elif sys.platform == "win32":
    port = "COM3"
else:
    print("Unsupported platform")
    exit(1)

def send_file_over_serial():
    with serial.Serial(port, 9600, timeout=1) as ser:
        print(f"Connected to {port} at 9600 baud.")

        with open(sys.argv[1], 'r') as file:
            lines = file.readlines()
        
        print("Sending file contents...")
        for line in lines:
            line = line.strip()
            ser.write(line.encode() + b'\n') 
            ser.flush()
            print(f"Sent: {line}")
            time.sleep(0.1)
        
        end = "EOF\n" # eof
        ser.write(end.encode());

        print("File contents sent successfully!")

if __name__ == "__main__":
    send_file_over_serial()