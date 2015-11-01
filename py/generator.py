# -*- coding: utf-8 -*-

def ranger():
    print("begin")
    for i in range(3):
        print("before yield", i)
        yield i
        print("after yield", i)
        print("end")


def echo(value=None):
    print("Execution starts when 'next()' is called for the first time.")
    try:
        while True:
            try:
                value = (yield value)
            except Exception as e:
                value = e
    finally:
        print("Don't forget to clean up when 'close()' is called.")


def main():
    print(ranger)
    print(hasattr(ranger, '__call__'))
    g = ranger()
    print(hasattr(g, '__call__'))
    print(g)
    print(next(g))
    print(next(g))

    generator = echo(1)
    print(next(generator))
    print(next(generator))
    print(generator.send(2))
    print(generator.throw(TypeError, "spam"))
    generator.close()


if __name__ == '__main__':
    main()
