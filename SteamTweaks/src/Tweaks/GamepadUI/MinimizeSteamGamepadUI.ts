import { initTweak } from '../../common/tweakApi';


initTweak('MinimizeSteamGamepadUI', async () => {

    const [isGamepadUI, minimizeGPUI] = await Promise.all([
        // (async () => (await SteamClient.UI.GetUiMode()) === SteamUiMode.GamepadUI)(),
        true, // Steam is always GamepadUI if injected into GamepadUI, duh!
        (async () => (await GlosSIApi.SteamTarget.getGlosSISettings()).minimizeSteamGamepadUI)()
    ]);
    if (isGamepadUI && minimizeGPUI) {
        SteamClient.Window.Minimize();
        return true;
    }
    if (!isGamepadUI && minimizeGPUI) {
        GlosSIApi.SteamTarget.log('warn', 'MinimizeSteamGamepadUI is enabled but Steam is not in GamepadUI mode');
    }
    return false;
}).then((minimized: boolean) => {
    GlosSIApi.SteamTarget.log('debug', 'MinimizeSteamGamepadUI installed; Minimized GamepadUI:', minimized);
}).catch((e) =>GlosSIApi.SteamTarget.log('error', 'MinimizeSteamGamepadUI failed to install', e));
