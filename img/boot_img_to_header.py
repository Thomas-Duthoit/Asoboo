from PIL import Image
import numpy as np


def convert_color(r,g,b):
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F

    result = ((r5 << 11) | (g6 << 5) | b5)
    return result

def convert_png_to_c_header(image_path):

    output_file = "include/boot_image.h"


    img = Image.open(image_path).convert('RGB')
    pixels = np.array(img)

    buffer = []

    for y in range(img.height):
        for x in range(img.width):
            r, g, b = pixels[y, x]
            r = int(r)
            g = int(g)
            b = int(b)
            rgb565 = convert_color(r, g, b);
            buffer.append(f"0x{(rgb565 >> 8) & 0xFF:02X}{rgb565 & 0xFF:02X}")

    with open(output_file, "w") as f:
        f.write(f"#ifndef BOOT_IMAGE_H\n#define BOOT_IMAGE_H\n\n")
        
        f.write("#define _BOOT_IMAGE { \\\n    ")

        for i, val in enumerate(buffer):
            if i != 0:
                f.write(",")
            f.write(val)
            if (i + 1) % 10 == 0:  # Saut de ligne tous les 10 pixels
                f.write("\\\n    ")
 
        f.write("}\n")


        f.write("""
#include "graph.h"
        
void _render_boot_img () {
    uint16_t img[SCREEN_WIDTH*SCREEN_HEIGHT] = _BOOT_IMAGE;
    g_draw_buffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img);
}
""") 
 
        f.write("\n\n#endif")



    print(f"Fichier {output_file} généré avec succès !")

# Utilisation
convert_png_to_c_header("img/boot.png")