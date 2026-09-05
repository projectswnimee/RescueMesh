%% ============================================================
% LoRa Connectivity Feasibility with DEM Terrain Adjustment
% Uses Streets Map, COST231-Hata, SF Coverage Rings
% ============================================================

clear; clc; close all;

%% ------------------ Gateway Location -----------------------
gateway = [6.9500, 79.9173];   % Sri Lanka

%% ------------------ Generate Node Positions ----------------
nodeCoords = [
    6.9489 79.9165;
    6.9511 79.9180;
    6.9520 79.9159;
    6.9495 79.9192;
    6.9478 79.9183;
    6.9507 79.9148;
    6.9530 79.9172;  % last "near" node
    6.9600 79.9300;
    6.9650 79.9400;
    6.9700 79.9550
];

N = size(nodeCoords,1);

%% ------------------ LoRa Parameters -------------------------
f = 915;          % MHz
hb = 30;          % Gateway antenna height (m)
hm = 1.5;         % Node antenna height (m)
Pt_dBm = 14;      % Tx power
Gt = 0; Gr = 0;   % Antenna gains

SF_sens = containers.Map( ...
    {'SF7','SF8','SF9','SF10','SF11','SF12'}, ...
    [-123, -126, -129, -132, -135, -137] );

%% ------------------ COST231-Hata Function ------------------
function L = cost231_hata(d_km, f, hb, hm)
    if d_km < 0.001, d_km = 0.001; end
    Cm = 8; % urban
    L = 46.3 + 33.9*log10(f) + 13.82*log10(hb) ...
        - (1.1*log10(f)-0.7)*hm + (1.56*log10(f)-0.8) ...
        + (44.9 - 6.55*log10(hb))*log10(d_km) + Cm;
end

%% ------------------ Haversine Distance ---------------------
function dkm = haversine(lat1,lon1,lat2,lon2)
    R = 6371; % km
    dlat = deg2rad(lat2-lat1);
    dlon = deg2rad(lon2-lon1);
    a = sin(dlat/2).^2 + cosd(lat1).*cosd(lat2).*sin(dlon/2).^2;
    dkm = 2*R*asin(sqrt(a));
end

%% ------------------ Load DEM --------------------------------
demFile = 'DEM_SRTMGL1.tif'; % put DEM TIFF in current folder
[DEM, R] = readgeoraster(demFile);
DEM = double(DEM); % ensure double for calculations
% DEM grid to lat/lon
latGrid = linspace(R.LatitudeLimits(1), R.LatitudeLimits(2), size(DEM,1));
lonGrid = linspace(R.LongitudeLimits(1), R.LongitudeLimits(2), size(DEM,2));

%% ------------------ Node RSSI & Best SF --------------------
RSSI = zeros(N,1);
dist_km = zeros(N,1);

for i = 1:N
    % Distance
    dist_km(i) = haversine(nodeCoords(i,1), nodeCoords(i,2), ...
                           gateway(1), gateway(2));
    
    % Terrain adjustment: sample DEM along path
    numSamples = 50;
    lats = linspace(gateway(1), nodeCoords(i,1), numSamples);
    lons = linspace(gateway(2), nodeCoords(i,2), numSamples);
    elev = interp2(lonGrid, latGrid, DEM, lons, lats); % meters
    % Use mean terrain elevation difference to adjust hm
    hm_eff = hm + mean(elev);
    hb_eff = hb + DEM(round(end/2), round(end/2)); % Gateway terrain
    
    % Path loss using COST231-Hata with terrain adjusted
    L = cost231_hata(dist_km(i), f, hb_eff, hm_eff);
    RSSI(i) = Pt_dBm + Gt + Gr - L;
end

bestSF = strings(N,1);
for i = 1:N
    sf_list = ["SF7","SF8","SF9","SF10","SF11","SF12"];
    fail = false;
    for sf = sf_list
        if RSSI(i) >= SF_sens(char(sf))
            bestSF(i) = sf;
            fail = true;
            break;
        end
    end
    if ~fail
        bestSF(i) = "NO LINK";
    end
end

%% ------------------ Plot Map -------------------------------
figure('Position',[200 100 900 600]);
geobasemap streets; hold on;

% Plot gateway
geoscatter(gateway(1), gateway(2), 200, 'r', 'filled', 'DisplayName','Gateway');

% Plot nodes with result coloring
colors = zeros(N,3);
for i = 1:N
    if bestSF(i) == "NO LINK"
        colors(i,:) = [1 0 0];
    else
        colors(i,:) = [0 1 0];
    end
end
geoscatter(nodeCoords(:,1), nodeCoords(:,2), 80, colors, 'filled');

% Label nodes
for i = 1:N
    text(nodeCoords(i,2), nodeCoords(i,1), sprintf("Node %d (%s)",i,bestSF(i)));
end

%% ------------------ Draw SF Coverage Rings ------------------
SF_colors = lines(6);
sf_keys = ["SF7","SF8","SF9","SF10","SF11","SF12"];

for s = 1:6
    sens = SF_sens(char(sf_keys(s)));
    
    % Find radius where RSSI = sensitivity using DEM-adjusted hm/hb
    testD = linspace(0.1,30,500); % km
    PLvals = zeros(size(testD));
    for k = 1:length(testD)
        % Assume radial in flat direction but adjust for mean DEM height
        PLvals(k) = cost231_hata(testD(k), f, hb + mean(DEM(:)), hm + mean(DEM(:)));
    end
    idx = find(PLvals > Pt_dBm - sens,1);
    if isempty(idx), continue; end
    R_km = testD(idx);
    
    % Draw ring
    latC = gateway(1);
    lonC = gateway(2);
    th = linspace(0,360,400);
    R_deg = (R_km/6371)*(180/pi);
    latR = latC + R_deg*cosd(th);
    lonR = lonC + R_deg*sind(th)./cosd(latC);
    
    geoplot(latR, lonR, 'Color', SF_colors(s,:), 'LineWidth', 1.2);
end

title("LoRa Connectivity Feasibility (DEM-adjusted COST231-Hata, Sri Lanka)");
