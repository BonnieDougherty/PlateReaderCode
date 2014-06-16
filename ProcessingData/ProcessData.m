function [OD] = ProcessData(filename)

% Read in files from plate text file. 
% Sort into four matrices (Res1, Res2, Res3, Res4) with each column
% representing a well and each row representing a time point. 
fileID = fopen(filename);
data = textscan(fileID, '%2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s %2s');

RES1 = [];
RES2 = [];
RES3 = [];
RES4 = [];

% Calculate time
temp1(:,4) = data{1,1};
temp1(:,3) = data{1,2};
temp1(:,2) = data{1,3};
temp1(:,1) = data{1,4};
time = strcat(temp1(:,1), temp1(:,2), temp1(:,3), temp1(:,4));
time = hex2dec(time);
% time = time - time(1);
clear temp1

holder = data{1,5};
for i = 1:length(holder)
    resistors(i,1) = str2num(holder{i,1});
end

% Convert from hexadecimal to decimal
for i = 6:197
    temp(:,i-5) = data{1,i};
end
for i = 1:96
    temp1(:,i) = strcat(temp(:,2*i), temp(:,2*i-1));
    ODvalues(:,i) = hex2dec(temp1(:,i));
end
clear temp

% Store each resistor. First column is time from the start of the experiment. 
for i = 1:length(data{1,1})
    if resistors(i) == 0
        RES1 = [RES1; time(i) ODvalues(i,:)];
    elseif resistors(i) == 1
        RES2 = [RES2; time(i) ODvalues(i,:)];
    elseif resistors(i) == 2
        RES3 = [RES3; time(i) ODvalues(i,:)];
    elseif resistors(i) == 3
        RES4 = [RES4; time(i) ODvalues(i,:)];
    end
end
clear i

RES4(:,1) = RES4(:,1)-RES4(1,1);

% Check times
if RES4(end,1) == 0
    for i = 2:length(RES4)
        RES4(i,1) = RES4(i-1,1)+120;
    end
end

% Use first 10 reads as blanks
blank4 = mean(RES4(1:10,2:97));

OD(:,1) = RES4(:,1)./3600;
for well = 1:96
    OD(:,well+1) = -log10(RES4(:,well+1)./blank4(well));
end
end

