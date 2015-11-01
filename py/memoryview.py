# -*- coding: utf-8 -*-

data = bytearray(b'abcefg')
print(data)
v = memoryview(data)
print(v)
print(v.readonly)

v[0] = ord(b'z')
print(data)

v[1:4] = b'123'
print(data)

# v[2:3] = b'spam'
# print(v[2:3])

v[2:6] = b'spam'
print(data)

def f():
    ...
