images = dir("uncropped_DPR_images/");
landmarks = dir("DPR_landmarks/");
imsize = 256;
images = images(3:end);
landmarks = landmarks(3:end);
for i=1:length(images)
    i
    curr_image = imread(sprintf("uncropped_DPR_images/%s", images(i).name));
    landmk = load(sprintf("DPR_landmarks/%s", landmarks(floor((i-1)/5)+1).name));
    landmk = landmk.pts;
    landmk = landmk+1;
    [img1, landmk1, s, x1, y1] = preprocessingAsDaYong_300W(curr_image, landmk, imsize);
    imwrite(img1, sprintf("cropped_DPR_images/%s", images(i).name));
end
