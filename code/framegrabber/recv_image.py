import serial
import numpy as np
from PIL import Image
import sys
import cv2

# Image parameters
IMAGE_WIDTH = 320
IMAGE_HEIGHT = 240
IMAGE_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT * 2  # 2 bytes per pixel (RGB565 or YUV422)

def yuv422_to_rgb8882(frame):
    """ Convert YUV422 byte array to an RGB888 OpenCV image """
    frame = np.frombuffer(frame, dtype=np.uint8)  # Convert byte buffer to numpy array
    
    # OpenCV expects a single row of interleaved YUYV data
    frame = frame.reshape((IMAGE_HEIGHT, IMAGE_WIDTH * 2))  # Ensure correct YUYV format

    # Convert YUV422 (YUYV) to RGB
    rgb_image = cv2.cvtColor(frame, cv2.COLOR_YUV2RGB_YUYV)

    return rgb_image  # Shape: (H, W, 3)

def yuv422_to_rgb888(frame):
    """ Convert YUV422 byte array to an RGB888 numpy array """
    frame = np.frombuffer(frame, dtype=np.uint8).reshape(IMAGE_HEIGHT, IMAGE_WIDTH * 2)  # YUYV pairs
    frame = np.flipud(frame)

    # Extract Y, U, V components
    Y = frame[:, 0::2]  # Y values
    U = frame[:, 1::4]  # U values (subsampled)
    V = frame[:, 3::4]  # V values (subsampled)

    # Correctly interleave U and V to match Y resolution
    U = np.column_stack((U, U)).reshape(IMAGE_HEIGHT, IMAGE_WIDTH)  # Expand U horizontally
    V = np.column_stack((V, V)).reshape(IMAGE_HEIGHT, IMAGE_WIDTH)  # Expand V horizontally


    # Convert to RGB using standard YUV to RGB conversion
    # Convert to RGB using standard YUV to RGB conversion
    C = Y.astype(np.int16) - 16
    D = U.astype(np.int16) - 128
    E = V.astype(np.int16) - 128

    R = np.clip((298 * C + 409 * E + 128) >> 8, 0, 255)
    G = np.clip((298 * C - 100 * D - 208 * E + 128) >> 8, 0, 255)
    B = np.clip((298 * C + 516 * D + 128) >> 8, 0, 255)

    return np.stack([R, G, B], axis=-1).astype(np.uint8)  # Shape: (H, W, 3)

def yuv422_to_grayscale(frame):
    """ Extracts the Y (luminance) channel from YUV422 to create a grayscale image """
    frame = np.frombuffer(frame, dtype=np.uint8).reshape(IMAGE_HEIGHT, IMAGE_WIDTH * 2)  # YUYV pairs 
    Y = frame[:, 0::2]  # Extract only the Y values (luminance)
    
    return np.stack([Y, Y, Y], axis=-1).astype(np.uint8)  # Convert to 3-channel grayscale image

def rgb565_to_rgb888(frame):
    """ Convert RGB565 byte array to an RGB888 numpy array """
    frame = np.frombuffer(frame, dtype=np.uint16).reshape(IMAGE_HEIGHT, IMAGE_WIDTH)
    frame = np.flipud(frame) 
    
    r = ((frame >> 11) & 0x1F) << 3  # Shift left by 3
    g = ((frame >> 5) & 0x3F) << 2   # Shift left by 2
    b = (frame & 0x1F) << 3          # Shift left by 3

    return np.stack([r, g, b], axis=-1).astype(np.uint8)  # Shape: (H, W, 3)

def save_image(data, filename="output.png"):
    """ Save the RGB888 image as a PNG file using PIL """
    img = Image.fromarray(data, mode="RGB")
    img.save(filename)
    print(f"Image saved as {filename}")

def save_raw_data(data, filename="output.hex"):
    """ Save raw image data as a hex file """
    with open(filename, "wb") as f:
        f.write(data)
    print(f"Raw data saved as {filename}")

def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <serial_port> <format: rgb565/yuv422/gray> [--save-raw]")
        sys.exit(1)

    SERIAL_PORT = sys.argv[1]  # First argument: Serial port
    FORMAT = sys.argv[2].lower()  # Second argument: Data format (rgb565, yuv422, or gray)
    BAUD_RATE = 115200

    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=None)  # Blocking mode

    while True:
        print("Waiting for image data...")
        frame = ser.read(IMAGE_SIZE)  # Block until full image is received
        
        if len(frame) == IMAGE_SIZE:
            save_raw_data(frame, "output.raw")  # Save raw data first
            
            if FORMAT == "rgb565":
                img_data = rgb565_to_rgb888(frame)
            elif FORMAT == "yuv422":
                img_data = yuv422_to_rgb888(frame)
            elif FORMAT == "gray":
                img_data = yuv422_to_grayscale(frame)
            else:
                print("Invalid format! Use 'rgb565', 'yuv422', or 'gray'.")
                break

            save_image(img_data)  # Save as PNG
            break  # Exit after saving one image

    ser.close()

if __name__ == "__main__":
    main()
