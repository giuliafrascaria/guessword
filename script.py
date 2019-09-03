with open("top250.txt") as f:
  lines = f.readlines()
  new = open("top1.txt","w+")
  for l in lines:
      wlist = l.strip().split('\t')
      print(wlist[3])
      word = wlist[3] + "\n"
      new.write(word)
      up = wlist[3].upper()
      if up != wlist[3]:
          upw = up + "\n"
          new.write(upw)
  new.close()
