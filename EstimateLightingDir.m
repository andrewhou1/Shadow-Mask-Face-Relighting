clear;

% create a 2D grid
%y = linspace(x1,x2,n) generates n points. The spacing between the points is (x2-x1)/(n-1).
th = linspace(0,pi,128);    % inclination
phi = linspace(pi/2,2*pi+pi/2,256); % azimuth
%phi = linspace(0,2*pi,256);

%th = linspace(0,pi,32);
%phi = linspace(0,2*pi,16);
[th,phi] = meshgrid(th,phi);

% compute real spherical harmonic
%Y = HARMONICY(N,M,TH,PHI) computes the surface spherical harmonic of
%   degree N and order M,
Y = zeros(256,128,9); 
%Y = zeros(16,32,9); 
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
%{
figure(1);
[x,y,z] = sph2cart(phi,pi/2-th,abs(Y(:,:,1)));

subplot(121); 
surf(x,y,z,Y(:,:,1));
[x,y,z] = sph2cart(phi,pi/2-th,abs(Y(:,:,2)));
subplot(122); surf(x,y,z,Y(:,:,5));
%}

%Create M matrix for integral on unit sphere
th1 = th(:);
th1 = th1 - pi/2;
length = size(th1,1);
Diag = cos(th1);
p = 1:length;
M = sparse(p,p,Diag,length,length);

%sanity check, the diagonal element should be close to 1 
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

%calculate coefficient for Multipie dataset environment
%{
coefficient = zeros(19,9);
for num = 1
    filename = [num2str(num) 'envir_map.csv'];   
    map = csvread(filename);
    %imshow(map);
   
    %calculate the coefficient
    f = map';
    for i = 1:9
        basis = Y(:,:,i);
        coefficient(num,i) = basis(:)'*M*f(:);
    end
end

writematrix(coefficient,"coefficient.csv");
writematrix(Y,"basis.csv");
%}

%UCSD evaulation using Multi-pie lighting, from SH coefficients to
%environment map
coefficient = csvread('Multi-PIE_coefficients.csv');
for in = 1: 19    
   co =  coefficient(in,:);
   co0 = co(1);
   co1 = co(2);
   co2 = co(3);
   co3 = co(4);
   new_map = Y(:,:,1)*co0 + Y(:,:,2)*co3 + Y(:,:,3)*co2 + Y(:,:,4)*co1;
   
  %{
   new_map = zeros(256,128);
   for i = 1: 9
     new_map = new_map + Y(:,:,i) *coefficient(in,i);
   end
   %}
   
   %figure(1);
   
   %new_map(new_map < 0) = 0;

   %new_map = new_map * 50;
   maximum = max(max(new_map));
   factor = 1/maximum;
   new_map = new_map * factor;
   imshow(new_map');
   filename1 = ['./SH_theta2/' num2str(in) 'test.jpg'];
   imwrite(new_map',filename1);
   %hdrwrite(new_map,'newHDRfile.hdr');
   
end

%lighting direction for DPR dataset
myFolder = 'lighting/';
filePattern = fullfile(myFolder, '*.mat');
matFiles = dir(filePattern);
for it = 1:size(matFiles,1)
    baseFileName = fullfile(myFolder, matFiles(it).name);
    co = load(baseFileName).SH_coefficients;
    co0 = co(1);
    co1 = co(2);
    co2 = co(3);
    co3 = co(4);
    new_map = Y(:,:,1)*co0 + Y(:,:,2)*co3 + Y(:,:,3)*co2 + Y(:,:,4)*co1;
    
    
    maximum = max(max(new_map));
    [i,j] = find(new_map == maximum);
    max_th = th(i,j);
    max_phi = phi(i,j);
    
    minimum = min(min(new_map));
    [i,j] = find(new_map == minimum);

    x = sin(max_th)*cos(max_phi);
    y = sin(max_th)*sin(max_phi);
    z = cos(max_th);

    light_dir = [-z,x,-y];
   
    baseFileName = baseFileName(10:end-4);
    Output = "LightingDir/"+baseFileName;
    fileID = fopen(Output,'w'); 
    fprintf(fileID, '%f %f %f \n', light_dir); 
    fclose(fileID);
    
end

new_map = new_map/maximum;
imshow(new_map);


%{
%reconstruction
sz = size(Y(:,:,1));
f_bar = zeros(sz);
for i = 1:9 
    f_bar = coefficient(i)*Y(:,:,i) + f_bar;
end

f_difference = f - f_bar;
sum = sum(f_difference(:));
figure(2);
subplot(121);
imshow(f');
subplot(122);
imshow(f_bar');
%}

