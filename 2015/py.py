def count(inp):
    res = 0
    for c in inp:
        if c == '(':
            res += 1
        elif c == ')':
            res -= 1
    return res


def main():
    inp = input()

    print("floor: ", count(inp))


main()
