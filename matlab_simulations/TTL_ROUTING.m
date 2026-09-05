clc; clear; close all;

%TTL limit – Every forwarded packet must respect a Time-To-Live value to prevent infinite looping.
%Randomized broadcast delay – Nodes must add a small random delay before rebroadcasting to avoid collisions and broadcast storms.
%Deduplication cache timeout – Each node keeps a short-term cache of recently seen packet IDs to avoid forwarding duplicates.

%% Coordinates (GW is top middle)
coords = [0 0; 2 2; 4 1; 6 3; 4 3.5]; % N1 N2 N3 N4 GW
nodeNames = {'N1','N2','N3','N4','GW'};

figure('Color','w'); hold on;
axis([-1 9 -1 5]); axis equal;

%% Draw nodes
for i = 1:5
    plot(coords(i,1), coords(i,2), 'o', 'MarkerSize',15, ...
         'MarkerFaceColor',[0.2 0.6 1], 'MarkerEdgeColor','k');
    text(coords(i,1)+0.1, coords(i,2), nodeNames{i}, ...
         'FontSize',12, 'FontWeight','bold');
end

%% Optional reference links
connections = [1 2; 2 3; 3 4; 1 5; 2 5; 3 5; 4 5];
for i = 1:size(connections,1)
    a = connections(i,1); b = connections(i,2);
    plot([coords(a,1) coords(b,1)], [coords(a,2) coords(b,2)], '--k');
end


%% --- Fading Arrow Animation Function ---
function animate_packet(src,dst,coords,color)

    x1 = coords(src,1);  y1 = coords(src,2);
    x2 = coords(dst,1);  y2 = coords(dst,2);

    % initial faint path line
    baseLine = plot([x1 x2],[y1 y2],'Color',[color 0.1],'LineWidth',2);
    drawnow;

    steps = 50;  % smooth animation

    for t = linspace(0,1,steps)

        alpha = min(1, t*2);     % fade in
        fadeColor = [color alpha];

        % Interpolated point
        x = x1 + t*(x2 - x1);
        y = y1 + t*(y2 - y1);

        % Larger arrowhead (MaxHeadSize increased)
        h = quiver(x, y, (x2-x)*0.001, (y2-y)*0.001, ...
                   'Color', fadeColor, 'MaxHeadSize', 30, 'LineWidth', 2.5);

        drawnow;
        pause(0.02);
        delete(h);
    end

    % Fade out the base line
    for fade = 0.1:-0.02:0
        set(baseLine,'Color',[color fade]);
        drawnow;
        pause(0.01);
    end

    delete(baseLine);
    pause;  % wait for keypress step-by-step
end


%% ------- ROUTING STEPS -------

fprintf('\n--- Step 1: N1 → Gateway ---\n');
animate_packet(1,5,coords,[1 0 0]); % red
fprintf('N1 SOS delivered.\n\n');

fprintf('--- Step 2: N2 → N1 → Gateway ---\n');
animate_packet(2,1,coords,[0 1 0]); % green
animate_packet(1,5,coords,[0 1 0]);
fprintf('N2 SOS delivered.\n\n');

fprintf('--- Step 3: N3 long path fails (TTL=2) ---\n');
ttl = 2;
animate_packet(3,2,coords,[1 0 1]); ttl=ttl-1; disp(['TTL: ' num2str(ttl)]);
animate_packet(2,1,coords,[1 0 1]); ttl=ttl-1; disp(['TTL: ' num2str(ttl)]);
disp('TTL expired. Retrying later via alternative route...'); 
pause;

%% *** Step 4 REMOVED COMPLETELY ***

fprintf('--- Step 4: N4 → N3 → Gateway ---\n');
animate_packet(4,3,coords,[0 1 1]); % cyan
animate_packet(3,5,coords,[0 1 1]);
fprintf('N4 SOS delivered.\n\n');

fprintf('--- Step 5: N3 final SOS via N4 → Gateway ---\n');
animate_packet(3,4,coords,[1 1 1]); % white
animate_packet(4,5,coords,[1 1 1]);
fprintf('N3 SOS delivered through N4.\n\n');

disp('--- Simulation Complete ---');
