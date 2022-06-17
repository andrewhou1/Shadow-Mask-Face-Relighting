subjects = dir("ExtendedYaleB/");
imsize = 256;
subjects = subjects(3:end);

for i=1:length(subjects)
    images = dir(fullfile(sprintf("ExtendedYaleB/%s", subjects(i).name), '*.pgm'));
    %images = images(3:end);
    for j = 1:9
        landmk = load(sprintf("Yale_landmarks/%s_P0%d.mat", subjects(i).name, j-1));
        landmk = landmk.pts;
        landmk = landmk+1;
        for k = 1:65
            img = imread(sprintf("ExtendedYaleB/%s/%s", subjects(i).name, images((j-1)*65+k).name));
            [img1, landmk1, s, x1, y1] = preprocessingAsDaYong_300W(img, landmk, imsize);
            imwrite(img1, sprintf("Yale_cropped_images/%s.png", images((j-1)*65+k).name));
        end
    end
end
