d = csvread('testimg.csv')/255;
[h,w] = size(d);
w = w/3;
img = reshape(d',[3,w,h]);
img = permute(img,[3,2,1]);
imshow(img);
keyboard;
