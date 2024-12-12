from PIL import Image, ImageEnhance

def convert_to_rgb565(r, g, b):
    # Convert 8-bit RGB values to 5-6-5 RGB
    r5 = (r >> 3) & 0x1F  # Convert to 5-bit
    g6 = (g >> 2) & 0x3F  # Convert to 6-bit
    b5 = (b >> 3) & 0x1F  # Convert to 5-bit

    # Combine into RGB565 format
    return (r5 << 11) | (g6 << 5) | b5

def image_to_rgb565_array(image_path, contrast_factor=1.0):
    # Open image and convert to RGB mode
    img = Image.open(image_path).convert("RGB")
    
    # Apply contrast adjustment
    enhancer = ImageEnhance.Contrast(img)
    img = enhancer.enhance(contrast_factor)
    
    # Resize or adjust the image dimensions if needed
    img = img.resize((img.width, img.height))  # Example: keep the original size
    
    # Initialize an array to hold the converted pixel data
    rgb565_data = []
    
    # Process each pixel
    for y in range(img.height):
        for x in range(img.width):
            r, g, b = img.getpixel((x, y))
            rgb565 = convert_to_rgb565(r, g, b)
            rgb565_data.append(rgb565)
    
    return rgb565_data, img.width, img.height

def save_as_cpp_array(rgb565_data, width, height, file_name="output.cpp", array_name="create_page"):
    # Start the C++ array string
    cpp_array = "#include <Arduino.h>\n\n"
    cpp_array += f"extern const uint16_t {array_name}[{len(rgb565_data)}] PROGMEM = {{\n"
    
    # Append each pixel data to the array string
    for i, value in enumerate(rgb565_data):
        if i % width == 0:
            cpp_array += "\n    "
        cpp_array += f"0x{value:04X}, "
    
    # Close the array definition
    cpp_array = cpp_array.rstrip(", ")  # Remove the last comma
    cpp_array += "\n};\n"
    
    # Output the C++ array as a file
    with open(file_name, "w") as f:
        f.write(f"// RGB565 Image Data: {width}x{height}\n")
        f.write(f"#define IMAGE_WIDTH {width}\n")
        f.write(f"#define IMAGE_HEIGHT {height}\n")
        f.write(cpp_array)
    
    print(f"Saved as {file_name}")

# Usage:
image_path = "lib/LCD/convert_image/create_page.jpg"
contrast_factor = 1.5  # Adjust contrast to make dark colours darker
rgb565_data, width, height = image_to_rgb565_array(image_path, contrast_factor)
save_as_cpp_array(rgb565_data, width, height, file_name="output.cpp", array_name="create_page")