fstL = (43,758)
fstR = (1132,551)

pin_width = pin_height = 29
pin_distance = 28
pinsL = 14
pinsR = 18

pin_gap_r_d = 62
pin_gap_l_d = 84

pin_gap_r = 1096
pin_gap_l = 1188

coords = []


#Left 
coords.append(fstL)
x = fstL[0]
y = fstL[1]
for i in range(1,pinsL):
  if(i== 8):
    y+=pin_gap_l_d-pin_distance+1
  y+=pin_height+pin_distance
  coords.append((x,y))

#Right
coords.append(fstR)
x = fstR[0]
y = fstR[1]
for i in range(1,pinsR):
  if(i == 10):
    y+=pin_gap_r_d-pin_distance+4
  y+=pin_height+pin_distance
  coords.append((x,y))

for i in range(0,len(coords)):
  print(i+1,":",coords[i])

