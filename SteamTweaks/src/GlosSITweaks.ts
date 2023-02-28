import type { SteamConfig } from './common/util/types';
import { fetchWithTimeout } from './common/util/util';
import type { GlosSISettings } from './@types/GlosSISettings';


class SteamTargetApi {

    public static GlosSIActive = true;

    public static readonly ACTIVE_FAIL_THRESHOLD = 2;
    private activeFailCounter = 0;
    private static ActiveCheckTimer = 0;

    public constructor() {
        if (SteamTargetApi.ActiveCheckTimer !== 0) {
            clearInterval(SteamTargetApi.ActiveCheckTimer);
        }
        SteamTargetApi.ActiveCheckTimer = setInterval(() => {
            void this.getGlosSIActive().then((active) => {
                if (!active) {
                    this.activeFailCounter++;
                    if (this.activeFailCounter >= SteamTargetApi.ACTIVE_FAIL_THRESHOLD) {
                        window?.GlosSITweaks?.GlosSI?.uninstall?.();
                    }
                }
            });
        }, 666);
    }

    public async getGlosSIActive() {
        return fetchWithTimeout('http://localhost:8756/running', { timeout: 500 })
            .then(
                () => {
                    SteamTargetApi.GlosSIActive = true;
                    return true;
                }
            ).catch(() => {
                SteamTargetApi.GlosSIActive = false;
                return false;
            });
    }
    public getSteamSettings(): Promise<SteamConfig> {
        return fetch('http://localhost:8756/steam_settings')
            .then(
                (res) => res.json().then(
                    (json) => (json as SteamConfig).UserLocalConfigStore as SteamConfig
                )
            );
    }

    public getGlosSISettings() {
        return fetch('http://localhost:8756/settings')
            .then(
                (res) => res.json().then(
                    (json) => json as GlosSISettings
                )
            );
    }

}


class GlosSIApiCtor {
    public readonly SteamTarget: SteamTargetApi = new SteamTargetApi();

}

interface GlosSITweaks {
    [tweakName: string]: { readonly install: () => unknown; readonly uninstall?: () => void };
}

declare global {
    interface Window {
        GlosSITweaks: GlosSITweaks;
        GlosSIApi: InstanceType<typeof GlosSIApiCtor>;
    }

    // eslint-disable-next-line
    const GlosSIApi: InstanceType<typeof GlosSIApiCtor>;
    // eslint-disable-next-line
    const GlosSITweaks: GlosSITweaks;
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
                    .filter(([tweakName]) => (tweakName !== 'GlosSI'))
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
            void window.GlosSIApi.SteamTarget.getGlosSIActive().then((active) => {
                if (!active) {
                    window?.GlosSITweaks?.GlosSI?.uninstall?.();
                }
            });
            return;
        }
        clearTimeout(glossiCheckInterval);
    }, 5000);

};

if (!window.GlosSITweaks || !window.GlosSIApi) {
    installGlosSIApi();
}

export default !!window.GlosSITweaks && !!window.GlosSIApi;
