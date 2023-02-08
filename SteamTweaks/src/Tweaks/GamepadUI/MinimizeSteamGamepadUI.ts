import { SteamUiMode } from '../../common/Steam';
import { initTweak } from '../../common/tweakApi';


initTweak('MinimizeSteamGamepadUI', async () => {

    const [isGamepadUI, minimizeGPUI] = await Promise.all([
        (async () => (await SteamClient.UI.GetUiMode()) === SteamUiMode.GamepadUI)(),
        (async () => (await GlosSIApi.SteamTarget.getGlosSISettings()).minimizeSteamGamepadUI)()
    ]);
    if (isGamepadUI && minimizeGPUI) {
        SteamClient.Window.Minimize();
        return true;
    }
    if (!isGamepadUI && minimizeGPUI) {
        console.warn('MinimizeSteamGamepadUI is enabled but Steam is not in GamepadUI mode');
    }
    return false;
}).then((minimized: boolean) => {
    console.log('MinimizeSteamGamepadUI installed; Minimized GamepadUI:', minimized);
}).catch((e) => console.error('MinimizeSteamGamepadUI failed to install', e));
