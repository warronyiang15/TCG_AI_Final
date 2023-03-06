from random import *
import numpy as np
from itertools import permutations 
  
# Get all permutations of length 2 
# and length 2 
#perm = permutations([1, 2, 3], 2) 
  
# Print the obtained permutations 
#for i in list(perm): 
#    print (i) 

# [1, 2]
# [1, 3]
# [2, 1]

print('hi')
pos = []
for i in range(25):
	pos.append(i)

#np.random.randint(low=4,high=10,size=10)
a = 1 << 63
cnts = 0

while cnts < 100000:
	mp = dict()
	pr_cube = []
	for i in range(6):
		temp = []
		for j in range(25):
			temp.append(np.random.randint(low=1,high=a))
		pr_cube.append(temp)

	ok = True
	for k in range(6):
		perm = permutations(pos, k)
		for arr in list(perm):
			key = 0
			for i in range(len(arr)):
				key ^= pr_cube[i][arr[i]]
			if mp.get(key) == None:
				mp[key] = True
			else:
				ok = False
				break
		if not ok:
			break

	if ok:
		print(pr_cube)
		cnts = cnts + 1
	else:
		print('nono')
	print('-----------------------------------')



	

