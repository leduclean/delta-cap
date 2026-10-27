"""Compute the NN world homography from a chessboard seen over UVC."""

import argparse
import os
import subprocess

import cv2
import numpy as np

# Fix open-cv Warnings
os.environ.setdefault("QT_QPA_PLATFORM", "xcb")
os.environ["QT_QPA_FONTDIR"] = "/usr/share/fonts/liberation"

SCREEN_SIZE = (320, 240)
RED = (0, 0, 255)  # OpenCV is BGR

# NN pixel (u, v) in [0, 1] -> screen pixel.
NN_TO_SCREEN = np.array([[240, 0, 40], [0, 240, 0], [0, 0, 1]], np.float64)


def frames_stream(dev: str):
    """Capture BGR frames and yield frames.
    Since OpenCV's V4L2 backend can't open the board's RGB565 stream, we use ffmpeg converts it for us."""
    w, h = SCREEN_SIZE
    cmd = [
        "ffmpeg",
        "-loglevel",
        "error",
        "-f",
        "v4l2",
        "-input_format",
        "rgb565le",
        "-video_size",
        f"{w}x{h}",
        "-i",
        dev,
        "-f",
        "rawvideo",
        "-pix_fmt",
        "bgr24",
        "-",
    ]
    with subprocess.Popen(
        cmd, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE
    ) as proc:
        assert proc.stdout
        try:
            while True:
                buf = proc.stdout.read(w * h * 3)
                if len(buf) != w * h * 3:
                    break
                yield np.frombuffer(buf, np.uint8).reshape(h, w, 3).copy()
        finally:
            proc.kill()


def world_corners(cols: int, rows: int, square: float) -> np.ndarray:
    """Get Chessboard inner corners in mm, in findChessboardCorners order."""
    # (N, 1, 2) like OpenCV points
    grid = np.mgrid[0:cols, 0:rows].T.reshape(-1, 1, 2)
    return (grid * square).astype(np.float32)


def nn_homography(screen_pts: np.ndarray, world_pts: np.ndarray):
    """Compute screen homography then convert it to the nn homography.
    Also return the reprojection error (rms, max) in mm to judge the fit."""
    # findChessboardCornersSB gives (N, 2), perspectiveTransform wants (N, 1, 2)
    screen_pts = screen_pts.reshape(-1, 1, 2)
    h_screen, _ = cv2.findHomography(screen_pts, world_pts)
    if h_screen is None:
        raise ValueError("homography not found")
    proj = cv2.perspectiveTransform(screen_pts, h_screen)
    err = np.linalg.norm(proj - world_pts, axis=-1)  # mm, one per corner
    h = h_screen @ NN_TO_SCREEN
    return h / h[2, 2], float(np.sqrt(np.mean(err**2))), float(err.max())


def main() -> None:
    parser = argparse.ArgumentParser(description="NN -> world chessboard calibration")
    parser.add_argument("--cam", type=int, default=4, help="/dev/videoN index")
    parser.add_argument(
        "--cols", type=int, default=7, help="inner corners (squares - 1)"
    )
    parser.add_argument("--rows", type=int, default=10, help="inner corners")
    parser.add_argument("--square", type=float, required=True, help="mm")
    args = parser.parse_args()

    print("Enter: compute H when you are satisfied")
    world = world_corners(args.cols, args.rows, args.square)
    for frame in frames_stream(f"/dev/video{args.cam}"):
        found, corners = cv2.findChessboardCornersSB(
            cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY), (args.cols, args.rows)
        )
        if found:
            cv2.drawChessboardCorners(frame, (args.cols, args.rows), corners, found)
            x0, y0 = corners[0].astype(int)
            cv2.circle(frame, (x0, y0), 6, RED, 2)
        cv2.imshow("calib", cv2.resize(frame, None, fx=2, fy=2))

        # Reads the key
        if cv2.waitKey(1) & 0xFF not in (10, 13):  # Enter
            continue
        if not found:
            print("chessboard not found, move it or improve the light")
            continue
        break
    else:
        raise SystemExit("Video stream error")
    h, rms, worst = nn_homography(corners, world)
    print(f"reprojection error: rms {rms:.2f} mm, max {worst:.2f} mm")
    vals = ", ".join(f"{v:.9g}f" for v in h.flatten())
    print(f"static const float H[9] = {{ {vals} }};")
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
