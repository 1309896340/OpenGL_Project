clear;
clf;
global uSize vSize lightPos lightRadius vd

uSize = 20;
vSize = 20;

set(gcf,"position",[20,20,800,700]);

meshPos = zeros(vSize*uSize,3);
meshIdx = zeros((vSize-1)*(uSize-1),6,"uint32");
for v=1:vSize
    for u=1:uSize
        meshPos((v-1)*uSize + u,:) = meshGen(u,v);
        if u~=uSize && v~=vSize
            meshIdx((v-1)*(uSize-1) + u,:) = [
                (v-1)*uSize + u,...
                (v-1)*uSize + u + 1,...
                v*uSize + u + 1,...
                (v-1)*uSize + u,...
                v*uSize + u + 1,...
                v*uSize + u
            ];
        end
    end
end

lightPos = [0.4,0.4,1];
lightRadius = 0.3;

hold on;
[X,Y,Z] = sphere(30);
X = lightRadius*X + lightPos(1);
Y = lightRadius*Y + lightPos(2);
Z = lightRadius*Z + lightPos(3);
surf(X,Y,Z,"LineStyle","none");
alpha 0.9;

vd = VideoWriter("a.avi");
vd.FrameRate = 10;
open(vd);

for v=1:vSize-1
    for u=1:uSize-1
        for k=1:2
            % 可视化三角形
            triPos = meshPos(meshIdx((v-1)*(uSize-1)+u,(k-1)*3+(1:3)),:);
            curve = [triPos;triPos(1,:)];
            plot3(curve(:,1),curve(:,2),curve(:,3));
        end
    end
end

for v=1:vSize-1
    for u=1:uSize-1
        for k=1:2
            % 发射三角形
            triPos = meshPos(meshIdx((v-1)*(uSize-1)+u,(k-1)*3+(1:3)),:);
            curve = [triPos;triPos(1,:)];
            processTriangle(triPos);
            if v==2 && u==3
                close(vd);
                return;
            end
        end
    end
end

close(vd);


function pos = meshGen(u,v)
    global uSize vSize
    pos = [u/(uSize-1)*2-1,v/(vSize-1)*2-1,0];
end

function processTriangle(triangle)
    global lightPos lightRadius linebuf normbuf vd

    aPos = triangle(1,:);
    bPos = triangle(2,:);
    cPos = triangle(3,:);
    
    ab = norm_v(bPos - aPos);
    ac = norm_v(cPos - aPos);
    normal_v = norm_v(cross(ab, ac));
    centroid = (aPos+bPos+cPos)/3;
    
    % 可视化三角面元上的法向量
    % start_n = centroid;
    % end_n = centroid + normal_v*0.1;
    % curve = [start_n;end_n];
    % plot3(curve(:,1),curve(:,2),curve(:,3),"r");

    sampleNum = 200;      % 球面上的采样点个数
    theta = acos(1 - rand([1,sampleNum]));
    phi = 2*pi*rand([1,sampleNum]);

    x = lightRadius*sin(theta).*cos(phi);
    y = lightRadius*sin(theta).*sin(phi);
    z = lightRadius*cos(theta);
    
    lightDir = norm_v(lightPos - centroid);
    rradius = acos(dot([0,0,1],-lightDir));
    raxis = norm_v(cross([0,0,1],-lightDir));

    % 将[x;y;z;0]向量沿着raxis旋转rradius
    T = makehgtform("axisrotate",raxis,rradius);
    pts = [x;y;z;zeros(1,sampleNum)];
    samplePosAfterRotate = T * pts;
    samplePosAfterRotate = samplePosAfterRotate(1:3,:)' + lightPos;
    normalVector = pts./lightRadius;
    normalAfterRotate = T * normalVector;
    normalAfterRotate = normalAfterRotate(1:3,:)';
    normalEnd = samplePosAfterRotate + normalAfterRotate*0.05;   % sampleNum行3列

    samplePos = samplePosAfterRotate;    % sampleNum行3列
    
    % 可视化射线
    linebuf = cell([1,sampleNum]);
    normbuf = cell([1,sampleNum]);
    for i=1:sampleNum
        curve = [samplePos(i,:);centroid];
        linebuf{i} = plot3(curve(:,1),curve(:,2),curve(:,3),"r");
        curve = [samplePosAfterRotate(i,:);normalEnd(i,:)];
        normbuf{i} = plot3(curve(:,1),curve(:,2),curve(:,3),"b");
    end
    view([30,45]);
    writeVideo(vd, getframe(gcf));
    drawnow;
    % pause(0.05);

    % 根据raxis自动调整视野
    % yaw = atan2(dot(raxis,[0,1,0]),dot(raxis,[1,0,0]))*180/pi;
    % pitch = acos(dot(raxis,[0,0,1]))*180/pi;
    % view([yaw+90,90-pitch+40]);
    for i=1:sampleNum
        delete(linebuf{i});
        delete(normbuf{i});
    end
end

function identity_vec = norm_v(vec)
    % 假设vec为(n,3)的矩阵，意为n个三维向量的行拼接
    identity_vec = zeros(size(vec),"like",vec);
    for i=1:size(vec,1)
        identity_vec(i,:) = vec(i,:)/norm(vec(i,:));
    end
end
