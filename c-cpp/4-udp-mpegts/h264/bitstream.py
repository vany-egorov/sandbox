#!/usr/bin/python3
# pip3 install bitstream
from bitstring import BitStream


DATA = (
    b"\x88\x80\x20\x00\x2F\xFF\xFE\xE6\x80\x03\x0A\x7C\x0A\x6C\xAC",
    b"\x9A\x06\x0C\x2C\x97\xFF\x00\x00\x03\x00\x00",
    b"\x9E\x07\x0A\x95\x25\xFF\x00\x00\x03\x00\x00",
)


def bits(byte: int) -> int:
    for i in range(8):
        yield (byte >> i) & 0x01


def main():
    for datum in DATA:
        as_hex = ":".join("{:02x}".format(h) for h in datum)
        as_bin = ":".join("{:08b}".format(h) for h in datum)

        print(as_hex)
        print(as_bin)

        a = BitStream(datum)
        first_mb_in_slice = a.read('ue')
        slice_type = a.read('ue')
        pic_parameter_set_id = a.read('ue')
        frame_num = a.read(9)
        print("first-mb-in-slice: {}".format(first_mb_in_slice))
        print("slice-type: {}".format(slice_type))
        print("pic-parameter-set-id: {}".format(pic_parameter_set_id))
        print("frame-num: {}".format(frame_num.int))


if __name__ == "__main__":
    main()
