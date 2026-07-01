import time, os, sys
from media.sensor import *
from media.display import *
from media.media import *
from ybUtils.YbUart import YbUart

uart = YbUart(baudrate=115200)

DISPLAY_WIDTH = 640
DISPLAY_HEIGHT = 480

# Priority order: RED, BLUE, YELLOW, GREEN
COLOR_THRESHOLDS = [
    (9, 63, 31, 73, -11, 57),          # RED
    (3, 35, -10, 8, -9, 3),        # BLUE
    (19, 67, 4, 127, 31, 69),         # YELLOW
    (13, 47, -69, -14, -128, 127),       # GREEN
]

DRAW_COLORS = [
    (255, 0, 0),      # RED
    (0, 0, 255),      # BLUE
    (255, 255, 0),    # YELLOW
    (0, 255, 0),      # GREEN
]

COLOR_LABELS = ['RED', 'BLUE', 'YELLOW', 'GREEN']

AREA_THRESHOLD = 800  # lower = can see farther cubes better

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
    return 0.45 <= ratio <= 1.55   # loose square filter

def score_blob(blob):
    # Pick the blob closest to the center of the screen
    cx = blob.cx()
    cy = blob.cy()
    dx = cx - (DISPLAY_WIDTH // 2)
    dy = cy - (DISPLAY_HEIGHT // 2)
    return dx * dx + dy * dy

def get_best_blob_for_color(img, color_idx):
    blobs = img.find_blobs(
        [COLOR_THRESHOLDS[color_idx]],
        pixels_threshold=AREA_THRESHOLD,
        area_threshold=AREA_THRESHOLD,
        merge=True
    )

    best_blob = None
    best_score = 999999999

    if blobs:
        for blob in blobs:
            x = blob[0]
            y = blob[1]
            w = blob[2]
            h = blob[3]

            if not is_squareish(w, h):
                continue

            s = score_blob(blob)
            if s < best_score:
                best_score = s
                best_blob = blob

    return best_blob

def choose_target(img):
    # Priority: red -> blue -> yellow -> green
    for color_idx in range(4):
        blob = get_best_blob_for_color(img, color_idx)
        if blob is not None:
            return color_idx, blob
    return -1, None

def send_target(color_id, cx, cy, w, h):
    msg = "<{},{},{},{},{}>\n".format(color_id, cx, cy, w, h)
    uart.send(msg)
    print(msg)

def main():
    try:
        sensor = init_sensor()
        init_display()
        sensor.run()

        clock = time.clock()

        while True:
            clock.tick()
            img = sensor.snapshot()

            # Optional visual center marker
            img.draw_cross(DISPLAY_WIDTH // 2, DISPLAY_HEIGHT // 2, color=(255, 255, 255), thickness=2)

            color_id, blob = choose_target(img)

            if blob is not None:
                x = blob[0]
                y = blob[1]
                w = blob[2]
                h = blob[3]
                cx = blob.cx()
                cy = blob.cy()

                # Draw chosen target
                img.draw_rectangle(blob[0:4], thickness=4, color=DRAW_COLORS[color_id])
                img.draw_cross(cx, cy, thickness=2, color=DRAW_COLORS[color_id])
                img.draw_string_advanced(
                    x, y - 35, 30,
                    COLOR_LABELS[color_id],
                    color=DRAW_COLORS[color_id]
                )

                send_target(color_id, cx, cy, w, h)

            else:
                # No target found
                send_target(-1, 0, 0, 0, 0)

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
