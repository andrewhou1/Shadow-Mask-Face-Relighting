
clear;
pi = 3.1415926;
radius = 1;


%FFHQ dataset
flash = zeros(8,3);
flash(1,:) =  [1, 0, -0.4642676300617170626];
flash(2,:) = [0.7071, 0.7071, -0.4642676300617170626];
flash(3,:) = [0, 1, -0.4642676300617170626];
flash(4,:) = [-0.7071, 0.7071, -0.4642676300617170626];
flash(5,:) = [-1, 0, -0.4642676300617170626];
flash(6,:) = [-0.7071, -0.7071, -0.4642676300617170626];
flash(7,:) = [0.7071, -0.7071, -0.4642676300617170626];
flash(8,:) = [0, -1, -0.4642676300617170626];

flash = flash * 2;

sigma = 30*pi/180;

for lighting = 1: 8
    image = zeros(128,256);
    for i = 1:128
        for j = 1:256
               light = flash(lighting,:);
               
               %exchange y and z
               tmp = light(2);
               light(2) = light(3);
               light(3) = tmp;
               
               phi = (j - 1) * 2 * pi / 255 + pi/2;
               theta = (i - 1) * pi / 127;
               v1 = radius * sin(theta) * cos(phi);
               v2 = radius * sin(theta) * sin(phi);
               v3 = radius * cos(theta);

               v = [v1,v2,v3];
               l = light - v; %theta1
               %l = light; %theta2
               

              v_sum = sqrt(v(1)*v(1) + v(2)*v(2) + v(3)*v(3));
              l_sum = sqrt(l(1)*l(1) + l(2)*l(2) + l(3)*l(3));
              l_norm = l/l_sum;
              v_norm = v/v_sum;
              product = dot(l_norm, v_norm);

              if(product>0 || product==0)
                angle = acos(product);
                intensity = exp(-angle*angle/(2*sigma*sigma));
                image(i,j) = intensity;
              end
        end
    end
    imshow(image);
    filename1 = ['./Multipie_envir_map/FFHQ/' num2str(lighting) 'multi.jpg'];
    %filename2 = ['./Multipie_envir_map/' num2str(lighting) 'envir_map.csv'];
    imwrite(image,filename1);
   % writematrix(image,filename2); 
end



%Multipie dataset
flash = zeros(19,3);
eye = [0,0,0.5];
flash(1,:) =  [0,3.75, 0.25];
flash(2,:) = [-1,3.75, 0.25];
flash(3,:) = [-2,3.75, 0.25];
flash(4,:) = [-2.5,2.85, 0.25];
flash(5,:) = [-2.5,1.5, 0.25];
flash(6,:) = [-2.5,0.75, 0.25];
flash(7,:) = [-2.5,0, 0.25];
flash(8,:) = [-2.5, -0.875, 0.25];
flash(9,:) = [-2.5, -1.8, 0.25];
flash(10,:) = [-2.5,-2.9, 0.25];
flash(11,:) = [-2,-3.25, 0.25];
flash(12,:) = [-1,-3.25, 0.25];
flash(13,:) = [0,-3.25, 0.25];
flash(14,:) = [-2.5,2.85, 1.9];
flash(15,:) = [-2.5,1.5, 1.9];
flash(16,:) = [-2.5,0, 1.9];
flash(17,:) = [-2.5,-1.8, 1.9];
flash(18,:) = [-2.5,-3, 1.9];
%flash(19,:) = [0, 0, 0];
%make it a dim frontal light
flash(19,:) = [-2.5, 0, 0.25];

sigma = 30*pi/180;

for lighting = 1: 19
    image = zeros(128,256);
    for i = 1:128
        for j = 1:256
               light = flash(lighting,:);
               phi = 2* pi - (j - 1) * 2 * pi / 255;
               theta = (i - 1) * pi / 127;
               v1 = radius * sin(theta) * cos(phi);
               v2 = radius * sin(theta) * sin(phi);
               v3 = radius * cos(theta);

               v = [v1,v2,v3];
               l = light - v; %theta1
               %l = light; %theta2
               

              v_sum = sqrt(v(1)*v(1) + v(2)*v(2) + v(3)*v(3));
              l_sum = sqrt(l(1)*l(1) + l(2)*l(2) + l(3)*l(3));
              l_norm = l/l_sum;
              v_norm = v/v_sum;
              product = dot(l_norm, v_norm);

              if(product>0 || product==0)
                angle = acos(product);
                intensity = exp(-angle*angle/(2*sigma*sigma));
                image(i,j) = intensity;
              end
        end
    end
    
    if(lighting == 19)
        image = image * 0.2;
    end
    imshow(image);
    filename1 = ['./Multipie_envir_map/30/' num2str(lighting) 'multi.jpg'];
    %filename2 = ['./Multipie_envir_map/' num2str(lighting) 'envir_map.csv'];
    imwrite(image,filename1);
   % writematrix(image,filename2); 
end





