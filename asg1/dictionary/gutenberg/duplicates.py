with open("13alice.txt") as infile:
    lst = []
    len11 = open("13len.txt", "w+")
    for line in infile:
        words = line.split()
        for word in words:
            if word not in lst:
                lst.append(word)
                wordline = word + "\n"
                len11.write(wordline)
