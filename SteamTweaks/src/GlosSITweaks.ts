import type { SteamConfig } from './common/util/types';
import { fetchWithTimeout } from './common/util/util';



class SteamTargetApi {
    public getSteamSettings(): Promise<SteamConfig> {
        return fetch('http://localhost:8756/steam_settings')
            .then(
                (res) => res.json().then(
                    (json) => (json as SteamConfig).UserLocalConfigStore as SteamConfig
                )
            );
    }

    public getGlosSIActive() {
        return fetchWithTimeout('http://localhost:8756/', { timeout: 10000 })
            .then(
                () => true
            ).catch((e) => false);
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

    const glossiCheckInterval = setInterval(() => {
        if (window.GlosSIApi) {
            window.GlosSIApi.SteamTarget.getGlosSIActive().then((active) => {
                if (!active) {
                    window?.GlosSITweaks?.GlosSI?.uninstall?.();
                }
            });
            return;
        }
        clearTimeout(glossiCheckInterval)
    }, 5000)

};

if (!window.GlosSITweaks || !window.GlosSIApi) {
    installGlosSIApi();
}

export default !!window.GlosSITweaks && !!window.GlosSIApi;
