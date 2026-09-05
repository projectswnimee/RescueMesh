%% ================= Load DEM from Local TIFF =================
clc; clear;

% DEM file in the current working folder
demFile = 'DEM_SRTMGL1.tif';

% Check if file exists
if ~isfile(demFile)
    error('DEM file not found in the current folder.');
end

% Read the DEM raster
[DEM, R] = readgeoraster(demFile);

% Convert DEM to double for plotting
DEM = double(DEM);

% Display DEM
figure('Name','DEM Visualization','NumberTitle','off');
geoshow(DEM, R, 'DisplayType', 'surface'); % surface plot
demcmap(DEM);  % apply colormap for elevation
title('DEM loaded from local TIFF');

% Optionally, show a colorbar
colorbar;
ylabel(colorbar,'Elevation (m)');
