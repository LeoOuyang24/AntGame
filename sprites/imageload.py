import sys
import cv2 as cv
import numpy as np
import time

start = time.time()
cv.imread(cv.samples.findFile("freeze-icon.png"))
print("Millieconds it took: ", 1000*(time.time() - start))
input("Press enter to continue")