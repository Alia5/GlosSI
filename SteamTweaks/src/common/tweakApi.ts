import type { SteamConfig } from './util/types';
class GlosSIApi {
    public getSettings(): Promise<SteamConfig> {
        return fetch('http://localhost:8756/steam_settings')
            .then(
                (res) => res.json().then(
                    (json) => (json as SteamConfig).UserLocalConfigStore as SteamConfig
                )
            );
    }
}

declare global {
    interface Window {
        GlosSITweaks: Record<string, {install: () => unknown; uninstall?: () => void}>;
        GlosSI: InstanceType<typeof GlosSIApi>;
    }
    // eslint-disable-next-line
    const GlosSI: InstanceType<typeof GlosSIApi>;
}


const installGlosSIApi = () => {
    window.GlosSITweaks = {
        GlosSI: {
            install: () => {
                const api = new GlosSIApi();
                Object.assign(window, { GlosSI: api });
            },
            uninstall: () => {
                // eslint-disable-next-line @typescript-eslint/ban-ts-comment
                // @ts-ignore
                delete window.GlosSI;
            }
        }
    };
    window.GlosSITweaks.GlosSI.install();
};

if (!window.GlosSITweaks || !window.GlosSI) {
    installGlosSIApi();
}

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
