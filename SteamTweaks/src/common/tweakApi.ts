
export const initTweak = <T>(name: string, tweakMain: (() => T)|{
    install: () => T;
    uninstall: () => void;
}, force = false): T => {
    if (!force && window.GlosSITweaks[name]) {
        throw new Error(`Tweak ${name} is already installed!`);
    }

    if (typeof tweakMain === 'object') {
        window.GlosSITweaks[name] = { install: tweakMain.install, uninstall: () => {
            try {
                tweakMain.uninstall();
            } catch (e) {
                GlosSIApi.SteamTarget.log('error', e);
            }
            delete window.GlosSITweaks[name];
        } };
    } else {
        window.GlosSITweaks[name] = { install: tweakMain };
    }
    try {
        return window.GlosSITweaks[name].install() as T;
    } catch (e) {
        GlosSIApi.SteamTarget.log('error', e);
        throw e;
    }

};
