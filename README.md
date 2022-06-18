# Towards High Fidelity Face Relighting with Realistic Shadows
Andrew Hou, Ze Zhang, Michel Sarkis, Ning Bi, Yiying Tong, Xiaoming Liu. In CVPR, 2021. 

![alt text](https://github.com/andrewhou1/Shadow-Mask-Face-Relighting/blob/main/sample_outputs/00508_lower_right.png)
![alt text](https://github.com/andrewhou1/Shadow-Mask-Face-Relighting/blob/main/sample_outputs/00841_lower_left.png)

The code for this project was developed using Python 3.6.8 and Tensorflow 1.9.0. 

## Trained model
To run our trained model on an input image and a target lighting: 
```
python test_relight_single_image.py input_image_path target_lighting_path output_image_path gpu_id
```
An example of this is provided below: 
```
python test_relight_single_image.py sample_images/01503.png sample_lightings/light_left.txt sample_outputs/01503_left.png 7
```
## Training Code
To retrain the model, first download the data from the following drive link (note that the data within the zip file is about 100GB in total): https://drive.google.com/file/d/1S7iTc_seTkb_6-FX5xYSDkklwzoJYb9N/view?usp=sharing 

After downloading the data, the only additional data we need are the training images. Please download the DPR dataset from https://drive.google.com/drive/folders/10luekF8vV5vo2GFYPRCe9Rm2Xy2DwHkT and the Extended Yale Face Database B from http://vision.ucsd.edu/~leekc/ExtYaleDatabase/ExtYaleB.html (select "original images").

For the DPR dataset: 

```
cd CVPR2021_data/
mkdir uncropped_DPR_images/
mkdir cropped_DPR_images/
```
Next, copy all DPR images in all folders that have a corresponding .mat file in the provided folder DPR_landmarks/ to the uncropped_DPR_images/ folder (e.g. copy the 5 images imgHQ00000_00.png, imgHQ00000_01.png, imgHQ00000_02.png, imgHQ00000_03.png, and imgHQ00000_04.png found in folder imgHQ00000/ to uncropped_DPR_images/ if imgHQ00000.mat exists in DPR_landmarks/). You should have 123,800 images in total in uncropped_DPR_images/ if this is done correctly. Next, run crop_DPR.m to generate the cropped DPR images in the cropped_DPR_images/ folder. 

## Citation 
If you utilize our code in your work, please cite our CVPR 2021 paper. 
```
@inproceedings{ towards-high-fidelity-face-relighting-with-realistic-shadows,
  author = { Andrew Hou and Ze Zhang and Michel Sarkis and Ning Bi and Yiying Tong and Xiaoming Liu },
  title = { Towards High Fidelity Face Relighting with Realistic Shadows },
  booktitle={IEEE/CVF Conference on Computer Vision and Pattern Recognition (CVPR)},
  year = { 2021 }
}
```
    
## Contact 
If there are any questions, please feel free to post here or contact the first author at **houandr1@msu.edu**
