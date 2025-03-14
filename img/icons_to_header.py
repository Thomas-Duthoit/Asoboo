from PIL import Image
import numpy as np
import os


def convert_color(r,g,b):
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F

    result = ((r5 << 11) | (g6 << 5) | b5)
    return result

def convert_png_to_c_header_string(image_path, idx):

    img = Image.open(image_path).convert('RGB')
    pixels = np.array(img)

    buffer = []

    out = ""

    for y in range(img.height):
        for x in range(img.width):
            r, g, b = pixels[y, x]
            r = int(r)
            g = int(g)
            b = int(b)
            rgb565 = convert_color(r, g, b);
            buffer.append(f"0x{(rgb565 >> 8) & 0xFF:02X}{rgb565 & 0xFF:02X}")

            
    out += "#define ICON_" + "".join(str(os.path.basename(image_path)).upper().split(".")[:-1]) + " " + str(idx) + "\n"
    out += "#define _ICON_" + "".join(str(os.path.basename(image_path)).upper().split(".")[:-1]) +" { \\\n"

    for i, val in enumerate(buffer):
        if i != 0:
            out += ","
        out += val
        if (i + 1) % 8 == 0:  # Saut de ligne tous les 10 pixels
            out += "\\\n"

    out += "}\n\n\n"

    return out



paths = []
for p in os.listdir("img/icons/"):
    if ".png" in p:
        paths.append(p)

with open("include/icons.h", "w") as f:
    f.write("#ifndef ICONS_H\n#define ICONS_H\n\n")

for i, p in enumerate(paths):
    if ".png" in p:
        print("Conversion de", p)
        out = convert_png_to_c_header_string("img/icons/" + p, i)
        with open("include/icons.h", "a") as f:
            f.write(out)

with open("include/icons.h", "a") as f:
    f.write("#include \"graph.h\"\n")
    f.write("void render_icon (uint16_t x, uint16_t y, const int icon) {\n")
    f.write("switch (icon) {\n")

    for i, p in enumerate(paths):
        f.write("\ncase " + str(i) + ": uint16_t img" + str(i) + "[8*8] = _ICON_" + "".join(p.upper().split(".")[:-1]) + "; g_draw_buffer(x, y, 8, 8, img" + str(i) + "); return;\n")

    f.write("default: return;")

    f.write("}\n}\n")
    f.write("#endif")


print("> Fichier include/icons.h généré avec succès !")


