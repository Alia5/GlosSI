
export const initTweak = <T>(name: string, tweakMain: (() => T)|{
    install: () => T;
    uninstall: () => void;
}, force = false): T|Error => {
    if (!force && window.GlosSITweaks[name]) {
        return new Error(`Tweak ${name} is already installed!`);
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
