import time, os, sys
from media.sensor import *
from media.display import *
from media.media import *
from ybUtils.YbUart import YbUart

uart = YbUart(baudrate=115200)

DISPLAY_WIDTH = 640
DISPLAY_HEIGHT = 480

# red, blue, green
COLOR_THRESHOLDS = [
                    (22, 65, 37, 79, -128, 47),          # RED
                    (10, 42, -23, 19, -54, -13),        # BLUE
    (21, 57, -73, -10, -128, 127),       # GREEN
]

DRAW_COLORS = [
    (255, 0, 0),    # RED
    (0, 0, 255),    # BLUE
    (0, 255, 0),    # GREEN
]

COLOR_NAMES = ["RED", "BLUE", "GREEN"]
COLOR_CODES = [1, 2, 3]   # what gets sent to Arduino

AREA_THRESHOLD = 1500

def init_sensor():
    sensor = Sensor()
    sensor.reset()
    sensor.set_framesize(width=DISPLAY_WIDTH, height=DISPLAY_HEIGHT)
    sensor.set_pixformat(Sensor.RGB565)
    return sensor

def init_display():
    Display.init(Display.ST7701, to_ide=True)
    MediaManager.init()

def is_squareish(w, h):
    if h == 0:
        return False
    ratio = w / h
    return 0.5 <= ratio <= 1.5

def blob_score(blob):
    # choose the blob closest to center
    cx = blob.cx()
    cy = blob.cy()
    dx = cx - (DISPLAY_WIDTH // 2)
    dy = cy - (DISPLAY_HEIGHT // 2)
    return dx * dx + dy * dy

def choose_best_cube(img):
    best_blob = None
    best_color_index = -1
    best_score = 999999999

    for color_idx in range(3):
        blobs = img.find_blobs(
            [COLOR_THRESHOLDS[color_idx]],
            pixels_threshold=AREA_THRESHOLD,
            area_threshold=AREA_THRESHOLD,
            merge=True
        )

        if blobs:
            for blob in blobs:
                x = blob[0]
                y = blob[1]
                w = blob[2]
                h = blob[3]

                if not is_squareish(w, h):
                    continue

                score = blob_score(blob)
                if score < best_score:
                    best_score = score
                    best_blob = blob
                    best_color_index = color_idx

    return best_color_index, best_blob

def send_code(code):
    msg = "{}\n".format(code)
    uart.send(msg)
    print("Sent:", msg)

def main():
    try:
        sensor = init_sensor()
        init_display()
        sensor.run()

        clock = time.clock()

        while True:
            clock.tick()
            img = sensor.snapshot()

            img.draw_cross(DISPLAY_WIDTH // 2, DISPLAY_HEIGHT // 2, color=(255, 255, 255), thickness=2)

            color_idx, blob = choose_best_cube(img)

            if blob is not None:
                x = blob[0]
                y = blob[1]
                w = blob[2]
                h = blob[3]
                cx = blob.cx()
                cy = blob.cy()

                img.draw_rectangle(blob[0:4], thickness=4, color=DRAW_COLORS[color_idx])
                img.draw_cross(cx, cy, thickness=2, color=DRAW_COLORS[color_idx])
                img.draw_string_advanced(x, y - 35, 30, COLOR_NAMES[color_idx], color=DRAW_COLORS[color_idx])

                send_code(COLOR_CODES[color_idx])
            else:
                send_code(0)

            fps_text = "FPS: {:.2f}".format(clock.fps())
            img.draw_string_advanced(0, 0, 30, fps_text, color=(255, 255, 255))
            Display.show_image(img)

    except KeyboardInterrupt as e:
        print("User interrupted:", e)
    except Exception as e:
        print("Error:", e)
    finally:
        if 'sensor' in locals() and isinstance(sensor, Sensor):
            sensor.stop()
        Display.deinit()
        MediaManager.deinit()

if __name__ == "__main__":
    main()
