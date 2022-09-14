# Copyright 2022 University of Manchester
#
# Licensed under the Apache License, Version 2.0(the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http:  // www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
from PIL import Image, ImageShow, ImageFilter
import numpy as np


def main(argv):
    # argv = ["lena_RGB.bmp", "lena_RGB.raw", "to_raw", "10"]
    # argv = ["lena_RGB.bmp", "bw_0.raw", "read_raw_BW", "10"]
    # argv = ["4k.bmp", "bw_0.raw", "read_raw_BW", "10"]
    argv = ["4k.bmp", "sobel_0.raw", "read_raw_BW_multi", "10"]
    if (len(argv) < 3):
        raise RuntimeError("Incorrect number of arguments!")
    with Image.open(argv[0]) as img:
        # ImageShow.show(img)

        # print(rgb_im.getpixel((0, 0)))
        #pixel_array = np.array(rgb_im)
        # print(pixel_array.size)
        # for i in range(9):
        #     print(i)
        #     print(hex(pixel_array.flat[i]))

        rgb_im = img.convert('RGB')
        width, height = rgb_im.size

        # TO convert to raw
        if (argv[2] == "to_raw"):
            im_data = rgb_im.tobytes("raw", "BGR", 0, -1)
            with open(argv[1], "wb") as newFile:
                newFile.write(im_data)

        # TO read raw RGB and display
        elif (argv[2] == "read_raw_RGB"):
            with open(argv[1], "rb") as newFile:
                im_data = newFile.read(width * height * 3)
                # loaded_img = Image.frombuffer(
                #     "RGB", (width, height), im_data, 'raw', "RGB", 0, 1)
                loaded_img = Image.frombuffer(
                    "RGB", (width, height), im_data, 'raw', "BGR", 0, -1)
                ImageShow.show(loaded_img)

        # TO convert to BW sobel
        elif (argv[2] == "to_sobel"):
            bw_im = img.convert('L')
            bw_im = bw_im.filter(ImageFilter.FIND_EDGES)
            im_data = bw_im.tobytes("raw")
            width, height = bw_im.size
            with open(argv[1], "wb") as newFile:
                newFile.write(im_data)

        # TO read raw BW and display
        elif (argv[2] == "read_raw_BW"):
            with open(argv[1], "rb") as newFile:
                im_data = newFile.read(width * height)
                loaded_img = Image.frombuffer(
                    "L", (width, height), im_data, 'raw', "L", 0, -1)
                ImageShow.show(loaded_img)
                loaded_img.save("saved.bmp")

        # TO convert to raw RGB "gifs"
        elif (argv[2] == "to_raw_multi"):
            im_data = rgb_im.tobytes("raw", "BGR", 0, -1)
            with open(argv[1], "wb") as newFile:
                for i in range(int(argv[3])):
                    newFile.write(im_data)

        # TO read raw BW "gifs" and display
        elif (argv[2] == "read_raw_BW_multi"):
            with open(argv[1], "rb") as newFile:
                # first = True
                for i in range(int(argv[3])):
                    im_data = newFile.read(width * height)
                    loaded_img = Image.frombuffer(
                        "L", (width, height), im_data, 'raw', "L", 0, -1)
                    loaded_img.save(f"saved{i}.bmp")
                    # if first:
                    #     loaded_img.save(f"{argv[0]}_modified_saved.bmp")
                    #     first = False
                    ImageShow.show(loaded_img)

        else:
            raise RuntimeError("Incorrect operation!")


if __name__ == '__main__':
    # Original image, Target file, Operation, Frame count
    main(sys.argv[1:])
