%% ============================================================
% LoRa Connectivity Feasibility with DEM-adjusted SF Rings
% Streets Map + COST231-Hata + DEM terrain adjustment
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
    6.9530 79.9172;
    6.9600 79.9300;
    6.9650 79.9400;
];
N = size(nodeCoords,1);

%% ------------------ LoRa Parameters -------------------------
f = 915;          % MHz
hb = 30;          % Gateway height (m)
hm = 1.5;         % Node height (m)
Pt_dBm = 14;      % Tx power
Gt = 0; Gr = 0;

%% Sensitivity values per SF
SF_sens = containers.Map( ...
    {'SF7','SF8','SF9','SF10','SF11','SF12'}, ...
    [-123, -126, -129, -132, -135, -137] );

%% ------------------ DEM Data ------------------------------
% Load your GeoTIFF DEM file (must be in current folder)
demFile = 'DEM_SRTMGL1.tif';
[DEM, R] = readgeoraster(demFile);
DEM = double(DEM); % convert to double for calculations

%% ------------------ COST231-Hata Function ------------------
function L = cost231_hata(d_km, f, hb, hm, Cm)
    if d_km < 0.001, d_km = 0.001; end
    L = 46.3 + 33.9*log10(f) + 13.82*log10(hb) ...
        - (1.1*log10(f)-0.7)*hm + (44.9 - 6.55*log10(hb))*log10(d_km) + Cm;
end

%% Helper: Haversine distance
function dkm = haversine(lat1,lon1,lat2,lon2)
    R = 6371; % km
    dlat = deg2rad(lat2-lat1);
    dlon = deg2rad(lon2-lon1);
    a = sin(dlat/2).^2 + cosd(lat1).*cosd(lat2).*sin(dlon/2).^2;
    dkm = 2*R*asin(sqrt(a));
end

%% ------------------ Node Feasibility ----------------------
RSSI = zeros(N,1);
dist_km = zeros(N,1);
Cm = 3; % urban loss

for i = 1:N
    % Distance
    dist_km(i) = haversine(nodeCoords(i,1), nodeCoords(i,2), gateway(1), gateway(2));

    % Sample DEM along straight line (radial)
    latLine = linspace(gateway(1), nodeCoords(i,1), 100);
    lonLine = linspace(gateway(2), nodeCoords(i,2), 100);
    hTerrain = zeros(1,100);
    for k = 1:100
        [row,col] = geographicToDiscrete(R, latLine(k), lonLine(k));
        row = min(max(row,1), size(DEM,1));
        col = min(max(col,1), size(DEM,2));
        hTerrain(k) = DEM(row,col);
    end
    deltaH = max(hTerrain) - min(hTerrain); % terrain variation along path

    % Adjust hm/hb slightly by terrain variation
    L = cost231_hata(dist_km(i), f, hb+deltaH/2, hm+deltaH/2, Cm);

    RSSI(i) = Pt_dBm + Gt + Gr - L;
end

%% Determine best SF per node
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

% Gateway
geoscatter(gateway(1), gateway(2), 200, 'r', 'filled', 'DisplayName','Gateway');

% Nodes
colors = zeros(N,3);
for i = 1:N
    colors(i,:) = double(bestSF(i)=="NO LINK") * [1 0 0] + double(bestSF(i)~="NO LINK") * [0 1 0];

end
geoscatter(nodeCoords(:,1), nodeCoords(:,2), 80, colors, 'filled');

% Node labels
for i = 1:N
    text(nodeCoords(i,2), nodeCoords(i,1), sprintf("Node %d (%s)",i,bestSF(i)));
end

%% ------------------ Draw DEM-adjusted SF Coverage Rings -----
SF_colors = lines(6);
sf_keys = ["SF7","SF8","SF9","SF10","SF11","SF12"];

for s = 1:6
    sens = SF_sens(char(sf_keys(s)));
    targetPL = Pt_dBm - sens;

    % Sweep radial directions
    th = linspace(0,360,360); % azimuth
    latR = zeros(size(th));
    lonR = zeros(size(th));
    for k = 1:length(th)
        % Radial line
        d_test = linspace(0.05,30,300); % km
        for j = 1:length(d_test)
            lat_pt = gateway(1) + (d_test(j)/6371)*(180/pi)*cosd(th(k));
            lon_pt = gateway(2) + (d_test(j)/6371)*(180/pi)*sind(th(k))/cosd(gateway(1));
            % Sample DEM
            [row,col] = geographicToDiscrete(R, lat_pt, lon_pt);
            row = min(max(row,1), size(DEM,1));
            col = min(max(col,1), size(DEM,2));
            hTerrain = DEM(row,col);

            L_test = cost231_hata(d_test(j), f, hb+hTerrain, hm+hTerrain, Cm);
            if L_test > targetPL
                latR(k) = lat_pt;
                lonR(k) = lon_pt;
                break;
            end
        end
    end
    geoplot(latR, lonR, 'Color', SF_colors(s,:), 'LineWidth', 1.2);
end

title("LoRa Feasibility with DEM-adjusted SF Rings (Sri Lanka)");

