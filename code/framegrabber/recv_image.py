import serial
import numpy as np
import cv2
import sys

# Image dimensions
IMAGE_WIDTH = 320
IMAGE_HEIGHT = 240
IMAGE_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT * 2  # 2 bytes per pixel (RGB565)

def rgb565_to_rgb888(frame):
    """ Convert RGB565 byte array to RGB888 numpy array """
    frame = np.frombuffer(frame, dtype=np.uint16).reshape(IMAGE_HEIGHT, IMAGE_WIDTH)
    
    r = ((frame >> 11) & 0x1F) * 255 // 31
    g = ((frame >> 5) & 0x3F) * 255 // 63
    b = (frame & 0x1F) * 255 // 31
    
    return np.stack([b, g, r], axis=-1).astype(np.uint8)

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <serial_port>")
        sys.exit(1)

    SERIAL_PORT = sys.argv[1]  # First argument: Serial port
    BAUD_RATE = 115200

    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    
    while True:
        frame = ser.read(IMAGE_SIZE)
        if len(frame) == IMAGE_SIZE:
            img = rgb565_to_rgb888(frame)
            cv2.imshow("Received Image", img)
        
        if cv2.waitKey(1) == 27:  # Press 'ESC' to exit
            break

    ser.close()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
