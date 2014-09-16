from pylab import *
A = rand(5,5)
figure(1)
imshow(A, interpolation='nearest')
grid(True)

figure(2)
imshow(A, interpolation='bilinear')
grid(True)

figure(3)
imshow(A, interpolation='bicubic')
grid(True)

show()
