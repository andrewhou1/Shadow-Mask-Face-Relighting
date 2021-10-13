# Towards High Fidelity Face-Relighting with Realistic Shadows
Andrew Hou, Ze Zhang, Michel Sarkis, Ning Bi, Yiying Tong, Xiaoming Liu. In CVPR, 2021. 

![alt text](https://github.com/andrewhou1/Shadow-Mask-Face-Relighting/blob/main/sample_outputs/00508_lower_right.png)
![alt text](https://github.com/andrewhou1/Shadow-Mask-Face-Relighting/blob/main/sample_outputs/00841_lower_left.png)

The code for this project was developed using Python 3 and Tensorflow 1.9.0. 

## Trained model
To run our trained model on an input image and a target lighting: 
```
python test_relight_single_image.py input_image_path target_lighting_path output_image_path gpu_id
```
An example of this is provided below: 
```
python test_relight_single_image.py sample_images/01503.png sample_lightings/light_left.txt sample_outputs/01503_left.png 7
```
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
If there are any questions, please feel free to post here or contact the authors at {houandr1, zhangze6, ytong, liuxm}@msu.edu, {msarkis, nbi}@qti.qualcomm.com

Thanks!
