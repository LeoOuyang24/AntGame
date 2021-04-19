import cv2 as cv
import numpy as np
import sys
import functools as tools

def main(filename,newName):
    img = cv.imread(filename, cv.IMREAD_UNCHANGED)
    height = int(len(img))
    width = int(len(img[0]))
    copy = []
    for i in range(height):
        for j in range(width):
            if (img[i][j][3] > 255/2):
                copy.append(list(img[i]))
                break
            #cv.imwrite(direct + "/" + name + str(i*size + j) + '.png',img[i*width:i*width+width,j*height:(j+1)*height]);
            #cv.imshow("ASD",temp);
            #cv.waitKey(0);
    i = 0
    while i < len(copy[0]):
        if tools.reduce(lambda x, y : x and y[i][3] <= 255/2, copy, True): #whole column is blank
            for j in copy:
                j.pop(i)
            i -= 1
        i += 1
        
    cv.imwrite(newName,np.array(copy))
if __name__ == "__main__":
    if (len(sys.argv) < 2):
        print("usage: python trim.py <image path> <new file name optional>")
    else:
        if (len(sys.argv) < 3): #if they didn't provide a new file name, use the old name
            sys.argv.append(sys.argv[1])
        main(sys.argv[1],sys.argv[2])   