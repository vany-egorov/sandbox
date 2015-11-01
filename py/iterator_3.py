# -*- coding: utf-8 -*-
class Iterator:
    def __init__(self, s):
        self.s = s[::-1]
        self.i = 0

    def __iter__(self):
        return self

    def __next__(self):
        i = self.i
        if i >= len(self.s):
            raise StopIteration
        self.i += 1
        return self.s[i]

class Container:
    def __init__(self, s):
        self.iterator = Iterator(s)

    def __iter__(self):
        return self.iterator


def main():
    print("".join((r for r in Container("hello world"))))

    x = iter([1, 2, 3])
    print(x)
    print(next(x))
    print(next(x))
    print(next(x))
    print(next(x))


if __name__ == "__main__":
    main()
