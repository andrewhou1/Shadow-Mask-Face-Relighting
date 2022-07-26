clear;
myFolder = 'yale_lighting/';
filePattern = fullfile(myFolder, '*.mat');
matFiles = dir(filePattern);
angle = [];

for it = 1:size(matFiles,1)
    baseFileName =  matFiles(it).name;
    a_sign = baseFileName(13);
    if(a_sign == '+')
        a = 1;
    else 
        a = -1;
    end
    a_angle = str2num(baseFileName(14:16));
    a_angle = a*a_angle;
    a_angle = deg2rad(a_angle);
    
    e_sign = baseFileName(18);
    if(e_sign == '+')
        e = 1;
    else
        e = -1;
    end
    e_angle = str2num(baseFileName(19:20));
    e_angle = e*e_angle;
    e_angle = deg2rad(e_angle);
    
    angle = [angle; a_angle, e_angle]; 
end

r = 3.0;

direction = [];
for i = 1:64
    z = r*cos(angle(i,2))*cos(angle(i,1));
    x = -r * cos(angle(i,2))*sin(angle(i,1));
    y = -r * sin(angle(i,2));
    direction = [direction;x,y,z,0];    
end

Output = "yale_lighting.txt";
fileID = fopen(Output,'w'); 
fprintf(fileID, '%f %f %f %f\n', direction'); 
fclose(fileID);


% create a 2D grid
%y = linspace(x1,x2,n) generates n points. The spacing between the points is (x2-x1)/(n-1).
th = linspace(0,pi,128);    % inclination
phi = linspace(0,2*pi,256); % azimuth
[th,phi] = meshgrid(th,phi);

% compute real spherical harmonic
%Y = HARMONICY(N,M,TH,PHI) computes the surface spherical harmonic of
%   degree N and order M,
Y = zeros(256,128,9); 
Y(:,:,1) = harmonicY(0,0,th,phi,'type','real');
Y(:,:,2) = harmonicY(1,-1,th,phi,'type','real');
Y(:,:,3) = harmonicY(1,0,th,phi,'type','real');
Y(:,:,4) = harmonicY(1,1,th,phi,'type','real');
Y(:,:,5) = harmonicY(2,-2,th,phi,'type','real');
Y(:,:,6) = harmonicY(2,-1,th,phi,'type','real');
Y(:,:,7) = harmonicY(2,0,th,phi,'type','real');
Y(:,:,8) = harmonicY(2,1,th,phi,'type','real');
Y(:,:,9) = harmonicY(2,2,th,phi,'type','real');

%normalize the basis
for i = 1:9
    fronorm = norm(Y(:,:,i),'fro');
    Y(:,:,i) = Y(:,:,i)/fronorm;
end

%show SH
figure(1);
[x,y,z] = sph2cart(phi,pi/2-th,abs(Y(:,:,1)));
subplot(121); surf(x,y,z,Y(:,:,4));
[x,y,z] = sph2cart(phi,pi/2-th,abs(Y(:,:,2)));
subplot(122); surf(x,y,z,Y(:,:,5));

%Create M matrix
th1 = th(:);
th1 = th1 - pi/2;
length = size(th1,1);
Diag = cos(th1);
p = 1:length;
M = sparse(p,p,Diag,length,length);

%sanity check 
sanity1 = zeros(9,9);
for i = 1:9
    for j = 1:9
        basis1 = Y(:,:,i);
        basis2 = Y(:,:,j);

        sanity1(i,j) = basis1(:)' * M * basis2(:); 
    end
end 

for i = 1:9
    factor = sqrt(sanity1(i,i));
    Y(:,:,i) = Y(:,:,i)/factor;
end

sanity2 = zeros(9,9);
for i = 1:9
    for j = 1:9
        basis1 = Y(:,:,i);
        basis2 = Y(:,:,j);

        sanity2(i,j) = basis1(:)' * M * basis2(:); 
    end
end 

%calculate SHcoefficient
coefficient = zeros(64,9);

pi = 3.1415926;
sigma = 8*pi/180;
for k = 1:64
    image = zeros(128,256);
    for i = 1:128
        for j = 1:256
            phi = (j - 1)*2*pi/255 - pi;
            theta = (i - 1)*pi/127 - pi/2;
            z = cos(theta)*cos(phi);
            x = -cos(theta)*sin(phi);
            y = -sin(theta);
            v = [x,y,z];
            
            l = direction(k,1:3);
            l = l - v ;
            
            v_sum = sqrt(v(1)*v(1)+v(2)*v(2)+v(3)*v(3));
            l_sum = sqrt(l(1)*l(1)+l(2)*l(2)+l(3)*l(3));
            l_norm = l/l_sum;
            v_norm = v/v_sum;
            product = dot(l_norm,v_norm);
            
            if(product>0 || product==0)
                angle = acos(product);
                intensity = exp(-angle*angle/(2*sigma*sigma));
                image(i,j) = intensity;
            end            
        end
    end
    
    imshow(image);
    f = image';
    for in = 1:9
        basis = Y(:,:,in);
        coefficient(k,in) = basis(:)'*M*f(:);
    end   
    stop =1;   
end

writematrix(coefficient,"coefficient.csv");
writematrix(Y,"basis.csv");



