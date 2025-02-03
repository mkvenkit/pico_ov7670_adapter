import serial
import numpy as np
from PIL import Image
import sys

# Image parameters
IMAGE_WIDTH = 320
IMAGE_HEIGHT = 240
IMAGE_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT * 2  # 2 bytes per pixel (RGB565)

def rgb565_to_rgb888(frame):
    """ Convert RGB565 byte array to an RGB888 numpy array """
    frame = np.frombuffer(frame, dtype=np.uint16).reshape(IMAGE_HEIGHT, IMAGE_WIDTH)
    
    r = ((frame >> 11) & 0x1F) << 3  # Shift left by 3
    g = ((frame >> 5) & 0x3F) << 2   # Shift left by 2
    b = (frame & 0x1F) << 3          # Shift left by 3

    return np.stack([r, g, b], axis=-1).astype(np.uint8)  # Shape: (H, W, 3)

def save_image(data, filename="output.png"):
    """ Save the RGB888 image as a PNG file using PIL """
    img = Image.fromarray(data, mode="RGB")
    img.save(filename)
    print(f"Image saved as {filename}")

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <serial_port>")
        sys.exit(1)

    SERIAL_PORT = sys.argv[1]  # First command-line argument
    BAUD_RATE = 115200

    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=None)  # Blocking mode

    while True:
        print("Waiting for image data...")
        frame = ser.read(IMAGE_SIZE)  # Block until full image is received
        
        if len(frame) == IMAGE_SIZE:
            img_data = rgb565_to_rgb888(frame)
            save_image(img_data)  # Save as PNG
            break  # Exit after saving one image

    ser.close()

if __name__ == "__main__":
    main()
