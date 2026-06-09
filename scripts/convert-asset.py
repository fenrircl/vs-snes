#!/usr/bin/env python3
import os
import sys
import subprocess
from PIL import Image

def main():
    if len(sys.argv) < 3:
        print("Uso: python3 convert-asset.py <imagen_origen> <tipo: sprite|bg> [ancho] [alto]")
        print("Ejemplo: python3 convert-asset.py sprites/Animated-Pipeestrello.gif sprite")
        sys.exit(1)

    input_path = sys.argv[1]
    asset_type = sys.argv[2].lower()

    if not os.path.exists(input_path):
        print(f"Error: El archivo de origen '{input_path}' no existe.")
        sys.exit(1)

    # Cargar imagen (puede ser estática o animada como un GIF)
    img = Image.open(input_path)
    w, h = img.size
    print(f"Imagen cargada: {input_path} ({w}x{h}px, modo: {img.mode})")

    frames = []
    is_animated = False

    # Si es un GIF, extraer frames de animación
    if input_path.lower().endswith('.gif'):
        is_animated = True
        try:
            while True:
                frames.append(img.convert('RGBA'))
                img.seek(img.tell() + 1)
        except EOFError:
            pass
        print(f"Detectados {len(frames)} frames de animación en el GIF.")
    else:
        frames.append(img.convert('RGBA'))

    # Determinar dimensiones objetivo para un frame individual (multiplos de 8)
    first_frame_w, first_frame_h = frames[0].size
    target_w = first_frame_w
    target_h = first_frame_h

    if len(sys.argv) >= 5:
        target_w = int(sys.argv[3])
        target_h = int(sys.argv[4])
    else:
        if asset_type == "sprite":
            # Forzar 16x16 para todos los sprites del juego para coincidir con el renderizado
            target_w = target_h = 16
        else:
            # Redondear al múltiplo de 8 más cercano para fondos
            target_w = (first_frame_w // 8) * 8
            target_h = (first_frame_h // 8) * 8

    if target_w <= 0 or target_h <= 0:
        print("Error: Las dimensiones objetivo deben ser mayores a cero.")
        sys.exit(1)

    # Procesar cada frame (redimensionar si es sprite/fondo para encajar en target)
    processed_frames = []
    for idx, frame in enumerate(frames):
        fw, fh = frame.size
        if asset_type == "sprite":
            # Caso especial para Antonio Belpaese (detectado por nombre o tamaño original)
            if "Antonio" in input_path or (fw == 192 and fh == 198):
                # Antonio original es 32x33 pixel art, escalado a 192x198 (6x)
                # Primero deshacemos la escala de 6x para recuperar el pixel art 1:1 limpio
                clean_w = fw // 6
                clean_h = fh // 6
                clean_img = frame.resize((clean_w, clean_h), Image.Resampling.NEAREST)
                
                # Luego recortamos/ajustamos a 32x32
                temp_32 = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
                temp_32.paste(clean_img, (0, 0))
                
                # Escalar al target final (si es 16x16, se reduce de forma limpia al 50%)
                if (target_w, target_h) == (16, 16):
                    new_img = temp_32.resize((16, 16), Image.Resampling.NEAREST)
                else:
                    new_img = temp_32.resize((target_w, target_h), Image.Resampling.NEAREST)
                processed_frames.append(new_img)
            else:
                # Para otros sprites (como Pipeestrello de 19x21)
                # Si caben dentro del target sin escalar, los centramos
                if fw <= target_w and fh <= target_h:
                    new_img = Image.new("RGBA", (target_w, target_h), (0, 0, 0, 0))
                    dx = (target_w - fw) // 2
                    dy = (target_h - fh) // 2
                    new_img.paste(frame, (dx, dy))
                    processed_frames.append(new_img)
                else:
                    # Si no caben, los reescalamos manteniendo el aspecto y los centramos
                    aspect = fw / fh
                    if aspect > 1:
                        new_w = target_w
                        new_h = int(target_w / aspect)
                    else:
                        new_h = target_h
                        new_w = int(target_h * aspect)
                    temp_img = frame.resize((new_w, new_h), Image.Resampling.NEAREST)
                    new_img = Image.new("RGBA", (target_w, target_h), (0, 0, 0, 0))
                    dx = (target_w - new_w) // 2
                    dy = (target_h - new_h) // 2
                    new_img.paste(temp_img, (dx, dy))
                    processed_frames.append(new_img)
        else:
            # Para fondos
            if (fw, fh) != (target_w, target_h):
                new_img = frame.resize((target_w, target_h), Image.Resampling.NEAREST)
                processed_frames.append(new_img)
            else:
                processed_frames.append(frame)

    # Si es animada, concatenar todos los frames procesados horizontalmente (Sprite Sheet)
    if is_animated and len(processed_frames) > 1:
        sheet_w = target_w * len(processed_frames)
        sheet_h = target_h
        # Para que los sprites de 16x16 se alineen con la cuadrícula de VRAM de 16 tiles de ancho de la SNES (128 píxeles),
        # debemos forzar que el ancho de la hoja sea un múltiplo de 128 píxeles.
        if sheet_w < 128:
            sheet_w = 128
        print(f"Creando Sprite Sheet horizontal de {sheet_w}x{sheet_h} px con {len(processed_frames)} frames...")
        sprite_sheet = Image.new("RGBA", (sheet_w, sheet_h), (0, 0, 0, 0))
        for idx, frame in enumerate(processed_frames):
            sprite_sheet.paste(frame, (idx * target_w, 0))
        img = sprite_sheet
    else:
        img = processed_frames[0]

    # Convertir a 16 colores indexados (4 bits) asegurando transparencia en el color 0
    print("Reduciendo colores a 16 (4bpp)...")
    
    # 1. Separar los canales RGB de la máscara alfa
    r, g, b, a = img.split()
    rgb_img = Image.merge("RGB", (r, g, b))
    
    # 2. Cuantizar la imagen RGB a 15 colores (dejando 1 para transparencia)
    quantized = rgb_img.quantize(colors=15, method=Image.Quantize.MAXCOVERAGE)
    
    # 3. Crear una nueva imagen de paleta de 16 colores donde el color 0 sea transparente
    # Obtener la paleta de 15 colores existente
    palette = quantized.getpalette()[:15*3]
    # Insertar un color magenta/negro en la posición 0 para representar transparencia
    full_palette = [0, 0, 0] + palette + [0]*(256*3 - 16*3)
    
    # Crear la imagen indexada final
    img_p = Image.new("P", img.size)
    img_p.putpalette(full_palette)
    
    # 4. Mapear cada píxel: si alpha es 0 -> color 0, de lo contrario -> color original cuantizado + 1
    width, height = img.size
    px_in = quantized.load()
    px_alpha = a.load()
    px_out = img_p.load()
    for y in range(height):
        for x in range(width):
            if px_alpha[x, y] < 128:
                px_out[x, y] = 0
            else:
                px_out[x, y] = px_in[x, y] + 1
                
    # Guardar como PNG indexado temporal en la subcarpeta correspondiente de gfx/
    basename = os.path.splitext(os.path.basename(input_path))[0].replace("-", "_")
    
    # Determinar subcarpeta basada en la ruta original
    subfolder = ""
    if "personajes" in input_path:
        subfolder = "personajes/"
    elif "fondos" in input_path:
        subfolder = "fondos/"
    elif "enemigos" in input_path:
        subfolder = "enemigos/"
        
    out_dir = f"gfx/{subfolder}"
    temp_png = f"{out_dir}{basename}_indexed.png"
    
    os.makedirs(out_dir, exist_ok=True)
    img_p.save(temp_png)
    print(f"Imagen indexada guardada temporalmente en: {temp_png}")

    # Ubicación de gfx4snes
    pvsneslib_home = os.environ.get("PVSNESLIB_HOME", "/home/fenrir/snesdev/pvsneslib")
    gfx4snes_path = os.path.join(pvsneslib_home, "devkitsnes/tools/gfx4snes")

    if not os.path.exists(gfx4snes_path):
        gfx4snes_path = "gfx4snes"

    # Construir comando de conversión
    cmd = [
        gfx4snes_path,
        "-s", "8",        # Tamaño de tile 8x8
        "-o", "16",       # 16 colores (4bpp)
        "-p",             # Paleta (.pal)
    ]

    if asset_type == "bg":
        cmd.append("-m")  # Generar mapa (.map)
        
    cmd.extend([
        "-e", "0",        # Offset de paleta 0
        "-i", temp_png
    ])

    print(f"Ejecutando: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        print("Conversión completada con éxito por gfx4snes:")
        print(result.stdout)
        
        # Comentado para conservar el PNG optimizado
        # if os.path.exists(temp_png):
        #     os.remove(temp_png)
            
        print(f"\nArchivos generados en '{out_dir}':")
        for ext in [".pic", ".pal", ".map", ".inc", "_data.as"]:
            check_file = f"{out_dir}{basename}_indexed{ext}"
            if os.path.exists(check_file):
                print(f" - {check_file}")
            else:
                alt_file = f"{out_dir}{basename}{ext}"
                if os.path.exists(alt_file):
                    print(f" - {alt_file}")
                    
    except FileNotFoundError:
        print(f"Error: No se encontró la herramienta gfx4snes en '{gfx4snes_path}'.")
        print("Asegúrate de configurar la variable de entorno PVSNESLIB_HOME correctamente.")
        sys.exit(1)
    except subprocess.CalledProcessError as e:
        print("Error al ejecutar gfx4snes:")
        print(e.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
