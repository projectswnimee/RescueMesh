function run_amplitude_modulation_sim()
%RUN_AMPLITUDE_MODULATION_SIM Build and run a Simulink model for AM modulation.
%   The message m(t)=Am*cos(2*pi*fm*t). Carrier c(t)=Ac*cos(2*pi*fc*t).
%   Creates a model programmatically, runs it, and plots results.

% Parameters
Am = 1;    % message amplitude
fm = 5;    % message frequency (Hz)
Ac = 2;    % carrier amplitude
fc = 50;   % carrier frequency (Hz)
mdepth = 0.7; % modulation index (optional use for standard AM: s = (1 + mdepth*m_norm).*carrier)
fs = 2000; % simulation sample rate (Hz)
tfinal = 0.2; % simulation stop time (s)
model = 'am_mod_model';

% Clean up any existing model with same name
if bdIsLoaded(model)
    close_system(model, 0);
end
if exist([model '.slx'], 'file')
    delete([model '.slx']);
end

% Create new model
new_system(model);
open_system(model);

% Set model configuration parameters
set_param(model, 'Solver', 'FixedStepDiscrete', 'FixedStep', num2str(1/fs), ...
    'StopTime', num2str(tfinal), 'SaveOutput', 'on', 'SaveFormat', 'StructureWithTime');

% Add blocks positions
x = 30; y = 30; dx = 170; dy = 70;
add_block('simulink/Sources/Sine Wave', [model '/MessageSine'], 'Position', [x y x+120 y+50], ...
    'Amplitude', num2str(Am), 'Frequency', num2str(2*pi*fm), 'SampleTime', num2str(1/fs));
add_block('simulink/Sources/Sine Wave', [model '/CarrierSine'], 'Position', [x y+dy x+120 y+dy+50], ...
    'Amplitude', num2str(Ac), 'Frequency', num2str(2*pi*fc), 'SampleTime', num2str(1/fs));

% Normalize message for standard AM: m_norm = m/Am (so range +/-1)
add_block('simulink/Math Operations/Gain', [model '/NormalizeMsg'], 'Position', [x+dx y x+dx+70 y+50], ...
    'Gain', num2str(1/Am));

% Add Gain for modulation index (multiply normalized message by mdepth)
add_block('simulink/Math Operations/Gain', [model '/ModIndex'], 'Position', [x+2*dx y x+2*dx+70 y+50], ...
    'Gain', num2str(mdepth));

% Add Sum block to compute (1 + mdepth*m_norm)
add_block('simulink/Math Operations/Sum', [model '/OnePlus'], 'Position', [x+3*dx y x+3*dx+70 y+50], ...
    'Inputs', '++'); % two positive inputs: one for constant 1, one for mdepth*m_norm
set_param([model '/OnePlus'], 'IconShape','round');

% Add Constant block for 1
add_block('simulink/Sources/Constant', [model '/OneConst'], 'Position', [x+2*dx y+dy x+2*dx+70 y+dy+50], ...
    'Value', '1');

% Product block to multiply (1 + m*m_norm) with carrier
add_block('simulink/Math Operations/Product', [model '/AMProduct'], 'Position', [x+4*dx y x+4*dx+70 y+50], ...
    'Inputs', '*');

% Add Scope and To Workspace blocks for plotting
add_block('simulink/Sinks/To Workspace', [model '/ToWorkspace_AM'], 'Position', [x+5*dx y x+5*dx+120 y+50], ...
    'VariableName', 'am_signal', 'SaveFormat', 'Structure', 'Decimation', '1');
add_block('simulink/Sinks/To Workspace', [model '/ToWorkspace_Msg'], 'Position', [x+5*dx y+dy x+5*dx+120 y+dy+50], ...
    'VariableName', 'msg_signal', 'SaveFormat', 'Structure', 'Decimation', '1');
add_block('simulink/Sinks/To Workspace', [model '/ToWorkspace_Carrier'], 'Position', [x+5*dx y+2*dy x+5*dx+120 y+2*dy+50], ...
    'VariableName', 'carrier_signal', 'SaveFormat', 'Structure', 'Decimation', '1');

% Connect blocks (rewired correctly)
add_line(model, 'MessageSine/1', 'NormalizeMsg/1', 'autorouting','on');      % message -> normalize
add_line(model, 'NormalizeMsg/1', 'ModIndex/1', 'autorouting','on');         % normalized -> mod index gain
add_line(model, 'ModIndex/1', 'OnePlus/2', 'autorouting','on');             % mdepth*m_norm -> OnePlus input 2
add_line(model, 'OneConst/1', 'OnePlus/1', 'autorouting','on');             % constant 1 -> OnePlus input 1
add_line(model, 'OnePlus/1', 'AMProduct/1', 'autorouting','on');            % (1 + m*m_norm) -> AMProduct input 1
add_line(model, 'CarrierSine/1', 'AMProduct/2', 'autorouting','on');        % carrier -> AMProduct input 2

% Connect final signals to workspace
add_line(model, 'AMProduct/1', 'ToWorkspace_AM/1', 'autorouting','on');
add_line(model, 'MessageSine/1', 'ToWorkspace_Msg/1', 'autorouting','on');
add_line(model, 'CarrierSine/1', 'ToWorkspace_Carrier/1', 'autorouting','on');

% Save and update diagram
save_system(model);
set_param(model, 'SimulationCommand', 'update');

% Run simulation
simOut = sim(model);

% Retrieve data
am_sig = simOut.get('am_signal'); % structure with time and signals
msg_sig = simOut.get('msg_signal');
car_sig = simOut.get('carrier_signal');

t = am_sig.time;
am = am_sig.signals.values;
m = msg_sig.signals.values;
c = car_sig.signals.values;

% Plot results
figure('Name','AM Modulation','NumberTitle','off');
subplot(3,1,1);
plot(t, m);
xlabel('Time (s)'); ylabel('m(t)');
title('Message Signal');

subplot(3,1,2);
plot(t, c);
xlabel('Time (s)'); ylabel('c(t)');
title('Carrier Signal');

subplot(3,1,3);
plot(t, am);
xlabel('Time (s)'); ylabel('s_{AM}(t)');
title('AM Signal (s(t) = (1 + mdepth*m/Am) * Ac * cos(2\pift))');

% Close model without saving
close_system(model, 0);
end