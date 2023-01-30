import type { SteamConfig } from './common/util/types';
class SteamTargetApi {
    public getSteamSettings(): Promise<SteamConfig> {
        return fetch('http://localhost:8756/steam_settings')
            .then(
                (res) => res.json().then(
                    (json) => (json as SteamConfig).UserLocalConfigStore as SteamConfig
                )
            );
    }
}

class GlosSIApiCtor {
    public readonly SteamTarget: SteamTargetApi = new SteamTargetApi();
}

interface GlosSITweaks {
    [tweakName: string]: { readonly install: () => unknown; readonly uninstall?: () => void }
}

declare global {
    interface Window {
        GlosSITweaks: GlosSITweaks
        GlosSIApi: InstanceType<typeof GlosSIApiCtor>;
    }

    // eslint-disable-next-line
    const GlosSIApi: InstanceType<typeof GlosSIApiCtor>;
    const GlosSITweaks: GlosSITweaks
}


const installGlosSIApi = () => {
    window.GlosSITweaks = {
        GlosSI: {
            install: () => {
                const api = new GlosSIApiCtor();
                Object.assign(window, { GlosSIApi: api });
            },
            uninstall: () => {
                Object.entries(window.GlosSITweaks)
                    .filter(([tweakName, obj]) => (tweakName !== 'GlosSI'))
                    .forEach(([, obj]) => obj.uninstall?.());
                // eslint-disable-next-line @typescript-eslint/ban-ts-comment
                // @ts-ignore
                delete window.GlosSIApi;
                // eslint-disable-next-line @typescript-eslint/ban-ts-comment
                // @ts-ignore
                delete window.GlosSITweaks;
            }
        }
    };
    window.GlosSITweaks.GlosSI.install();
};

if (!window.GlosSITweaks || !window.GlosSIApi) {
    installGlosSIApi();
}
