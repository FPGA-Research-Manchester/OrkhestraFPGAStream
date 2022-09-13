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
    # argv = ["lena_RGB.bmp", "bw_0.raw"]
    argv = ["lena_RGB.bmp", "bw_0.raw"]
    with Image.open(argv[0]) as img:
        # ImageShow.show(img)

        # print(rgb_im.getpixel((0, 0)))
        #pixel_array = np.array(rgb_im)
        # print(pixel_array.size)
        # for i in range(9):
        #     print(i)
        #     print(hex(pixel_array.flat[i]))

        # TO convert to raw
        rgb_im = img.convert('RGB')
        # im_data = rgb_im.tobytes("raw", "BGR", 0, -1)
        width, height = rgb_im.size
        # with open(argv[1], "wb") as newFile:
        #     newFile.write(im_data)

        # TO read raw RGB and display
        # with open(argv[1], "rb") as newFile:
        #     im_data = newFile.read()
        #     # loaded_img = Image.frombuffer(
        #     #     "RGB", (width, height), im_data, 'raw', "RGB", 0, 1)
        #     loaded_img = Image.frombuffer(
        #         "RGB", (width, height), im_data, 'raw', "BGR", 0, -1)
        #     ImageShow.show(loaded_img)

        # TO convert to BW sobel
        # bw_im = img.convert('L')
        # bw_im = bw_im.filter(ImageFilter.FIND_EDGES)
        # im_data = bw_im.tobytes("raw")
        # width, height = bw_im.size
        # with open(argv[1], "wb") as newFile:
        #     newFile.write(im_data)

        # TO read raw BW and display
        with open(argv[1], "rb") as newFile:
            im_data = newFile.read()
            loaded_img = Image.frombuffer(
                "L", (width, height), im_data, 'raw', "L", 0, -1)
            ImageShow.show(loaded_img)


if __name__ == '__main__':
    main(sys.argv[1:])
