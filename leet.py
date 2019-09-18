with open("final.txt") as f:
    lines = f.readlines()
    extendedfile = open("dictleet3.txt", "w+")
    for l in lines:
        extendedfile.write(l)
        if len(l) < 10:
            if "l" in l:
                newLeet = l.replace("l", "!_", 1)
                extendedfile.write(newLeet)
                newLeet = l.replace("l", "1_", 1)
                extendedfile.write(newLeet)
            if "d" in l:
                newLeet = l.replace("d", "[)", 1)
                extendedfile.write(newLeet)
            if "m" in l:
                newLeet = l.replace("m", "|\\/|", 1)
                extendedfile.write(newLeet)
                newLeet = l.replace("m", "/V\\", 1)
                extendedfile.write(newLeet)
            if "h" in l:
                newLeet = l.replace("h", "/-/", 1)
                extendedfile.write(newLeet)
                newLeet = l.replace("h", "}{", 1)
                extendedfile.write(newLeet)
            if "o" in l:
                newLeet = l.replace("o", "<>", 1)
                extendedfile.write(newLeet)
                newLeet = l.replace("o", "0", 1)
                extendedfile.write(newLeet)
            if "a" in l:
                newLeet = l.replace("a", "/-\\", 1)
                extendedfile.write(newLeet)
                newLeet = l.replace("a", "^", 1)
                extendedfile.write(newLeet)
            if "w" in l:
                newLeet = l.replace("w", "\\_//\\_/", 1)
                extendedfile.write(newLeet)
            if "b" in l:
                newLeet = l.replace("b", "|3", 1)
                extendedfile.write(newLeet)
            if "s" in l:
                newLeet = l.replace("s", "$", 1)
                extendedfile.write(newLeet)
            if "k" in l:
                newLeet = l.replace("k", "|(", 1)
                extendedfile.write(newLeet)
            if len(l) < 8:
                l = l.strip('\n')
                newLeet = l + "xor"
                extendedfile.write(newLeet + "\n")
                newLeet = l + "zorz"
                extendedfile.write(newLeet + "\n")
                upperstr = l.upper()
                extendedfile.write(upperstr + "\n")
                for s in l:
                    newLeet = l.replace(s, s.upper(), 1)
                    extendedfile.write(newLeet + "\n")
