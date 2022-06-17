function paddedIm = facePadding(img, top, left, bottom, right)

% pad the face with nearest neighbor to expand the images 
% for color images, padding is done for each channel separately. 

[row, col, ndims] = size(img);

paddedIm = zeros(row+top+bottom, col+left+right, ndims);
paddedIm = uint8(paddedIm);
for n = 1 : ndims
    img1 = img(:,:,n);
    img2 = zeros(row+top+bottom, col+left+right);
    img2 = uint8(img2);
    img2(top+1:top+row, left+1:left+col) = img1;
    [~, idx] = bwdist(img2);
    paddedIm(:, :, n) = img2(idx);
end

