
export const initTweak = <T>(name: string, tweakMain: (() => T)|{
    install: () => T;
    uninstall: () => void;
}, force = false): T => {
    if (!force && window.GlosSITweaks[name]) {
        throw new Error(`Tweak ${name} is already installed!`);
    }

    if (typeof tweakMain === 'object') {
        window.GlosSITweaks[name] = { install: tweakMain.install, uninstall: () => {
            tweakMain.uninstall();
            delete window.GlosSITweaks[name];
        } };
    } else {
        window.GlosSITweaks[name] = { install: tweakMain };
    }
    return window.GlosSITweaks[name].install() as T;

};
