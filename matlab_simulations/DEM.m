%% ============================================================
% LoRa Coverage Feasibility – Gateway + Nodes + DEM + SF Rings
% Sri Lanka Example
% Author: Hiran Geeth + ChatGPT
% ============================================================

clear; clc; close all;

%% ------------------ Step 0: Define Gateway & Nodes -----------------
gateway = [6.9500, 79.9173];  % Gateway (Lat,Lon)

% Nodes: 10 domestic nodes, last 3 are far
nodeCoords = [
    6.9490 79.9165;
    6.9510 79.9178;
    6.9520 79.9160;
    6.9485 79.9185;
    6.9475 79.9178;
    6.9505 79.9150;
    6.9530 79.9170;
    6.9465 79.9168;
    6.9600 79.9250;  % Far node 1
    6.9550 79.9300   % Far node 2
];

N = size(nodeCoords,1);

%% ------------------ Step 1: Download DEM -------------------------
% Set this in your local environment; do not commit a personal API key.
apiKey = getenv('OPENTOPOGRAPHY_API_KEY');
if isempty(apiKey) && ~isfile('dem_tile.tif')
    error('Set OPENTOPOGRAPHY_API_KEY before downloading terrain data.');
end

% Bounding box: small area around gateway
latMin = 6.945; latMax = 6.965;
lonMin = 79.915; lonMax = 79.935;

demType = 'SRTMGL1'; % 30m DEM
outputFile = 'dem_tile.tif';

url = sprintf(['https://portal.opentopography.org/API/globaldem?'...
    'demtype=%s&south=%f&north=%f&west=%f&east=%f&outputFormat=GTiff&API_Key=%s'], ...
    demType, latMin, latMax, lonMin, lonMax, apiKey);

disp('Downloading DEM (this may take a while)...');
options = weboptions('Timeout', 120);  % 2-minute timeout

if ~isfile(outputFile)
    try
        websave(outputFile, url, options);
        disp('DEM downloaded successfully.');
    catch ME
        disp('DEM download failed:');
        % Request errors may contain the credential-bearing URL.
        disp('Check your API key, service access, and network connection.');
    end
else
    disp('DEM already downloaded.');
end

%% ------------------ Step 2: Load DEM -----------------------------
try
    [DEM, R] = readgeoraster(outputFile);
    DEM = double(DEM);  % convert to double for calculations
    disp('DEM loaded successfully.');
catch
    warning('Failed to load DEM. Terrain-based calculations will be skipped.');
    DEM = [];
end

%% ------------------ Step 3: Prepare Figure ------------------------
figure('Name','LoRa Coverage Feasibility','NumberTitle','off',...
    'Position',[100 100 1000 700]);
geobasemap('streets');
hold on;

% Plot Gateway
geoscatter(gateway(1), gateway(2), 200, 'r', 'filled', 'DisplayName','Gateway');

% Plot Nodes
nodePlots = gobjects(N,1);
for i = 1:N
    nodePlots(i) = geoscatter(nodeCoords(i,1), nodeCoords(i,2), 80, 'b', 'filled',...
                              'DisplayName', ['Node ' num2str(i)]);
end

% Enable data cursor
dcm = datacursormode(gcf);
set(dcm,'Enable','on','SnapToDataVertex','on','UpdateFcn',@(src,event_obj) myNodeTooltip(event_obj));

%% ------------------ Step 4: SF Coverage Rings ----------------------
% Parameters: LoRa 868MHz, COST-231 Hata, example urban
% SF7–SF12 thresholds in dBm (approx)
SFthresholds = [-123 -126 -129 -132 -134 -137]; % SF7–SF12

% Gateway Tx power, frequency
Pt = 14; % dBm
freq = 868; % MHz

for sf = 1:6
    % Max path loss for this SF
    Lmax = Pt - SFthresholds(sf);
    
    % Approx distance assuming free-space (simple model)
    % More accurate: COST-231/Hata using urban model with terrain
    dkm = 0.1:0.01:5; % distance vector km
    % Free-space path loss
    PL = 32.45 + 20*log10(freq) + 20*log10(dkm*1000/1000); % in dB
    idx = find(PL <= Lmax,1,'last');
    if ~isempty(idx)
        r = dkm(idx);
        % Plot circle
        [latC, lonC] = scircle1(gateway(1), gateway(2), r); % km
        geoplot(latC, lonC, '--','LineWidth',1.5,'DisplayName',['SF' num2str(sf+6)]);
    end
end

title('LoRa Gateway & Nodes with SF Coverage Rings');
legend('Location','best');

%% ------------------ Step 5: DEM Overlay (optional) ----------------
if ~isempty(DEM)
    geoshow(DEM, R, 'DisplayType','surface');
end

%% ------------------ Step 6: Node Tooltip --------------------------
function txt = myNodeTooltip(event_obj)
    target = event_obj.Target;
    pos = event_obj.Position;
    if isprop(target,'DisplayName')
        nodeName = target.DisplayName;
    else
        nodeName = '';
    end
    txt = {nodeName, ['Lat: ' num2str(pos(1))], ['Lon: ' num2str(pos(2))]};
end

