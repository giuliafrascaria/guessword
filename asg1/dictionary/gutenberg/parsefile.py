with open("theprince.txt") as infile:
    lst = []
    len11 = open("11alice.txt", "a")
    len13 = open("13alice.txt", "a")
    for line in infile:
        words = line.split()
        for word in words:
            if word not in lst:
                lst.append(word)    # append only this word to the list, not all words on this line
                if len(word) == 11:
                    if "," not in word:
                        if ";" not in word:
                            if "!" not in word:
                                if "-" not in word:
                                    if "." not in word:
                                        if "'" not in word:
                                            if "?" not in word:
                                                if ":" not in word:
                                                    if "_" not in word:
                                                        wordline = word.lower() + "\n"
                                                        len11.write(wordline)
                if len(word) == 13:
                    if "," not in word:
                        if ";" not in word:
                            if "!" not in word:
                                if "-" not in word:
                                    if "." not in word:
                                        if "'" not in word:
                                            if "?" not in word:
                                                if ":" not in word:
                                                    if "_" not in word:
                                                        wordline = word.lower() + "\n"
                                                        len13.write(wordline)
    #lst.sort()
    #print(lst)
