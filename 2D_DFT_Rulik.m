clear,clc
Img=double((imread(uigetfile("*.pgm"))));
prompt="Low Pass Filter Cut Out Frequency: ";
Q0 = input(prompt);
figure(1);
imshow(cast(Img,"uint8"));
[Img_M, Img_N] = size(Img);
P=2*Img_M-1;
Q=2*Img_N-1;
swM=0;
swN=0;
if(~mod(Img_M,2))
    Img_M = Img_M + 1;
    swM=1;
end
if(~mod(Img_N,2))
    Img_N = Img_N + 1;
    swN=1;
end
A=cos(0:pi:(Img_M*Img_N-1)*pi);
CosMat=reshape(A,[Img_M,Img_N]);
if(swM==1)
    CosMat(end,:)=[];
    Img_M = Img_M - 1;
end
if(swN==1)
    CosMat(:,end)=[];
    Img_N = Img_N -1;
end
ImgC=Img.*CosMat;
% ImgC=cast(ImgC,"uint8");
ImgPad=zeros(P,Q);
ImgPad([1:Img_M],[1:Img_N])=ImgC;
% ImgPad=cast(ImgPad,"uint8");
figure(2);
imshow(cast(ImgPad,"uint8"));
[ImgFRe,ImgFIm]=DFT(ImgPad,P,Q);
disp("DFT done");
figure(3);
imshow(cast(ImgFRe,"uint8"));
LPFMat = LPF(P,Q,Q0);
ImgFReLP=ImgFRe.*LPFMat;
figure(4);
imshow(cast(ImgFReLP,"uint8"));
% ImgFRe=cast(ImgFRe,"uint8");
[ImgBackRe,ImgBackIm]=iDFT(ImgFReLP,P,Q);
disp("iDFT done");
FINAL=ImgBackRe([1:Img_M],[1:Img_N]);
FINAL=FINAL.*CosMat;
figure(5);
imshow(cast(FINAL,"uint8"));


function [re,im] = DFT(img,M,N)
reRow = zeros(M,N);
imRow = zeros(M,N);
re = zeros(M,N);
im = zeros(M,N);
%rows
for (y=1:N)
    for(u=1:M)
        for(x=1:M)
            reRow(u,y)=reRow(u,y)+img(x,y)*cos((2*pi*u*x)/M);
            imRow(u,y)=imRow(u,y)-img(x,y)*sin((2*pi*u*x)/M);
        end
    end
end
% columns
for (x=1:M)
    for(v=1:N)
        for(y=1:N)
            re(x,v)=re(x,v)+reRow(x,y)*cos((2*pi*v*y)/N)+imRow(x,y)*sin((2*pi*v*y)/N);
            im(x,v)=im(x,v)-reRow(x,y)*sin((2*pi*v*y)/N)+imRow(x,y)*cos((2*pi*v*y)/N);
        end
    end
end

end

function [H] = LPF(P,Q,D0)
for(u=1:P)
    for(v=1:Q)   
    D(u,v)=sqrt((u-P/2)^2 +(v-Q/2)^2);
        if(D(u,v)<=D0)
            H(u,v)=1;
        else
            H(u,v)=0;
        end
    end
end
end

function [re,im] = iDFT(imgF,M,N)
reRow = zeros(M,N);
imRow = zeros(M,N);
re = zeros(M,N);
im = zeros(M,N);
%rows
for (v=1:N)
    for(x=1:M)
        for(u=1:M)
            reRow(x,v)=reRow(x,v)+imgF(u,v)*cos((2*pi*u*x)/M);
            imRow(x,v)=imRow(x,v)+imgF(u,v)*sin((2*pi*u*x)/M);
        end
    end
end
% columns
for (u=1:M)
    for(y=1:N)
        for(v=1:N)
            re(u,y)=re(u,y)+reRow(u,v)*cos((2*pi*v*y)/N)-imRow(u,v)*sin((2*pi*v*y)/N);
            im(u,y)=im(u,y)-reRow(u,v)*sin((2*pi*v*y)/N)-imRow(u,v)*cos((2*pi*v*y)/N);
        end
    end
end
re=re./(M*N);
im=im./(M*N);
end