with open("top250.txt") as f:
  lines = f.readlines()
  new = open("top.txt","w+")
  for l in lines:
      wlist = l.strip().split('\t')
      print(wlist[3])
      word = wlist[3] + "\n"
      new.write(word)
  new.close()
